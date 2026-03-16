/**
 * @file robot.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief LPSS 机器人控制接口实现
 * @version 1.0
 * @date 2026-03-04
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#ifdef RMVL_LPSS_WITH_KDL

#include "robot_impl.hpp"

namespace rm::lpss {

bool RobotPlanner::Impl::buildChain(std::string_view frame, KDL::Chain &chain,
                                    std::vector<const JointInfo *> &path) const {
    // DFS从根连杆出发，找到通往目标连杆 frame 的关节路径，记录在 path 中
    std::function<bool(const std::string &)> dfs = [&](const std::string &cur) -> bool {
        if (cur == frame)
            return true;
        auto it = model.children_joints.find(cur);
        if (it == model.children_joints.end())
            return false;
        for (std::size_t idx : it->second) {
            const auto &j = model.joints[idx];
            path.push_back(&j);
            if (dfs(j.child_link))
                return true;
            path.pop_back();
        }
        return false;
    };

    if (!dfs(model.root_link))
        return false;

    /**
     * @brief 首段 fixed segment 承载根关节的 origin 偏移
     * KDL 的 Segment 只能描述"关节旋转 + 到下一关节的偏移"，没有地方描述第一个关节自身的安装偏移（origin），所以在运动链最前面加一个不会转动的 fixed segment 专门承载这个偏移
     */
    const auto *j0 = path[0];
    chain.addSegment(KDL::Segment(
        "root_offset", KDL::Joint(KDL::Joint::Fixed),
        KDL::Frame(KDL::Rotation::RPY(j0->origin_rpy[0], j0->origin_rpy[1], j0->origin_rpy[2]),
                   KDL::Vector(j0->origin_xyz[0], j0->origin_xyz[1], j0->origin_xyz[2]))));

    for (std::size_t i = 0; i < path.size(); ++i) {
        const auto *j = path[i];

        // 根据关节类型创建对应的KDL关节（旋转/移动/固定）
        KDL::Joint kdl_joint;
        switch (j->type) {
        case JointType::Revolute:
        case JointType::Continuous:
            // 旋转关节
            kdl_joint = KDL::Joint(j->name, KDL::Vector::Zero(),
                                   KDL::Vector(j->axis[0], j->axis[1], j->axis[2]), KDL::Joint::RotAxis);
            break;
        case JointType::Prismatic:
            // 移动关节
            kdl_joint = KDL::Joint(j->name, KDL::Vector::Zero(),
                                   KDL::Vector(j->axis[0], j->axis[1], j->axis[2]), KDL::Joint::TransAxis);
            break;
        default:
            // 固定关节
            kdl_joint = KDL::Joint(j->name, KDL::Joint::Fixed);
            break;
        }

        // f_tip 是从当前关节原点到下一关节原点的变换，末端关节取 Identity（无偏移）
        KDL::Frame f_tip = KDL::Frame::Identity();
        if (i + 1 < path.size()) {
            const auto *jn = path[i + 1];
            f_tip = KDL::Frame(
                KDL::Rotation::RPY(jn->origin_rpy[0], jn->origin_rpy[1], jn->origin_rpy[2]),
                KDL::Vector(jn->origin_xyz[0], jn->origin_xyz[1], jn->origin_xyz[2]));
        }
        chain.addSegment(KDL::Segment(j->child_link, kdl_joint, f_tip));
    }
    return true;
}

bool RobotPlanner::Impl::solveIK(const KDL::Chain &chain,
                                 const std::vector<const JointInfo *> &path,
                                 const msg::Pose &target,
                                 KDL::JntArray &q_out) const {
    // 使用 KDL 提供的 LMA（Levenberg-Marquardt）迭代求解器：
    //   从一个初始关节角出发，不断调整，直到末端位姿和目标足够接近
    //   迭代最多 5000 次，精度要求 1e-5
    const unsigned int dof = chain.getNrOfJoints();

    // 以当前关节角作为IK初始值
    std::unordered_map<std::string, double> q_map;
    for (std::size_t i = 0; i < joint_state.name.size(); ++i)
        q_map[joint_state.name[i]] = joint_state.position[i];

    KDL::JntArray q_init(dof);
    unsigned int qi = 0;
    for (const auto *j : path) {
        if (j->type != JointType::Fixed) {
            auto it = q_map.find(j->name);
            q_init(qi++) = (it != q_map.end()) ? it->second : 0.0;
        }
    }

    // 将目标位姿从msg::Pose 转换为 KDL::Frame
    const auto &p = target.position;
    const auto &q = target.orientation;
    KDL::Frame t_goal(KDL::Rotation::Quaternion(q.x, q.y, q.z, q.w),
                      KDL::Vector(p.x, p.y, p.z));

    // DOF < 6 时只约束位置，忽略姿态，避免低自由度机械臂无解
    Eigen::Matrix<double, 6, 1> L;
    L << 1, 1, 1, (dof >= 6 ? 1.0 : 0.0), (dof >= 6 ? 1.0 : 0.0), (dof >= 6 ? 1.0 : 0.0);
    KDL::ChainIkSolverPos_LMA ik_solver(chain, L, 1e-5, 5000);
    q_out.resize(dof);
    return ik_solver.CartToJnt(q_init, t_goal, q_out) >= 0;
}

