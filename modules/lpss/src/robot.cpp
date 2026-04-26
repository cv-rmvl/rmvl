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

#include <algorithm> // for clamp in Windows
#include <limits>

#include "rmvl/lpss/ctl/base.hpp"
#include "rmvl/lpss/robot.hpp"
#include "tinyxml2/tinyxml2.h"

#include "robot_impl.hpp"

namespace rm {

namespace msg {

msg::Quaternion operator*(const msg::Quaternion &q1, const msg::Quaternion &q2) noexcept {
    return {q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
            q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
            q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
            q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z};
}

msg::Vector3 rotate(const msg::Quaternion &q, const msg::Vector3 &v) noexcept {
    const double tx = 2.0 * (q.y * v.z - q.z * v.y);
    const double ty = 2.0 * (q.z * v.x - q.x * v.z);
    const double tz = 2.0 * (q.x * v.y - q.y * v.x);
    return {
        v.x + q.w * tx + (q.y * tz - q.z * ty),
        v.y + q.w * ty + (q.z * tx - q.x * tz),
        v.z + q.w * tz + (q.x * ty - q.y * tx)};
}

msg::Pose operator*(const msg::Transform &t, const msg::Pose &p) noexcept {
    auto rotated = rotate(t.rotation, {p.position.x, p.position.y, p.position.z});
    return {{t.translation.x + rotated.x, t.translation.y + rotated.y, t.translation.z + rotated.z}, t.rotation * p.orientation};
}

msg::Transform operator*(const msg::Transform &t1, const msg::Transform &t2) noexcept {
    auto rotated = rotate(t1.rotation, t2.translation);
    return {{t1.translation.x + rotated.x, t1.translation.y + rotated.y, t1.translation.z + rotated.z}, t1.rotation * t2.rotation};
}

} // namespace msg

namespace lpss {

/**
 * @brief 从 RPY 角（固定轴 XYZ）计算四元数
 *
 * @param[in] roll Row 滚转角（绕 X 轴）
 * @param[in] pitch Pitch 俯仰角（绕 Y 轴）
 * @param[in] yaw Yaw 偏航角（绕 Z 轴）
 * @return msg::Quaternion
 */
static inline msg::Quaternion rpy2quat(double roll, double pitch, double yaw) {
    const double cr = std::cos(roll * 0.5), sr = std::sin(roll * 0.5);
    const double cp = std::cos(pitch * 0.5), sp = std::sin(pitch * 0.5);
    const double cy = std::cos(yaw * 0.5), sy = std::sin(yaw * 0.5);

    msg::Quaternion q{};
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    return q;
}

/**
 * @brief 从轴角计算四元数
 *
 * @param[in] axis 归一化旋转轴
 * @param[in] angle 旋转角度（弧度）
 * @return 四元数
 */
static inline msg::Quaternion axisAngle2quat(const std::array<double, 3> &axis, double angle) {
    const double ha = angle * 0.5;
    const double s = std::sin(ha);
    return {axis[0] * s, axis[1] * s, axis[2] * s, std::cos(ha)};
}

/**
 * @brief 从路径读取文件内容
 *
 * @param[in] path 文件路径
 * @return 文件内容字符串，失败时返回空字符串
 */
static std::string read_file_content(std::string_view path) {
    auto fp = std::fopen(path.data(), "rb");
    if (!fp)
        return {};

    std::fseek(fp, 0, SEEK_END);
    const auto size = std::ftell(fp);
    std::fseek(fp, 0, SEEK_SET);

    if (size <= 0) {
        std::fclose(fp);
        return {};
    }

    std::string content(static_cast<size_t>(size), '\0');
    const auto read = std::fread(content.data(), 1, content.size(), fp);
    std::fclose(fp);

    content.resize(read);
    return content;
}

void URDFModel::clear() {
    root_link.clear();
    links.clear();
    joints.clear();
    link_index.clear();
    joint_index.clear();
    active_joint_indices.clear();
    children_joints.clear();
}

void URDFModel::parse(std::string_view urdf_str) {
    clear();
    // 解析
    tinyxml2::XMLDocument doc{};
    if (doc.Parse(urdf_str.data(), urdf_str.size()) != tinyxml2::XML_SUCCESS)
        RMVL_Error(RMVL_StsBadArg, "Failed to parse URDF from string");
    auto *robot_elem = doc.FirstChildElement("robot");
    if (!robot_elem)
        RMVL_Error(RMVL_StsBadArg, "No <robot> element in URDF string");

    // 解析 <link>
    for (auto *elem = robot_elem->FirstChildElement("link"); elem;
         elem = elem->NextSiblingElement("link")) {
        LinkInfo li;
        li.name = elem->Attribute("name");

        // 解析 <inertial>
        if (auto *inertial = elem->FirstChildElement("inertial")) {
            if (auto *mass_elem = inertial->FirstChildElement("mass"))
                mass_elem->QueryDoubleAttribute("value", &li.mass);
            if (auto *inertia_elem = inertial->FirstChildElement("inertia")) {
                inertia_elem->QueryDoubleAttribute("ixx", &li.inertia[0]);
                inertia_elem->QueryDoubleAttribute("ixy", &li.inertia[1]);
                inertia_elem->QueryDoubleAttribute("ixz", &li.inertia[2]);
                inertia_elem->QueryDoubleAttribute("iyy", &li.inertia[3]);
                inertia_elem->QueryDoubleAttribute("iyz", &li.inertia[4]);
                inertia_elem->QueryDoubleAttribute("izz", &li.inertia[5]);
            }
        }

        link_index[li.name] = links.size();
        links.push_back(std::move(li));
    }

    // 解析 <joint>
    auto parseType = [](const char *s) -> JointType {
        if (!s)
            return JointType::Fixed;
        std::string ts(s);
        if (ts == "revolute")
            return JointType::Revolute;
        if (ts == "continuous")
            return JointType::Continuous;
        if (ts == "prismatic")
            return JointType::Prismatic;
        return JointType::Fixed;
    };

    for (auto *elem = robot_elem->FirstChildElement("joint"); elem;
         elem = elem->NextSiblingElement("joint")) {
        JointInfo ji;
        ji.name = elem->Attribute("name");
        ji.type = parseType(elem->Attribute("type"));

        if (auto *p = elem->FirstChildElement("parent"))
            ji.parent_link = p->Attribute("link");
        if (auto *c = elem->FirstChildElement("child"))
            ji.child_link = c->Attribute("link");

        if (auto *origin = elem->FirstChildElement("origin")) {
            if (const char *xyz = origin->Attribute("xyz"))
                std::sscanf(xyz, "%lf %lf %lf", &ji.origin_xyz[0], &ji.origin_xyz[1], &ji.origin_xyz[2]);
            if (const char *rpy = origin->Attribute("rpy"))
                std::sscanf(rpy, "%lf %lf %lf", &ji.origin_rpy[0], &ji.origin_rpy[1], &ji.origin_rpy[2]);
        }

        if (auto *axis = elem->FirstChildElement("axis"))
            if (const char *xyz = axis->Attribute("xyz"))
                std::sscanf(xyz, "%lf %lf %lf", &ji.axis[0], &ji.axis[1], &ji.axis[2]);

        if (auto *limit = elem->FirstChildElement("limit")) {
            limit->QueryDoubleAttribute("lower", &ji.lower);
            limit->QueryDoubleAttribute("upper", &ji.upper);
            limit->QueryDoubleAttribute("velocity", &ji.max_velocity);
            limit->QueryDoubleAttribute("effort", &ji.max_effort);
            if (ji.max_velocity <= 0.0)
                ji.max_velocity = 3.14;
        } else
            ji.max_velocity = 3.14;

        std::size_t idx = joints.size();
        joint_index[ji.name] = idx;
        children_joints[ji.parent_link].push_back(idx);

        if (ji.type != JointType::Fixed)
            active_joint_indices.push_back(idx);

        joints.push_back(std::move(ji));
    }

    // 确定 base_link
    std::unordered_set<std::string> child_links{};
    for (const auto &j : joints)
        child_links.insert(j.child_link);
    for (const auto &l : links) {
        if (child_links.find(l.name) == child_links.end()) {
            root_link = l.name;
            break;
        }
    }

    // 根据 effort / 转动惯量 估算 max_acceleration
    // 对旋转关节: a = M / I，I 取子连杆惯性张量在关节旋转轴方向的投影 I = axis^T * I_mat * axis
    // 对移动关节: a = F / m
    // 如果 effort 或惯性信息缺失，则保留默认值 10
    for (auto &j : joints) {
        if (j.type == JointType::Fixed)
            continue;
        if (j.max_effort <= 0.0)
            continue; // effort 未指定，保留默认 max_acceleration = 10

        auto child_it = link_index.find(j.child_link);
        if (child_it == link_index.end())
            continue;
        const auto &child = links[child_it->second];

        // 移动关节: a = F / m
        if (j.type == JointType::Prismatic) {
            if (child.mass > 1e-12)
                j.max_acceleration = j.max_effort / child.mass;
        }
        // 旋转关节: alpha = M / I_axis
        else {
            const auto &ax = j.axis;
            // I_mat = [ixx ixy ixz; ixy iyy iyz; ixz iyz izz]
            const auto &I = child.inertia; // [ixx, ixy, ixz, iyy, iyz, izz]
            // I_axis = axis^T * I_mat * axis（轴方向的等效转动惯量）
            const double Iax = I[0] * ax[0] * ax[0] + I[3] * ax[1] * ax[1] + I[5] * ax[2] * ax[2] + 2.0 * (I[1] * ax[0] * ax[1] + I[2] * ax[0] * ax[2] + I[4] * ax[1] * ax[2]);
            if (Iax > 1e-12)
                j.max_acceleration = j.max_effort / Iax;
        }
    }
}

void RobotPlanner::Impl::resetJointState() {
    const auto n = model.active_joint_indices.size();
    joint_state.name.resize(n);
    joint_state.position.assign(n, 0.0);
    joint_state.velocity.assign(n, 0.0);
    joint_state.effort.assign(n, 0.0);
    for (std::size_t i = 0; i < n; ++i)
        joint_state.name[i] = model.joints[model.active_joint_indices[i]].name;
}

msg::TransformStamped computeJointTransform(const JointInfo &joint, double q) {
    // Step 1: 静态原点变换：origin xyz + rpy
    msg::Transform origin{{joint.origin_xyz[0], joint.origin_xyz[1], joint.origin_xyz[2]},
                          rpy2quat(joint.origin_rpy[0], joint.origin_rpy[1], joint.origin_rpy[2])};

    // Step 2: 关节运动引起的附加变换
    msg::Transform joint_tf{{0, 0, 0}, {0, 0, 0, 1}};
    switch (joint.type) {
    case JointType::Revolute:
    case JointType::Continuous:
        joint_tf.rotation = axisAngle2quat(joint.axis, q);
        break;
    case JointType::Prismatic:
        joint_tf.translation = {joint.axis[0] * q, joint.axis[1] * q, joint.axis[2] * q};
        break;
    case JointType::Fixed:
    default:
        break;
    }

    // Step 3: 合成：T_parent_child = origin * joint_tf
    msg::TransformStamped ts{};
    ts.header.frame_id = joint.parent_link;
    ts.child_frame_id = joint.child_link;
    ts.transform = origin * joint_tf;
    return ts;
}

void RobotPlanner::Impl::updateTF() {
    // [关节名:当前角度]
    std::unordered_map<std::string, double> q_map{};
    for (std::size_t i = 0; i < joint_state.name.size(); ++i)
        q_map[joint_state.name[i]] = joint_state.position[i];

    tf.transforms.clear();
    tf.transforms.reserve(model.joints.size());
    for (const auto &joint : model.joints) {
        double q{};
        if (auto it = q_map.find(joint.name); it != q_map.end())
            q = it->second;
        tf.transforms.push_back(computeJointTransform(joint, q));
    }
}

std::vector<msg::JointTrajectoryPoint> interpolate(const double *q_start, const double *q_end, std::size_t n,
                                                   int64_t t_start, int64_t duration, std::size_t n_seg) {
    const double T = duration * 1e-3;

    std::vector<msg::JointTrajectoryPoint> res{};
    res.reserve(n);

    /**
     * @brief 五次多项式轨迹插值
     *
     * 将关节从 q_start 平滑运动到 q_end，结果追加到 traj 末尾。
     *
     * **时间归一化**
     *
     * 把时间 t 归一化为 s = t / T（T 为总时长），s 从 0 增长到 1。
     * s=0 对应运动起点，s=1 对应运动终点。
     * 归一化后无论总时长是多少，插值公式的形式都一样。
     *
     * **位置插值公式**
     *
     * p(s) = 10s³ - 15s⁴ + 6s⁵
     *
     * 该公式满足以下边界条件，保证机械臂平滑启停、无冲击：
     * - p(0) = 0，p(1) = 1（起点对应 q_start，终点对应 q_end）
     * - p'(0) = 0，p'(1) = 0（两端速度为零）
     * - p''(0) = 0，p''(1) = 0（两端加速度为零）
     *
     * 对每个关节 i，实际位置为：
     *   q_i(s) = q_start_i + (q_end_i - q_start_i) * p(s)
     *
     * 速度和加速度通过对 p(s) 求导得到：
     *   p'(s)  = 30s²(1-s)²           → 速度  = dq * p'(s) / T
     *   p''(s) = 60s(1 - 3s + 2s²)    → 加速度 = dq * p''(s) / T²
     *   其中 dq = q_end_i - q_start_i 是该关节的总位移
     *
     * @param[in] q_start  起始关节角
     * @param[in] q_end    终止关节角
     * @param[in] t_start  起始时间戳（毫秒），用于多段拼接时的时间偏移
     * @param[in] duration 运动总时长（毫秒）
     * @param[in] n_points 插值段数，实际生成 n_points+1 个轨迹点
     * @param[out] traj    轨迹（追加模式，不清空已有内容）
     */

    for (std::size_t k = 0; k <= n_seg; ++k) {
        // s 是归一化时间，从 0 均匀增长到 1
        const double s = static_cast<double>(k) / n_seg;
        const double ps = s * s * s * (10.0 + s * (-15.0 + 6.0 * s));         // p(s)：位置系数
        const double vs = 30.0 * s * s * (1.0 - s) * (1.0 - s) / T;           // p'(s)/T：速度
        const double as = 60.0 * s * (1.0 - 3.0 * s + 2.0 * s * s) / (T * T); // p''(s)/T²：加速度

        msg::JointTrajectoryPoint pt;
        pt.positions.resize(n);
        pt.velocities.resize(n);
        pt.accelerations.resize(n);
        for (std::size_t i = 0; i < n; ++i) {
            const double dq = q_end[i] - q_start[i]; // 该关节的总位移
            pt.positions[i] = q_start[i] + dq * ps;
            pt.velocities[i] = dq * vs;
            pt.accelerations[i] = dq * as;
        }
        pt.time_from_start = t_start + static_cast<int64_t>(s * duration);
        res.push_back(std::move(pt));
    }
    return res;
}

#ifdef _WIN32
#undef max
#undef min
#endif

int64_t estimateDuration(const double *q_start, const double *q_end, const std::vector<const JointInfo *> &joints,
                         double velocity_scale, double acceleration_scale) {
    velocity_scale = std::clamp(velocity_scale, 0.01, 1.0);
    acceleration_scale = std::clamp(acceleration_scale, 0.01, 1.0);

    /**
     * 直接基于五次多项式 p(s) = 10s³ - 15s⁴ + 6s⁵ 的解析峰值求时间下界。
     *
     *   p'(s)  的最大值 = 1.875（在 s=0.5 处） → 峰值速度     = 1.875 * |dq| / T
     *   p''(s) 的最大值 ≈ 5.7735（在 s≈0.2113处）→ 峰值加速度 = 5.7735 * |dq| / T²
     *
     * 为保证峰值不超限:
     *   速度约束:   T >= 1.875  * |dq| / v_max
     *   加速度约束: T >= sqrt(5.7735 * |dq| / a_max)
     *
     * 对所有关节取最大值即可。复杂度 O(n)，n 为活动关节数。
     */
    constexpr double kv = 1.875;  // 五次多项式峰值速度系数
    constexpr double ka = 5.7735; // 五次多项式峰值加速度系数（= 10√3/3）

    double t_max = 0.0;
    for (std::size_t i = 0; i < joints.size(); ++i) {
        const auto *j = joints[i];
        const double dq = std::abs(q_end[i] - q_start[i]);
        if (dq < 1e-12)
            continue;

        const double v = std::max(j->max_velocity * velocity_scale, 1e-6);
        const double a = std::max(j->max_acceleration * acceleration_scale, 1e-6);

        // 速度约束: kv * dq / T <= v  =>  T >= kv * dq / v
        const double t_vel = kv * dq / v;
        // 加速度约束: ka * dq / T² <= a  =>  T >= sqrt(ka * dq / a)
        const double t_acc = std::sqrt(ka * dq / a);

        t_max = std::max({t_max, t_vel, t_acc});
    }

    // 最小时间保护
    t_max = std::max(t_max, 0.05);

    return static_cast<int64_t>(std::ceil(t_max * 1000.0));
}

void RobotPlanner::setMaxVelocityScalingFactor(double factor) noexcept {
    _impl->velocity_scale = std::clamp(factor, 0.01, 1.0);
}

double RobotPlanner::getMaxVelocityScalingFactor() const noexcept {
    return _impl->velocity_scale;
}

void RobotPlanner::setMaxAccelerationScalingFactor(double factor) noexcept {
    _impl->acceleration_scale = std::clamp(factor, 0.01, 1.0);
}

double RobotPlanner::getMaxAccelerationScalingFactor() const noexcept {
    return _impl->acceleration_scale;
}

RobotPlanner::RobotPlanner(std::string_view urdf_path, std::string_view mesh_path) : _impl(std::make_unique<Impl>()) { load(urdf_path, mesh_path); }

RobotPlanner::RobotPlanner(RobotPlanner &&) noexcept = default;
RobotPlanner &RobotPlanner::operator=(RobotPlanner &&) noexcept = default;
RobotPlanner::~RobotPlanner() = default;

void RobotPlanner::load(std::string_view urdf_path, std::string_view mesh_path) {
    _impl->urdf.data = read_file_content(urdf_path);
    if (_impl->urdf.data.empty())
        RMVL_Error_(RMVL_StsBadArg, "Failed to read URDF file: %s", urdf_path.data());
    _impl->urdf.mesh_path = mesh_path;
    _impl->model.parse(_impl->urdf.data);
    _impl->resetJointState();
    _impl->updateTF();
}

void RobotPlanner::update(const msg::JointState &joint_state) {
    _impl->joint_state = joint_state;
    _impl->updateTF();
}

const msg::JointState &RobotPlanner::joints() const noexcept { return _impl->joint_state; }
const msg::URDF &RobotPlanner::urdf() const noexcept { return _impl->urdf; }
const msg::TF &RobotPlanner::tf() const noexcept { return _impl->tf; }

msg::JointTrajectory RobotPlanner::plan(const msg::JointState &target) const {
    msg::JointTrajectory traj{};
    traj.joint_names = _impl->joint_state.name;
    const auto dof = _impl->joint_state.position.size();
    if (dof == 0)
        return traj;

    // 收集活动关节的限位信息用于时间估算
    std::vector<const JointInfo *> active_joints;
    active_joints.reserve(_impl->model.active_joint_indices.size());
    for (auto idx : _impl->model.active_joint_indices)
        active_joints.push_back(&_impl->model.joints[idx]);

    // 起点：当前关节角；终点：目标关节角
    const int64_t duration = estimateDuration(_impl->joint_state.position, target.position,
                                              active_joints, _impl->velocity_scale, _impl->acceleration_scale);
    traj.points = interpolate(_impl->joint_state.position, target.position, 0, duration, 20);
    return traj;
}

msg::Pose RobotPlanner::linkpose(std::string_view link_name) const {
    // child_frame_id -> TransformStamped 的 LUT
    std::unordered_map<std::string, const msg::TransformStamped *> tf_map;
    for (const auto &ts : _impl->tf.transforms)
        tf_map[ts.child_frame_id] = &ts;

    // 从目标连杆沿 TF 树向上递推到根连杆，累积变换
    msg::Transform acc{{0, 0, 0}, {0, 0, 0, 1}};

    std::string current(link_name);
    while (current != _impl->model.root_link) {
        auto it = tf_map.find(current);
        if (it == tf_map.end())
            return {}; // 找不到连杆，返回零位姿
        const auto &ts = *it->second;

        // acc = ts.transform * acc（先应用 acc，再应用 ts）
        acc = ts.transform * acc;
        current = ts.header.frame_id; // 向父连杆移动
    }

    msg::Pose pose{};
    pose.position = {acc.translation.x, acc.translation.y, acc.translation.z};
    pose.orientation = acc.rotation;
    return pose;
}

RobotController::RobotController(const std::vector<std::string> &joint_names, ctl::ControlLawBase::ptr ctl_law) : _ctl_law(std::move(ctl_law)) {
    _ctl_joints.reserve(joint_names.size());
    for (const auto &name : joint_names)
        _ctl_joints.push_back({name, _ctl_joints.size()});
}

bool RobotController::submit(const msg::JointTrajectory &traj) {
    // 关节名校验
    if (traj.joint_names.empty() || traj.points.empty())
        return false;
    for (auto &[name, remap_idx] : _ctl_joints) {
        auto it = std::find(traj.joint_names.begin(), traj.joint_names.end(), name);
        if (it == traj.joint_names.end()) {
            _ctl_joints.clear();
            return false;
        }
        remap_idx = std::distance(traj.joint_names.begin(), it);
    }
    // 时间校验
    if (!std::is_sorted(traj.points.begin(), traj.points.end(), [](const auto &a, const auto &b) { return a.time_from_start < b.time_from_start; }))
        return false;
    // 维度校验
    const auto dof = _ctl_joints.size();
    for (const auto &pt : traj.points)
        if (pt.positions.size() != dof || (!pt.velocities.empty() && pt.velocities.size() != dof) || (!pt.accelerations.empty() && pt.accelerations.size() != dof))
            return false;

    _traj_cache = traj;
    _traj_cache.header.stamp = now();
    return true;
}

void RobotController::reset() noexcept {
    _traj_cache = {};
    for (auto &joint : _ctl_joints)
        joint.remap = _ctl_joints.size(); // 设为无效索引
    _ctl_law->reset();
}

msg::JointState RobotController::sample(const msg::JointState &feedback) noexcept {
    // 计算当前时间
    auto now_time = now();
    const auto to_ms = [](const msg::Time &t) noexcept {
        return static_cast<int64_t>(t.sec) * 1000 + static_cast<int64_t>(t.nsec) / 1000000;
    };

    const auto now_ms = to_ms(now_time);
    const auto start_ms = to_ms(_traj_cache.header.stamp);
    const auto elapsed_ms = std::max<int64_t>(0, now_ms - start_ms);

    const auto dof = _ctl_joints.size();
    msg::JointState desired{};
    desired.header.stamp = now_time;
    desired.name.reserve(dof);
    desired.position.assign(dof, 0.0);
    desired.velocity.assign(dof, 0.0);
    desired.effort.assign(dof, 0.0);
    for (const auto &joint : _ctl_joints)
        desired.name.push_back(joint.name);

    const auto assign_from_point = [&](const msg::JointTrajectoryPoint &pt) {
        for (std::size_t i = 0; i < dof; ++i) {
            const auto remap = _ctl_joints[i].remap;
            if (remap < pt.positions.size())
                desired.position[i] = pt.positions[remap];
            if (remap < pt.velocities.size())
                desired.velocity[i] = pt.velocities[remap];
            if (remap < pt.effort.size())
                desired.effort[i] = pt.effort[remap];
        }
    };

    if (_traj_cache.points.empty()) {
        if (!feedback.name.empty()) {
            desired = feedback;
            desired.velocity.assign(desired.velocity.size(), 0.0);
            desired.effort.assign(desired.effort.size(), 0.0);
            return desired;
        }
        return {};
    }
    const auto &pts = _traj_cache.points;
    if (elapsed_ms <= pts.front().time_from_start)
        assign_from_point(pts.front());
    else if (elapsed_ms >= pts.back().time_from_start) {
        assign_from_point(pts.back());
        desired.velocity.assign(desired.velocity.size(), 0.0);
        desired.effort.assign(desired.effort.size(), 0.0);
    } else {
        const auto it_hi = std::upper_bound(pts.begin(), pts.end(), elapsed_ms, [](int64_t t, const msg::JointTrajectoryPoint &pt) {
            return t < pt.time_from_start;
        });
        const auto &p1 = *it_hi;
        const auto &p0 = *(it_hi - 1);
        const auto dt = p1.time_from_start - p0.time_from_start;
        const double ratio = (dt > 0) ? std::clamp(static_cast<double>(elapsed_ms - p0.time_from_start) / static_cast<double>(dt), 0.0, 1.0) : 0.0;

        for (std::size_t i = 0; i < dof; ++i) {
            const auto remap = _ctl_joints[i].remap;
            // 位置插值
            if (remap < p0.positions.size() && remap < p1.positions.size())
                desired.position[i] = p0.positions[remap] + (p1.positions[remap] - p0.positions[remap]) * ratio;
            // 速度插值
            if (remap < p0.velocities.size() && remap < p1.velocities.size())
                desired.velocity[i] = p0.velocities[remap] + (p1.velocities[remap] - p0.velocities[remap]) * ratio;
            else if (remap < p0.velocities.size())
                desired.velocity[i] = p0.velocities[remap];
            else if (remap < p1.velocities.size())
                desired.velocity[i] = p1.velocities[remap];
            // 转矩插值
            if (remap < p0.effort.size() && remap < p1.effort.size())
                desired.effort[i] = p0.effort[remap] + (p1.effort[remap] - p0.effort[remap]) * ratio;
            else if (remap < p0.effort.size())
                desired.effort[i] = p0.effort[remap];
            else if (remap < p1.effort.size())
                desired.effort[i] = p1.effort[remap];
        }
    }

    // 应用控制律
    msg::JointState cmd{};
    const auto ctl_time_ms = static_cast<int32_t>(std::clamp<int64_t>(elapsed_ms, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()));
    auto res = _ctl_law->compute(desired, feedback, ctl_time_ms, cmd);
    if (res != ctl::ControlStatus::Ok) {
        cmd = !feedback.name.empty() ? feedback : desired;
        cmd.effort.assign(cmd.effort.size(), 0.0);
        cmd.velocity.assign(cmd.velocity.size(), 0.0);
    }
    return cmd;
}

using namespace std::chrono_literals;

RobotStatePublisher::RobotStatePublisher(std::string_view name, Node &node, RobotPlanner &planner, uint32_t period)
    : _node(node), _planner(planner), _urdf_pub(node.createPublisher<msg::URDF>(std::string(name) + "/robot_description")),
      _tf_pub(node.createPublisher<msg::TF>(std::string(name) + "/tf")), _traj_pub(node.createPublisher<msg::JointTrajectory>(std::string(name) + "/trajectory")) {
    if (_urdf_pub.invalid() || _tf_pub.invalid() || _traj_pub.invalid())
        RMVL_Error(RMVL_StsError, "Failed to create publishers for RobotStatePublisher");

    _tf_thread = std::thread([this, period] {
        while (true) {
            {
                std::unique_lock lk(_shutdown_mtx);
                if (_shutdown_cv.wait_for(lk, std::chrono::milliseconds(period), [this] { return !_running; }))
                    break;
            }
            msg::TF current_tf;
            {
                std::lock_guard lk(_mtx);
                current_tf = _planner.get().tf();
            }
            _tf_pub.publish(current_tf);
        }
    });
    _urdf_thread = std::thread([this] {
        while (true) {
            {
                std::unique_lock lk(_shutdown_mtx);
                if (_shutdown_cv.wait_for(lk, 1s, [this] { return !_running; }))
                    break;
            }
            msg::URDF current_urdf;
            {
                std::lock_guard lk(_mtx);
                current_urdf = _planner.get().urdf();
            }
            _urdf_pub.publish(current_urdf);
        }
    });
    _traj_thread = std::thread([this] {
        while (true) {
            {
                std::unique_lock lk(_shutdown_mtx);
                if (_shutdown_cv.wait_for(lk, 1s, [this] { return !_running; }))
                    break;
            }
            msg::JointTrajectory current_traj;
            {
                std::lock_guard lk(_mtx);
                current_traj = _traj_cache;
            }
            _traj_pub.publish(current_traj);
        }
    });
}

RobotStatePublisher::~RobotStatePublisher() {
    {
        std::lock_guard lk(_shutdown_mtx);
        _running = false;
    }
    _shutdown_cv.notify_all();
    if (_tf_thread.joinable())
        _tf_thread.join();
    if (_urdf_thread.joinable())
        _urdf_thread.join();
    if (_traj_thread.joinable())
        _traj_thread.join();
    _node.get().destroyPublisher(_tf_pub);
    _node.get().destroyPublisher(_urdf_pub);
    _node.get().destroyPublisher(_traj_pub);
}

void RobotStatePublisher::updateTrajectory(msg::JointTrajectory &&traj) noexcept {
    std::lock_guard lk(_mtx);
    _traj_cache = std::move(traj);
}

void RobotStatePublisher::updateTrajectory(const msg::JointTrajectory &traj) noexcept {
    std::lock_guard lk(_mtx);
    _traj_cache = traj;
}

#if __cplusplus >= 202002L

namespace async {

RobotStatePublisher::RobotStatePublisher(std::string_view name, Node &node, RobotPlanner &planner, uint32_t period)
    : _node(node), _planner(planner) {
    _urdf_pub = node.createPublisher<msg::URDF>(std::string(name) + "/robot_description");
    _tf_pub = node.createPublisher<msg::TF>(std::string(name) + "/tf");
    _traj_pub = node.createPublisher<msg::JointTrajectory>(std::string(name) + "/trajectory");
    if (!_urdf_pub || !_tf_pub || !_traj_pub || _urdf_pub->invalid() || _tf_pub->invalid() || _traj_pub->invalid())
        RMVL_Error(RMVL_StsError, "Failed to create publishers for RobotStatePublisher");
    _low_timer = node.createTimer(1s, [this] {
        _urdf_pub->publish(_planner.get().urdf());
        _traj_pub->publish(_traj_cache);
    });
    _high_timer = node.createTimer(std::chrono::milliseconds(period), [this] {
        _tf_pub->publish(_planner.get().tf());
    });
}

RobotStatePublisher::~RobotStatePublisher() {
    _node.get().destroyPublisher(_tf_pub);
    _node.get().destroyPublisher(_urdf_pub);
    _node.get().destroyPublisher(_traj_pub);
}

} // namespace async

#endif

} // namespace lpss

} // namespace rm
