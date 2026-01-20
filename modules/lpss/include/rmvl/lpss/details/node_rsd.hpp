/**
 * @file node_rsd.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief RMVL 发现协议定义实现
 * @version 1.0
 * @date 2025-12-04
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "rmvl/io/socket.hpp"

namespace rm::lpss {

//! @cond

//! GUID
union Guid {
    uint64_t full;

    struct {
        uint32_t host : 32;   //!< 主机 ID (MAC 低 4 字节)
        uint16_t pid : 16;    //!< 进程 ID
        uint16_t entity : 16; //!< 实体 ID
    } fields;

    // 构造函数
    Guid() : full(0) {}
    Guid(uint64_t val) : full(val) {}
    Guid(uint32_t h, uint16_t p, uint16_t e);

    // 运算符重载
    inline bool operator==(const Guid &oth) const { return full == oth.full; }
    inline bool operator!=(const Guid &oth) const { return full != oth.full; }
    inline bool operator<(const Guid &oth) const { return full < oth.full; }
};

//! GUID 哈希函数对象
struct GuidHash {
    inline std::size_t operator()(const rm::lpss::Guid &guid) const noexcept { return static_cast<std::size_t>(guid.full); }
};

constexpr std::size_t RNDP_HEADER_SIZE = 4 + sizeof(Guid) + 2;
constexpr std::size_t REDP_HEADER_SIZE = 4 + sizeof(Guid) + 2;
constexpr auto BROADCAST_IP = "239.255.0.5";

//! 定位器
struct Locator {
    uint16_t port{};               //!< REDP 端口号
    std::array<uint8_t, 4> addr{}; //!< 地址

    bool invalid() const noexcept { return port == 0 && addr == std::array<uint8_t, 4>{}; }
};

//! RNDP 节点发现消息
struct RNDPMessage {
    //! 序列化 RNDP 消息
    std::string serialize() const noexcept;

    //! 反序列化 RNDP 消息
    static RNDPMessage deserialize(const char *data) noexcept;

    Guid guid{};                     //!< 节点唯一标识符
    uint8_t heartbeat_timeout{};     //!< 心跳周期（单位：s）
    std::vector<Locator> locators{}; //!< 用于 REDP 的单播定位器列表
    std::string name{};              //!< 节点名称
};

//! REDP 通信端点发现消息
struct REDPMessage {
    //! 序列化 REDP 消息
    std::string serialize() const noexcept;

    //! 反序列化 REDP 消息
    static REDPMessage deserialize(const char *data) noexcept;

    //! 创建添加读取端点消息，其中 `l` 表示 DataReader 所监听的定位器
    static inline REDPMessage addReader(const Guid &g, std::string_view topic, uint16_t port) noexcept {
        return {Action::Add, Type::Reader, g, port, std::string(topic)};
    }

    //! 创建添加写入端点消息
    static inline REDPMessage addWriter(const Guid &g, std::string_view topic) noexcept {
        return {Action::Add, Type::Writer, g, {}, std::string(topic)};
    }

    //! 创建移除读取端点消息
    static inline REDPMessage removeReader(const Guid &g, std::string_view topic) noexcept {
        return {Action::Remove, Type::Reader, g, {}, std::string(topic)};
    }

    //! 创建移除写入端点消息
    static inline REDPMessage removeWriter(const Guid &g, std::string_view topic) noexcept {
        return {Action::Remove, Type::Writer, g, {}, std::string(topic)};
    }

    enum class Action : uint8_t {
        Remove = 0b00, //!< 端点移除
        Add = 0b01     //!< 端点更新
    } action{};        //!< 端点动作

    enum class Type : uint8_t {
        Reader = 0b00, //!< 数据读取端点
        Writer = 0b10  //!< 数据写入端点
    } type{};          //!< 端点类型

    Guid endpoint_guid{}; //!< 端点所属实体 GUID
    uint16_t port{};      //!< 监听端口，用于 UDPv4 通道
    std::string topic{};  //!< 话题名称，用于 SHM 通道
};

//! 节点存储信息
struct NodeStorageInfo {
    RNDPMessage rndp_msg{}; //!< RNDP 消息
    double last_alive{};    //!< 上次收到该节点消息的时间戳（单位：ms）
    Locator ctrl_loc{};     //!< 与该节点通信的最佳控制平面定位器
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
    void write(std::string_view data) noexcept;

protected:
    Guid _guid;               //!< 写入器所属实体 GUID
    DgramSocket _socket;      //!< UDPv4 通道 Socket
    std::string_view _type{}; //!< 消息类型
    std::string _topic{};     //!< 写入话题

    //! 保护目标列表的读写锁
    std::shared_mutex _mtx;
    //! 目标 UDPv4 定位器缓存集合
    std::unordered_map<Guid, Locator, GuidHash> _udpv4_targets;
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
    uint16_t _port{};         //!< 监听端口
    Guid _guid;               //!< 读取器所属实体 GUID
    DgramSocket _udpv4;       //!< UDPv4 通道
    std::string_view _type{}; //!< 消息类型
    std::string _topic{};     //!< 监听话题
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
    DataReader(const Guid &guid, std::string_view topic, Callback &&callback) : DataReaderBase(guid, MsgType::msg_type, topic) {
        _thrd = std::thread([this, cb = std::forward<Callback>(callback)]() {
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

void sendREDPMessage(Locator loc, const REDPMessage &msg);

//! @endcond

} // namespace rm::lpss
