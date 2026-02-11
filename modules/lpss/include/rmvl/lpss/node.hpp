/**
 * @file node.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 轻量发布订阅服务框架（Lightweight Pub/Sub Service）
 * @version 1.0
 * @date 2025-11-03
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#include <condition_variable>
#include <shared_mutex>
#include <thread>
#include <unordered_set>

#if __cplusplus >= 202002L
#include "rmvl/io/async.hpp"
#endif

#include "details/node_rsd.hpp"

namespace rm {

//! @addtogroup lpss
//! @{

//! 轻量发布订阅服务框架命名空间，包含节点、发布者、订阅者等相关定义
namespace lpss {

class Node;

/**
 * @brief 发布者代理
 *
 * @tparam MsgType 消息类型
 * @details
 * - 用户需使用 lpss::Node 的 `createPublisher` 方法创建发布者
 * - 发布者可使用 `publish` 方法发布消息到指定话题
 * - 在创建发布者后，自动注册到本地通信端点，并向所有通过 NDP 发现的节点发送 EDP 消息
 */
template <typename MsgType>
class Publisher final {
    friend class Node;

public:
    //! @cond
    Publisher(std::nullptr_t) {}

    /**
     * @brief 创建发布者
     *
     * @param[in] topic 话题名称
     * @param[in] writer 底层数据写入器
     */
    Publisher(std::string_view topic, DataWriterBase::ptr writer) : _topic(topic), _writer(std::move(writer)) {}
    //! @endcond

    //! 判断发布者是否无效
    bool invalid() const noexcept { return !_writer; }

    /**
     * @brief 发布消息到指定话题
     *
     * @param[in] msg 消息内容
     */
    void publish(const MsgType &msg);

private:
    DataWriterBase::ptr _writer{}; //!< 底层数据写入器
    std::string _topic{};          //!< 话题名称
};

/**
 * @brief 订阅者代理
 *
 * @tparam MsgType 消息类型
 * @details
 * - 用户需使用 lpss::Node 的 `createSubscriber` 方法创建订阅者
 * - 订阅者可使用 `subscribe` 方法 **重新** 订阅指定话题的消息
 * - 在创建订阅者后，自动注册到本地通信端点，并向所有通过 NDP 发现的节点发送 EDP 消息
 */
template <typename MsgType>
class Subscriber {
    friend class Node;

public:
    //! @cond
    Subscriber(std::nullptr_t) {}

    /**
     * @brief 创建订阅者
     *
     * @param[in] topic 话题名称
     * @param[in] reader 底层数据读取器
     */
    Subscriber(std::string_view topic, DataReaderBase::ptr reader) : _topic(topic), _reader(std::move(reader)) {}
    //! @endcond

    //! 判断订阅者是否无效
    bool invalid() const noexcept { return !_reader; }

private:
    DataReaderBase::ptr _reader; //!< 底层数据读取器
    std::string _topic;          //!< 话题名称
};

/**
 * @brief 轻量级发布订阅服务节点
 * @details
 * - 内置节点发现协议 NDP (Node Discovery Protocol)，用于节点间的自动发现与通信
 * - 内置通信端点发现协议 EDP (Endpoint Discovery Protocol)，用于发布者与订阅者间的自动发现与通信
 * @see 详情见 @ref tutorial_modules_lpss
 */
class Node {
public:
    /**
     * @brief 创建通用节点，默认域 ID 为 0
     *
     * @param[in] name 节点名称
     * @param[in] domain_id 域 ID
     */
    explicit Node(std::string_view name, uint8_t domain_id = 0);

    ~Node() noexcept { shutdown(); }

    //! 获取节点唯一标识符
    inline Guid guid() const noexcept { return _uid; }

    /**
     * @brief 创建发布者
     *
     * @tparam MsgType 消息类型
     * @param[in] topic 话题名称
     * @return Publisher<MsgType> 发布者对象
     */
    template <typename MsgType>
    Publisher<MsgType> createPublisher(std::string_view topic) noexcept;

