/**
 * @file transform.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 三维坐标变换数学操作
 * @version 1.0
 * @date 2026-08-16
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

#include "rmvlmsg/geometry/point.hpp"
#include "rmvlmsg/geometry/pose.hpp"
#include "rmvlmsg/geometry/transform.hpp"
#include "rmvlmsg/geometry/transform_stamped.hpp"
#include "rmvlmsg/motion/tf.hpp"

namespace rm::lpss {
class Node;
namespace async {
class Node;
}
} // namespace rm::lpss

namespace rm::msg {

//! @defgroup lpss_transform 坐标变换数学
//! @ingroup lpss
//! @{
//! @brief 提供四元数旋转、点和位姿转换、SE(3) 复合与求逆操作

//! 四元数乘法
Quaternion operator*(const Quaternion &lhs, const Quaternion &rhs) noexcept;

//! 使用四元数旋转向量
Vector3 rotate(const Quaternion &rotation, const Vector3 &vector) noexcept;

//! 使用变换转换点坐标
Point operator*(const Transform &transform, const Point &point) noexcept;

//! 使用变换转换位姿
Pose operator*(const Transform &transform, const Pose &pose) noexcept;

//! 复合两个变换，结果等价于先应用 @p rhs 再应用 @p lhs
Transform operator*(const Transform &lhs, const Transform &rhs) noexcept;

//! 计算变换的逆
Transform inverse(const Transform &transform) noexcept;

//! @} lpss_transform

} // namespace rm::msg

//! 坐标变换缓存、监听与广播功能命名空间
namespace rm::lpss::tf {

//! @defgroup lpss_transform_buffer 坐标变换缓存
//! @ingroup lpss
//! @{
//! @brief 提供坐标树维护、变换历史缓存和按时间查询能力
//! @details
//! `TransformStamped` 统一表示 \f$T_{parent,child}\f$：`child_frame_id` 在
//! `header.frame_id` 中的位姿，同时也将 child 中的坐标映射到 parent。
//! `Buffer::lookup(target, source)` 返回 \f$T_{target,source}\f$。

//! 坐标变换操作状态
enum class Status : uint8_t {
    Ok,                    //!< 操作成功
    InvalidArgument,       //!< 坐标系为空、父子同名或时间格式非法
    InvalidTransform,      //!< 变换包含非有限值或零范数四元数
    MultipleParents,       //!< 子坐标系已属于另一个父坐标系
    CycleDetected,         //!< 新变换会在坐标树中形成环
    StaticDynamicConflict, //!< 同一条边被同时用作静态和动态变换
    FrameNotFound,         //!< 坐标系不存在
    ConnectivityError,     //!< 两个坐标系不在同一棵树中
    ExtrapolationPast,     //!< 查询时间早于缓存最早时间
    ExtrapolationFuture,   //!< 查询时间晚于缓存最新时间
    NoCommonTime,          //!< 路径上的动态边没有公共时间区间
    TimestampOutOfRange,   //!< 插入时间已超出当前历史缓存窗口
};

//! 获取坐标变换状态的稳定文本描述
const char *to_string(Status status) noexcept;

//! 坐标变换查询结果
struct LookupResult {
    Status status{Status::FrameNotFound}; //!< 查询状态
    msg::TransformStamped transform{};    //!< 查询成功时的变换

    //! 查询是否成功
    explicit operator bool() const noexcept { return status == Status::Ok; }
};

/**
 * @brief 线程安全的坐标变换缓存
 * @details
 * - 坐标关系组成单父节点、无环的森林，其中，互不连通的树允许分别维护；
 * - 静态边保存单个变换，查询时不受时间限制，动态边按时间保存历史；
 * - 查询时间为零时使用整条路径上所有动态边的最新公共时间，全静态路径返回零时间；
 * - 动态变换使用平移线性插值和单位四元数 SLERP，不允许向缓存区间外外推。
 */
class Buffer {
public:
    //! 构造坐标变换缓存，默认保留最近 10 秒的动态历史
    explicit Buffer(std::chrono::nanoseconds cache_duration = std::chrono::seconds(10));

    Buffer(Buffer &&) noexcept;
    Buffer &operator=(Buffer &&) noexcept;
    ~Buffer();

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    /**
     * @brief 插入一条坐标变换
     * @param[in] transform 父子坐标变换，方向为 \f$T_{parent,child}\f$
     * @return 插入状态；同一时间的动态样本会被新值替换
     */
    Status set(const msg::TransformStamped &transform);

    /**
     * @brief 批量插入动态坐标变换
     * @return 首个失败状态；所有变换都会尝试插入
     */
    Status set(const msg::TF &transforms);

    //! 插入静态坐标变换
    Status setStatic(const msg::TransformStamped &transform);

