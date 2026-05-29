/**
 * @file node_rmtp.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2026-05-27
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "node_util.hpp"

namespace rm::lpss {

//! @cond

//! MTP 数据发送目标
struct MTPWriterTarget {
    Locator locator{};  //!< 目标定位器
    uint32_t mtu{1500}; //!< 发送目标对应的本地接口 MTU
};

//! MTP 重组缓存索引
struct MTPAsmKey {
    std::string addr{};
    uint16_t port{};
    uint16_t sequence{};

    bool operator==(const MTPAsmKey &other) const noexcept { return addr == other.addr && port == other.port && sequence == other.sequence; }
};

struct MTPAsmKeyHash {
    std::size_t operator()(const MTPAsmKey &key) const noexcept {
        std::size_t seed = std::hash<std::string>{}(key.addr);
        seed ^= static_cast<std::size_t>(key.port) + 0x9e3779b9U + (seed << 6) + (seed >> 2);
        seed ^= static_cast<std::size_t>(key.sequence) + 0x9e3779b9U + (seed << 6) + (seed >> 2);
        return seed;
    }
};

//! MTP 未完成载荷
struct MTPAsm {
    uint32_t total_size{};
    std::map<uint16_t, std::string> fragments{};
    std::size_t bytes{};
    std::chrono::steady_clock::time_point updated{};
};

/**
 * @brief 数据写入器基类
 * @details 每个 DataWriter 都对应一个动态分配端口的 UDPv4 通道以及设置指定话题的 MPMC SHM 通道
 */
class DataWriterBase {
public:
    using ptr = std::shared_ptr<DataWriterBase>;

    /**
     * @brief 创建数据写入器基类
     *
     * @param[in] guid 含 Entity ID 的 GUID
     * @param[in] type 消息类型，使用 `<MsgType>::msg_type` 获取
     * @param[in] topic 写入话题，用于共享内存通道
     */
    DataWriterBase(const Guid &guid, std::string_view type, std::string_view topic);

    virtual ~DataWriterBase() = default;

    //! 获取写入器所属实体 GUID
    inline const Guid &guid() const noexcept { return _guid; }

    //! 获取写入话题的消息类型
    inline std::string_view msgtype() const noexcept { return _type; }

    /**
     * @brief 添加数据接收端点
     *
     * @param[in] guid 端点所属实体 GUID，根据 GUID MAC 区分 SHM 或 UDPv4 通道
     * @param[in] loc 端点监听定位器，用于 UDPv4 通道，端口部分同样作为 SHM 通道标识
     */
    void add(const Guid &guid, Locator loc) noexcept;

    /**
     * @brief 移除数据接收端点
     *
     * @param[in] guid 端点所属实体 GUID
     */
    void remove(const Guid &guid) noexcept;

    /**
     * @brief 写入数据
     *
     * @param[in] data 待写入的数据
     */
    void write(std::string data) noexcept;

protected:
    Guid _guid;               //!< 写入器所属实体 GUID
    DgramSocket _socket;      //!< UDPv4 通道 Socket
    std::string_view _type{}; //!< 消息类型
    std::string _topic{};     //!< 写入话题

    //! 保护目标列表的读写锁
    std::shared_mutex _mtx;
    //! 目标 UDPv4 定位器缓存集合
    std::unordered_map<Guid, MTPWriterTarget, GuidHash> _udpv4_targets;
    std::atomic_uint16_t _sequence{}; //!< MTP 发送序列号
};

//! 发现的发布者端点存储信息
struct DiscoveredWriterStorageInfo {
    std::unordered_set<Guid, GuidHash> writers; //!< 相关的端点 GUID 列表
    std::string msgtype;                        //!< 消息类型
};

/**
 * @brief 数据读取器基类
 * @details 每个 DataReader 都对应一个监听端口的 UDPv4 通道以及设置指定话题的 MPMC SHM 通道
 */
class DataReaderBase {
public:
    using ptr = std::shared_ptr<DataReaderBase>;

    /**
     * @brief 创建数据读取器基类
     *
     * @param[in] guid 含 Entity ID 的 GUID
     * @param[in] type 消息类型，使用 `<MsgType>::msg_type` 获取`
     * @param[in] topic 监听话题，用于共享内存通道
     */
    DataReaderBase(const Guid &guid, std::string_view type, std::string_view topic);