    /**
     * @brief 创建订阅者
     *
     * @tparam MsgType 消息类型
     * @tparam SubscribeMsgCallback 订阅回调函数类型
     * @param[in] topic 话题名称
     * @param[in] callback 订阅回调函数
     * @return Subscriber<MsgType> 订阅者对象
     */
    template <typename MsgType, typename SubscribeMsgCallback, typename = std::enable_if_t<std::is_invocable_v<SubscribeMsgCallback, const MsgType &>>>
    Subscriber<MsgType> createSubscriber(std::string_view topic, SubscribeMsgCallback &&callback) noexcept;

    /**
     * @brief 销毁发布者
     *
     * @tparam MsgType 消息类型
     * @param[in] pub 发布者对象
     */
    template <typename MsgType>
    void destroyPublisher(const Publisher<MsgType> &pub);

    /**
     * @brief 销毁订阅者
     *
     * @tparam MsgType 消息类型
     * @param[in] sub 订阅者对象
     */
    template <typename MsgType>
    void destroySubscriber(const Subscriber<MsgType> &sub);

    //! 手动停止节点运行 @note 推荐使用析构函数自动清理资源并停止节点，除非在多线程环境中需要提前停止节点以释放资源
    void shutdown() noexcept;

private:
    //! 广播 NDP 消息
    void rndp_multicast(std::vector<ip::Networkv4> networks, std::string node_name);

    //! 监听 NDP 消息
    void rndp_listener(DgramSocket rndp_reader);

    //! 监听 EDP 消息
    void redp_listener(DgramSocket redp_socket);

    //! 心跳检测
    void heartbeat_detect();

    std::atomic_bool _running{true};   //!< 运行状态
    std::atomic_uint16_t _next_eid{1}; //!< 用于生成实体 ID 的原子计数器

    uint16_t _rndp_port{}; //!< RNDP 广播端口号
    uint16_t _redp_port{}; //!< REDP 监听端口号

    Guid _uid{};              //!< 节点唯一标识符
    DgramSocket _rndp_writer; //!< NDP 广播 Socket

    std::thread _bcast_thrd{};       //!< 广播线程
    std::thread _rndp_listen_thrd{}; //!< NDP 监听线程
    std::thread _redp_listen_thrd{}; //!< EDP 监听线程
    std::thread _hbt_detect_thrd{};  //!< 心跳检测线程
    std::mutex _hbt_mtx{};
    std::condition_variable _hbt_cv{};

protected:
    //! 节点读写锁
    std::shared_mutex _nodes_mtx{};
    //! 已发现的节点列表
    std::unordered_map<Guid, NodeStorageInfo, GuidHash> _discovered_nodes{};

    //! 本地实体读写锁
    std::shared_mutex _local_mtx{};
    //! 已注册的 Writers 列表
    std::unordered_map<std::string, DataWriterBase::ptr> _local_writers{};
    //! 已注册的 Readers 列表
    std::unordered_map<std::string, DataReaderBase::ptr> _local_readers{};

    //! 已发现实体读写锁
    std::shared_mutex _discovered_mtx{};
    //! 已发现的 Writers 列表，[Topic: [Guid]]
    std::unordered_map<std::string, std::unordered_set<Guid, GuidHash>> _discovered_writers{};
    //! 已发现的 Readers 列表，[Topic: [Guid: Locator]]
    std::unordered_map<std::string, std::unordered_map<Guid, Locator, GuidHash>> _discovered_readers{};
};

#if __cplusplus >= 202002L

//! 基于 C++20 协程的异步操作
namespace async {

class Node;

/**
 * @brief 异步发布者代理
 *
 * @tparam MsgType 消息类型
 * @details
 * - 用户需使用 lpss::async::Node 的 `createPublisher` 方法创建发布者
 * - 发布者可使用 `publish` 方法发布消息到指定话题
 * - 在创建发布者后，自动注册到本地通信端点，并向所有通过 NDP 发现的节点发送 EDP 消息
 */
template <typename MsgType>
class Publisher final {
    friend class Node;

public:
    using ptr = std::shared_ptr<Publisher<MsgType>>;

