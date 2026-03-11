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

#include <fstream>
#include <unordered_set>

#include "rmvl/core/util.hpp"
#include "rmvl/lpss/robot.hpp"
#include "rmvlmsg/motion/urdf.hpp"

#include "tinyxml2/tinyxml2.h"

#include "robot_impl.hpp"

namespace rm::lpss {

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

msg::Quaternion operator*(const msg::Quaternion &q1, const msg::Quaternion &q2) {
    return {q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
            q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
            q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
            q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z};
}

msg::Vector3 rotate(const msg::Quaternion &q, const msg::Vector3 &v) {
    const double tx = 2.0 * (q.y * v.z - q.z * v.y);
    const double ty = 2.0 * (q.z * v.x - q.x * v.z);
    const double tz = 2.0 * (q.x * v.y - q.y * v.x);
    return {
        v.x + q.w * tx + (q.y * tz - q.z * ty),
        v.y + q.w * ty + (q.z * tx - q.x * tz),
        v.z + q.w * tz + (q.x * ty - q.y * tx)};
}

msg::Transform operator*(const msg::Transform &t1, const msg::Transform &t2) {
    auto rotated = rotate(t1.rotation, t2.translation);
    return {{t1.translation.x + rotated.x, t1.translation.y + rotated.y, t1.translation.z + rotated.z}, t1.rotation * t2.rotation};
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
        }

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

msg::TransformStamped RobotPlanner::Impl::computeJointTransform(const JointInfo &joint, double q) {
    // Step 1: 静态原点变换：origin xyz + rpy
    msg::Transform origin{{joint.origin_xyz[0], joint.origin_xyz[1], joint.origin_xyz[2]},
                          rpy2quat(joint.origin_rpy[0], joint.origin_rpy[1], joint.origin_rpy[2])};

    // Step 2: 关节运动引起的附加变换
    msg::Transform joint_tf{};
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

msg::JointTrajectory RobotPlanner::plan(std::string_view /* frame */, const msg::Pose & /* target_pose */) const {
    msg::JointTrajectory trajectory;
    trajectory.joint_names = _impl->joint_state.name;

    // TODO: 逆运动学求解目标关节角
    // TODO: 轨迹插值（如梯形速度规划 / 三次样条 / 五次多项式）
    // TODO: 填充 trajectory.points

    return trajectory;
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

using namespace std::chrono_literals;

RobotStatePublisher::RobotStatePublisher(std::string_view name, Node &node, RobotPlanner &planner, uint32_t period)
    : _node(node), _planner(planner), _urdf_pub(node.createPublisher<msg::URDF>(std::string(name) + "/robot_description")),
      _tf_pub(node.createPublisher<msg::TF>(std::string(name) + "/tf")) {
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

    _node.get().destroyPublisher<msg::TF>(_tf_pub);
    _node.get().destroyPublisher<msg::URDF>(_urdf_pub);
}

#if __cplusplus >= 202002L

namespace async {

RobotStatePublisher::RobotStatePublisher(std::string_view name, Node &node, RobotPlanner &planner, uint32_t period)
    : _node(node), _planner(planner) {
    _urdf_pub = node.createPublisher<msg::URDF>(std::string(name) + "/robot_description");
    _tf_pub = node.createPublisher<msg::TF>(std::string(name) + "/tf");
    _urdf_timer = node.createTimer(std::chrono::milliseconds(period), [this] {
        _urdf_pub->publish(_planner.get().urdf());
    });
    _tf_timer = node.createTimer(1s, [this] {
        _tf_pub->publish(_planner.get().tf());
    });
}

RobotStatePublisher::~RobotStatePublisher() {
    _node.get().destroyPublisher<msg::TF>(_tf_pub);
    _node.get().destroyPublisher<msg::URDF>(_urdf_pub);
}

} // namespace async

#endif

} // namespace rm::lpss