    virtual ~DataReaderBase() = default;

    //! 获取监听话题的消息类型
    inline std::string_view msgtype() const noexcept { return _type; }

    /**
     * @brief 读取数据
     *
     * @return 读取到的数据
     */
    std::string read() noexcept;

    //! 获取读取器所属实体 GUID
    inline const Guid &guid() const noexcept { return _guid; }

    //! 获取监听的端口
    inline uint16_t port() const noexcept { return _port; }

protected:
    uint16_t _port{};                                             //!< 监听端口
    Guid _guid;                                                   //!< 读取器所属实体 GUID
    DgramSocket _udpv4;                                           //!< UDPv4 通道
    std::string_view _type{};                                     //!< 消息类型
    std::string _topic{};                                         //!< 监听话题
    std::unordered_map<MTPAsmKey, MTPAsm, MTPAsmKeyHash> _asms{}; //!< MTP 重组缓存
    std::size_t _asm_bytes{};                                     //!< 待重组载荷占用字节数
};

/**
 * @brief 底层数据写入器
 * @details
 * - 需使用发布功能，请使用 `lpss::Node` 的 `create_publisher` 方法创建发布者
 * @see lpss::Publisher
 *
 * @tparam MsgType
 */
template <typename MsgType>
class DataWriter : public DataWriterBase {
public:
    DataWriter(const Guid &guid, std::string_view topic) : DataWriterBase(guid, MsgType::msg_type, topic) {}
};

/**
 * @brief 底层数据读取器
 * @details
 * - 需使用订阅功能，请使用 `lpss::Node` 的 `create_subscriber` 方法创建订阅者
 * @see lpss::Subscriber
 *
 * @tparam MsgType 消息类型
 */
template <typename MsgType>
class DataReader : public DataReaderBase {
public:
    /**
     * @brief 创建数据读取器
     *
     * @param[in] guid 含 Entity ID 的 GUID
     * @param[in] topic 监听话题，用于共享内存通道，UDPv4 通道的监听端口自动分配
     * @param[in] callback 消息回调函数
     */
    template <typename Callback, typename = std::enable_if_t<std::is_invocable_v<Callback, const MsgType &>>>
    DataReader(const Guid &guid, std::string_view topic, Callback callback) : DataReaderBase(guid, MsgType::msg_type, topic) {
        _thrd = std::thread([this, cb = std::move(callback)]() {
            while (true) {
                auto data = this->read();
                if (data.empty())
                    continue;
                cb(MsgType::deserialize(data.data()));
            }
        });
    }

private:
    std::thread _thrd; //!< 读取线程
};

#if __cplusplus >= 202002L

namespace async {

/**
 * @brief 异步数据写入器基类
 * @details 每个 async::DataWriter 都对应一个动态分配端口的 UDPv4 通道以及设置指定话题的 MPMC SHM 通道
 */
class DataWriterBase {
public:
    using ptr = std::shared_ptr<DataWriterBase>;

    /**
     * @brief 创建数据写入器基类
     *
     * @param[in] io_context IO 上下文
     * @param[in] guid 含 Entity ID 的 GUID
     * @param[in] type 消息类型，使用 `<MsgType>::msg_type` 获取
     * @param[in] topic 写入话题，用于共享内存通道
     */
    DataWriterBase(rm::async::IOContext &io_context, const Guid &guid, std::string_view type, std::string_view topic);

    virtual ~DataWriterBase() = default;

    //! 获取写入器所属实体 GUID
    inline const Guid &guid() const noexcept { return _guid; }

    //! 获取写入话题的消息类型
    inline std::string_view msgtype() const noexcept { return _type; }

    /**
     * @brief 添加数据接收端点
     *
     * @param[in] guid 端点所属实体 GUID，根据 GUID MAC 区分 SHM 或 UDPv4 通道
     * @param[in] loc 端点监听定位器，用于 UDPv4 通道，端口部分同样作为 SHM 通道标识
     */
    void add(const Guid &guid, Locator loc) noexcept;

    /**
     * @brief 移除数据接收端点
     *
     * @param[in] guid 端点所属实体 GUID
     */
    void remove(const Guid &guid) noexcept;