    /**
     * @brief 批量插入静态坐标变换
     * @return 首个失败状态；所有变换都会尝试插入
     */
    Status setStatic(const msg::TF &transforms);

    /**
     * @brief 查询 source 坐标系到 target 坐标系的变换
     * @param[in] target_frame 目标坐标系
     * @param[in] source_frame 源坐标系
     * @param[in] time 查询时间；零时间表示最新公共时间
     * @return 查询结果，成功时得到 \f$T_{target,source}\f$
     */
    LookupResult lookup(std::string_view target_frame, std::string_view source_frame, const msg::Time &time = {}) const;

    //! 判断指定时间是否可完成坐标变换
    bool can(std::string_view target_frame, std::string_view source_frame, const msg::Time &time = {}) const;

    //! 设置动态历史缓存时长；负值按零处理，并立即裁剪现有历史
    void setCacheDuration(std::chrono::nanoseconds cache_duration) noexcept;

    //! 获取动态历史缓存时长
    std::chrono::nanoseconds cacheDuration() const noexcept;

    //! 获取当前坐标树边数
    std::size_t size() const noexcept;

    //! 清空全部静态和动态变换
    void clear() noexcept;

private:
    Status setImpl(const msg::TransformStamped &transform, bool is_static);

    class Impl;
    std::unique_ptr<Impl> _impl;
};

/**
 * @brief 动态坐标变换发布器
 * @details 发布到 `<name>/tf`，Node 的生命周期必须长于发布器。
 */
class Broadcaster {
public:
    /**
     * @brief 创建同步动态坐标变换发布器
     * @param[in] name 话题隔离名称，发布话题为 `<name>/tf`
     * @param[in] node 同步 LPSS 节点
     */
    Broadcaster(std::string_view name, Node &node);
#if __cplusplus >= 202002L
    /**
     * @brief 创建异步动态坐标变换发布器
     * @param[in] name 话题隔离名称，发布话题为 `<name>/tf`
     * @param[in] node 异步 LPSS 节点
     */
    Broadcaster(std::string_view name, async::Node &node);
#endif
    ~Broadcaster();

    Broadcaster(const Broadcaster &) = delete;
    Broadcaster &operator=(const Broadcaster &) = delete;

    //! 发布单条动态坐标变换
    void send(const msg::TransformStamped &transform);

    //! 批量发布动态坐标变换
    void send(const msg::TF &transforms);

    //! 判断底层发布器是否无效
    bool invalid() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

/**
 * @brief 静态坐标变换发布器
 * @details 发布到 `<name>/tf_static`，Node 的生命周期必须长于发布器。
 */
class StaticBroadcaster {
public:
    /**
     * @brief 创建同步静态坐标变换发布器
     * @param[in] name 话题隔离名称，发布话题为 `<name>/tf_static`
     * @param[in] node 同步 LPSS 节点
     */
    StaticBroadcaster(std::string_view name, Node &node);
#if __cplusplus >= 202002L
    /**
     * @brief 创建异步静态坐标变换发布器
     * @param[in] name 话题隔离名称，发布话题为 `<name>/tf_static`
     * @param[in] node 异步 LPSS 节点
     */
    StaticBroadcaster(std::string_view name, async::Node &node);
#endif
    ~StaticBroadcaster();

    StaticBroadcaster(const StaticBroadcaster &) = delete;
    StaticBroadcaster &operator=(const StaticBroadcaster &) = delete;

    //! 发布单条静态坐标变换
    void send(const msg::TransformStamped &transform);

    //! 批量发布静态坐标变换
    void send(const msg::TF &transforms);

    //! 判断底层发布器是否无效
    bool invalid() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

/**
 * @brief 坐标变换监听器
 * @details 订阅 `<name>/tf` 和 `<name>/tf_static` 并自动写入 Buffer。Buffer 与 Node 的生命周期必须长于监听器。
 */
class Listener {
public:
    /**
     * @brief 创建同步坐标变换监听器
     * @param[in] name 话题隔离名称
     * @param[in] node 同步 LPSS 节点
     * @param[in,out] buffer 接收动态和静态变换的缓存
     */
    Listener(std::string_view name, Node &node, Buffer &buffer);
#if __cplusplus >= 202002L
    /**
     * @brief 创建异步坐标变换监听器
     * @param[in] name 话题隔离名称
     * @param[in] node 异步 LPSS 节点
     * @param[in,out] buffer 接收动态和静态变换的缓存
     */
    Listener(std::string_view name, async::Node &node, Buffer &buffer);
#endif
    ~Listener();

    Listener(const Listener &) = delete;
    Listener &operator=(const Listener &) = delete;

    //! 判断任一底层订阅器是否无效
    bool invalid() const noexcept;

    //! 获取最近一批 TF 消息的插入状态
    Status status() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

//! @} lpss_transform_buffer

} // namespace rm::lpss::tf