    //! @cond
    Publisher(std::nullptr_t) {}

    /**
     * @brief 创建发布者
     *
     * @param[in] io_context 异步 IO 上下文
     * @param[in] topic 话题名称
     * @param[in] writer 底层数据写入器
     */
    Publisher(rm::async::IOContext &io_context, std::string_view topic, DataWriterBase::ptr writer) : _ctx(io_context), _writer(std::move(writer)), _topic(topic) {}
    //! @endcond

    //! 判断发布者是否无效
    bool invalid() const noexcept { return !_writer; }

    /**
     * @brief 发布消息到指定话题
     *
     * @param[in] msg 消息内容
     */
    void publish(const MsgType &msg);

private:
    rm::async::IOContextRef _ctx; //!< 异步 IO 上下文引用
    DataWriterBase::ptr _writer;  //!< 底层数据写入器
    std::string _topic;           //!< 话题名称
};

/**
 * @brief 异步订阅者代理
 *
 * @tparam MsgType 消息类型
 * @details
 * - 用户需使用 lpss::async::Node 的 `createSubscriber` 方法创建订阅者
 * - 在创建订阅者后，自动注册到本地通信端点，并向所有通过 NDP 发现的节点发送 EDP 消息
 */
template <typename MsgType>
class Subscriber {
    friend class Node;

public:
    using ptr = std::shared_ptr<Subscriber<MsgType>>;

    //! @cond
    Subscriber(std::nullptr_t) {}

    /**
     * @brief 创建订阅者
     *
     * @param[in] io_context 异步 IO 上下文
     * @param[in] topic 话题名称
     * @param[in] reader 底层数据读取器
     */
    Subscriber(rm::async::IOContext &io_context, std::string_view topic, DataReaderBase::ptr reader) : _ctx(io_context), _reader(std::move(reader)), _topic(topic) {}
    //! @endcond

    //! 判断订阅者是否无效
    bool invalid() const noexcept { return !_reader; }

private:
    rm::async::IOContextRef _ctx; //!< 异步 IO 上下文引用
    DataReaderBase::ptr _reader;  //!< 底层数据读取器
    std::string _topic;           //!< 话题名称
};

//! 异步定时器代理
class Timer : public rm::async::Timer {
public:
    using ptr = std::shared_ptr<Timer>;

    //! @cond
    /**
     * @brief 创建异步定时器
     *
     * @param[in] io_context 异步 IO 上下文
     */
    Timer(rm::async::IOContext &io_context) : rm::async::Timer(io_context) {}
    //! @endcond
};

/**
 * @brief 轻量级发布订阅服务异步节点
 * @details
 * - 内置节点发现协议 NDP (Node Discovery Protocol)，用于节点间的自动发现与通信
 * - 内置通信端点发现协议 EDP (Endpoint Discovery Protocol)，用于发布者与订阅者间的自动发现与通信
 * @see 详情见 @ref tutorial_modules_lpss
 */
class Node {
public:
    /**
     * @brief 创建通用节点，默认域 ID 为 0
     *
     * @param[in] name 节点名称
     * @param[in] domain_id 域 ID
     */
    explicit Node(std::string_view name, uint8_t domain_id = 0);

    ~Node() noexcept { shutdown(); }

    //! 获取节点唯一标识符
    inline Guid guid() const noexcept { return _uid; }

    /**
     * @brief 创建发布者
     *
     * @tparam MsgType 消息类型
     * @param[in] topic 话题名称
     * @return 发布者对象的智能指针
     */
    template <typename MsgType>
    typename Publisher<MsgType>::ptr createPublisher(std::string_view topic) noexcept;