    /**
     * @brief 写入数据
     *
     * @param[in] data 待写入的数据
     */
    rm::async::Task<> write(std::string data) noexcept;

protected:
    Guid _guid;                     //!< 写入器所属实体 GUID
    rm::async::DgramSocket _socket; //!< UDPv4 通道 Socket
    std::string_view _type{};       //!< 消息类型
    std::string _topic{};           //!< 写入话题

    //! 目标 UDPv4 定位器缓存集合
    std::unordered_map<Guid, MTPWriterTarget, GuidHash> _udpv4_targets;
    std::atomic_uint16_t _sequence{};       //!< MTP 发送序列号
    std::optional<std::string> _pending{};  //!< 发送中收到的最新待发送消息
    bool _sending{};                        //!< 是否已有发送协程正在运行
};

/**
 * @brief 异步数据读取器基类
 * @details 每个 async::DataReader 都对应一个监听端口的 UDPv4 通道以及设置指定话题的 MPMC SHM 通道
 */
class DataReaderBase {
public:
    using ptr = std::shared_ptr<DataReaderBase>;

    /**
     * @brief 创建数据读取器基类
     *
     * @param[in] io_context IO 上下文
     * @param[in] guid 含 Entity ID 的 GUID
     * @param[in] type 消息类型，使用 `<MsgType>::msg_type` 获取
     * @param[in] topic 监听话题，用于共享内存通道
     */
    DataReaderBase(rm::async::IOContext &io_context, const Guid &guid, std::string_view type, std::string_view topic);

    virtual ~DataReaderBase() = default;

    //! 获取监听话题的消息类型
    inline std::string_view msgtype() const noexcept { return _type; }

    /**
     * @brief 读取数据
     *
     * @return 读取到的数据
     */
    rm::async::Task<std::string> read() noexcept;

    //! 获取读取器所属实体 GUID
    inline const Guid &guid() const noexcept { return _guid; }

    //! 获取监听的端口
    inline uint16_t port() const noexcept { return _port; }

protected:
    uint16_t _port{};                                             //!< 监听端口
    Guid _guid;                                                   //!< 读取器所属实体 GUID
    rm::async::DgramSocket _udpv4;                                //!< UDPv4 通道
    std::string_view _type{};                                     //!< 消息类型
    std::string _topic{};                                         //!< 监听话题
    std::unordered_map<MTPAsmKey, MTPAsm, MTPAsmKeyHash> _asms{}; //!< MTP 重组缓存
    std::size_t _asm_bytes{};                                     //!< 待重组载荷占用字节数
};

/**
 * @brief 底层数据写入器
 * @details
 * - 需使用发布功能，请使用 `lpss::Node` 的 `create_publisher` 方法创建发布者
 * @see lpss::Publisher
 *
 * @tparam MsgType
 */
template <typename MsgType>
class DataWriter : public DataWriterBase {
public:
    DataWriter(rm::async::IOContext &io_context, const Guid &guid, std::string_view topic) : DataWriterBase(io_context, guid, MsgType::msg_type, topic) {}
};

/**
 * @brief 底层数据读取器
 * @details
 * - 需使用订阅功能，请使用 `lpss::Node` 的 `create_subscriber` 方法创建订阅者
 * @see lpss::Subscriber
 *
 * @tparam MsgType 消息类型
 */
template <typename MsgType>
class DataReader : public DataReaderBase {
public:
    /**
     * @brief 创建数据读取器
     *
     * @param[in] guid 含 Entity ID 的 GUID
     * @param[in] topic 监听话题，用于共享内存通道，UDPv4 通道的监听端口自动分配
     * @param[in] callback 消息回调函数
     */
    template <typename Callback, typename = std::enable_if_t<std::is_invocable_v<Callback, const MsgType &>>>
    DataReader(rm::async::IOContext &io_context, const Guid &guid, std::string_view topic, Callback callback) : DataReaderBase(io_context, guid, MsgType::msg_type, topic) {
        co_spawn(io_context, &DataReader<MsgType>::read_task<Callback>, this, std::move(callback));
    }

private:
    template <typename Callback, typename = std::enable_if_t<std::is_invocable_v<Callback, const MsgType &>>>
    rm::async::Task<> read_task(Callback cb) {
        while (true) {
            auto data = co_await this->read();
            if (data.empty())
                continue;
            cb(MsgType::deserialize(data.data()));
        }
    }
};

} // namespace async

#endif

//! @endcond

} // namespace rm::lpss