/**
 * @brief 笛卡尔空间点到点规划
 *
 * 已知末端目标位姿，先用逆运动学（LMA 迭代）求出对应关节角，
 * 再从当前关节角出发做五次多项式插值。
 *
 * 若目标位姿超出工作空间或逆运动学不收敛，返回空轨迹。
 *
 * @param[in] frame 目标连杆名称（末端执行器所在连杆）
 * @param[in] target 目标位姿（位置 + 四元数姿态）
 * @return 关节轨迹，IK 失败或连杆不存在时返回空轨迹
 */
msg::JointTrajectory RobotPlanner::plan(std::string_view frame, const msg::Pose &target) const {
    msg::JointTrajectory traj;
    traj.joint_names = _impl->joint_state.name;

    // 从URDF建运动链，找不到目标连杆则返回空轨迹
    KDL::Chain chain;
    std::vector<const JointInfo *> path;
    if (!_impl->buildChain(frame, chain, path) || chain.getNrOfJoints() == 0)
        return traj;

    // IK求解目标关节角，失败则返回空轨迹
    KDL::JntArray q_out;
    if (!_impl->solveIK(chain, path, target, q_out))
        return traj;

    // 起点为当前关节角
    const unsigned int dof = q_out.rows();
    KDL::JntArray q_start(dof);
    unsigned int qi = 0;
    for (const auto *j : path) {
        if (j->type != JointType::Fixed) {
            auto it = std::find(_impl->joint_state.name.begin(),
                                _impl->joint_state.name.end(), j->name);
            q_start(qi++) = (it != _impl->joint_state.name.end())
                                ? _impl->joint_state.position[std::distance(
                                      _impl->joint_state.name.begin(), it)]
                                : 0.0;
        }
    }
    // 收集路径上的活动关节信息用于时间估算
    std::vector<const JointInfo *> active_path;
    for (const auto *j : path)
        if (j->type != JointType::Fixed)
            active_path.push_back(j);

    const int64_t duration = estimateDuration(q_start.data.data(), q_out.data.data(),
                                              active_path, _impl->velocity_scale, _impl->acceleration_scale);
    traj.points = interpolate(q_start.data.data(), q_out.data.data(), dof, 0, duration, 20);
    return traj;
}

/**
 * @brief 笛卡尔空间多段途经点规划
 *
 * 末端依次经过多个目标位姿，对每段分别做逆运动学 + 五次多项式插值，
 * 各段轨迹首尾拼接，每段根据关节速度/加速度限制自动估算时长。
 *
 * 若某段逆运动学失败，返回已成功规划的部分轨迹。
 *
 * @param[in] frame 目标连杆名称（末端执行器所在连杆）
 * @param[in] waypoints 途经位姿列表，按顺序依次到达
 * @return 关节轨迹，连杆不存在时返回空轨迹
 */
msg::JointTrajectory RobotPlanner::plan(std::string_view frame, const std::vector<msg::Pose> &waypoints) const {
    msg::JointTrajectory traj;
    traj.joint_names = _impl->joint_state.name;

    KDL::Chain chain;
    std::vector<const JointInfo *> path;
    if (!_impl->buildChain(frame, chain, path) || chain.getNrOfJoints() == 0)
        return traj;

    // 初始化起点为当前关节角
    const unsigned int dof = chain.getNrOfJoints();
    KDL::JntArray q_prev(dof);
    unsigned int qi = 0;
    for (const auto *j : path)
        if (j->type != JointType::Fixed) {
            auto it = std::find(_impl->joint_state.name.begin(), _impl->joint_state.name.end(), j->name);
            q_prev(qi++) = (it != _impl->joint_state.name.end())
                               ? _impl->joint_state.position[std::distance(
                                     _impl->joint_state.name.begin(), it)]
                               : 0.0;
        }

    // 收集路径上的活动关节信息用于时间估算
    std::vector<const JointInfo *> active_path;
    for (const auto *j : path)
        if (j->type != JointType::Fixed)
            active_path.push_back(j);

    // 逐段 IK + 插值，每段自动估算时长，时间戳累加
    int64_t t_offset = 0;
    for (const auto &wp : waypoints) {
        KDL::JntArray q_curr;
        if (!_impl->solveIK(chain, path, wp, q_curr))
            return traj; // 任意一段 IK 失败则返回已有部分
        const int64_t seg_duration = estimateDuration(q_prev.data.data(), q_curr.data.data(),
                                                      active_path, _impl->velocity_scale, _impl->acceleration_scale);
        auto seg_traj = interpolate(q_prev.data.data(), q_curr.data.data(), dof, t_offset, seg_duration, 20);
        traj.points.insert(traj.points.end(), std::make_move_iterator(seg_traj.begin()), std::make_move_iterator(seg_traj.end()));
        q_prev = q_curr;
        t_offset += seg_duration;
    }

    return traj;
}

} // namespace rm::lpss

#endif