    /**
     * @brief 创建订阅者
     *
     * @tparam MsgType 消息类型
     * @tparam SubscribeMsgCallback 订阅回调函数类型
     * @param[in] topic 话题名称
     * @param[in] callback 订阅回调函数
     * @return 订阅者对象的智能指针
     */
    template <typename MsgType, typename SubscribeMsgCallback, typename = std::enable_if_t<std::is_invocable_v<SubscribeMsgCallback, const MsgType &>>>
    typename Subscriber<MsgType>::ptr createSubscriber(std::string_view topic, SubscribeMsgCallback callback) noexcept;

    /**
     * @brief 销毁发布者
     *
     * @tparam MsgType 消息类型
     * @param[in] pub 发布者对象
     */
    template <typename MsgType>
    void destroyPublisher(typename Publisher<MsgType>::ptr pub);

    /**
     * @brief 销毁订阅者
     *
     * @tparam MsgType 消息类型
     * @param[in] sub 订阅者对象
     */
    template <typename MsgType>
    void destroySubscriber(typename Subscriber<MsgType>::ptr sub);

    /**
     * @brief 创建异步定时器
     *
     * @tparam Rep 定时器时间间隔的表示类型
     * @tparam Period 定时器时间间隔的周期类型
     * @tparam TimerCallback 定时器回调函数类型
     * @param[in] dur 定时器时间间隔
     * @param[in] callback 定时器回调函数
     * @return 定时器对象的智能指针
     */
    template <typename Rep, typename Period, typename TimerCallback>
    Timer::ptr createTimer(std::chrono::duration<Rep, Period> dur, TimerCallback callback) noexcept;

    //! 运行异步 IO 上下文
    void spin() { _ctx.run(); }

    //! 手动停止节点运行 @note 推荐使用析构函数自动清理资源并停止节点，除非在多线程环境中需要提前停止节点以释放资源
    void shutdown() noexcept;

private:
    //! 广播 NDP 消息
    rm::async::Task<> rndp_multicast(std::vector<ip::Networkv4> networks, std::string node_name);

    //! 监听 NDP 消息
    rm::async::Task<> rndp_listener(rm::async::DgramSocket rndp_reader);

    //! 监听 EDP 消息
    rm::async::Task<> redp_listener(rm::async::DgramSocket redp_socket);

    //! 心跳检测
    rm::async::Task<> heartbeat_detect();

    //! 处理 SIGINT 信号
    rm::async::Task<> on_sigint();

protected:
    rm::async::IOContext _ctx{}; //!< 异步 IO 上下文

private:
    bool _running{true};   //!< 运行状态
    uint16_t _next_eid{1}; //!< 用于生成实体 ID 的计数器

    uint16_t _rndp_port{}; //!< RNDP 广播端口号
    uint16_t _redp_port{}; //!< REDP 监听端口号

    Guid _uid{};                         //!< 节点唯一标识符
    rm::async::DgramSocket _rndp_writer; //!< NDP 广播 Socket

    rm::async::Timer _broadcast_timer{_ctx}; //!< NDP 广播异步定时器
    rm::async::Timer _hbt_timer{_ctx};       //!< 心跳检测异步定时器

protected:
    //! 已发现的节点列表
    std::unordered_map<Guid, NodeStorageInfo, GuidHash> _discovered_nodes{};

    //! 已注册的 Writers 列表
    std::unordered_map<std::string, DataWriterBase::ptr> _local_writers{};
    //! 已注册的 Readers 列表
    std::unordered_map<std::string, DataReaderBase::ptr> _local_readers{};

    //! 已发现的 Writers 列表，[Topic: [Guid]]
    std::unordered_map<std::string, std::unordered_set<Guid, GuidHash>> _discovered_writers{};
    //! 已发现的 Readers 列表，[Topic: [Guid: Locator]]
    std::unordered_map<std::string, std::unordered_map<Guid, Locator, GuidHash>> _discovered_readers{};
};

} // namespace async

#endif

} // namespace lpss

//! @} lpss

} // namespace rm

#include "details/node_impl.hpp"
