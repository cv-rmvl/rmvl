/**
 * @file server.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 服务器
 * @version 2.2
 * @date 2024-03-29
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include <atomic>
#include <unordered_set>

#include "event.hpp"
#include "object.hpp"
#include "view.hpp"

namespace rm
{

//! @addtogroup opcua
//! @{

//! @example samples/opcua/opcua_server.cpp OPC UA 服务器例程

//! OPC UA 服务器视图
class RMVL_EXPORTS_W ServerView final
{
public:
    RMVL_W ServerView() = default;

    /**
     * @brief 创建不占有生命周期的 OPC UA 服务器视图，在 OPC UA 方法节点中使用特别有效
     *
     * @param[in] server OPC UA 服务器指针
     */
    ServerView(UA_Server *server) : _server(server) {}

    ServerView &operator=(UA_Server *const server)
    {
        _server = server;
        return *this;
    }

    //! 获取 OPC UA 服务器指针
    inline UA_Server *get() const { return _server; }

    /**
     * @brief 获取路径搜索必要信息
     * @brief 需要配合管道运算符 `|` 完成路径搜索
     * @code{.cpp}
     * auto dst_mode = src_node | srv.node("person") | srv.node("name");
     * @endcode
     *
     * @param[in] browse_name 浏览名
     * @param[in] ns 命名空间索引，默认为 `1`
     * @return 目标节点信息
     * @retval fnis `[_client, browse_name]` 元组
     */
    inline FindNodeInServer node(std::string_view browse_name, uint16_t ns = 1U) const { return {_server, browse_name, ns}; }

    /**
     * @brief 通过 BrowseName 的路径搜索命名空间 `ns` 为 `1` 的节点
     *
     * @param[in] browse_path BrowseName 路径，使用 `/` 分隔
     * @param[in] src_nd 源节点 ID，默认为 `rm::nodeObjectsFolder`
     * @return 节点 ID
     *
     * @code{.cpp}
     * auto node = srv.find("person/name", src_nd);
     * // 等效于 auto node = src_nd | srv.node("person") | srv.node("name");
     * @endcode
     */
    RMVL_W NodeId find(std::string_view browse_path, const NodeId &src_nd = nodeObjectsFolder) const noexcept;

    /**
     * @brief 从指定的变量节点读数据
     *
     * @param[in] nd 既存的变量节点的 `NodeId`
     * @return 读出的用 `rm::Variable` 表示的数据，未成功读取则返回空
     */
    RMVL_W Variable read(const NodeId &nd) const;

    /**
     * @brief 给指定的变量节点写数据
     *
     * @param[in] nd 既存的变量节点的 `NodeId`
     * @param[in] val 待写入的数据
     * @return 是否写入成功
     */
    RMVL_W bool write(const NodeId &nd, const Variable &val) const;

    /**
     * @brief 创建并触发事件
     *
     * @param[in] nd 触发事件的节点 `NodeId`
     * @param[in] event `rm::Event` 表示的事件
     * @return 是否创建并触发成功？
     */
    RMVL_W bool triggerEvent(const NodeId &nd, const Event &event) const;

private:
    UA_Server *_server{nullptr}; //!< OPC UA 服务器指针
};

/**
 * @brief 值回调函数，Read 可调用对象定义
 *
 * @param[in] server_view OPC UA 服务器视图，指代当前服务器
 * @param[in] node_id 待读取的变量节点的 `NodeId`
 * @param[in] value 服务器读取到的变量
 */
using ValueCallbackBeforeRead = std::function<void(ServerView, const NodeId &, const Variable &)>;

/**
 * @brief 值回调函数，Write 可调用对象定义
 *
 * @param[in] server_view OPC UA 服务器视图，指代当前服务器
 * @param[in] nodeid 待写入的变量节点的 `NodeId`
 * @param[in] data 服务器写入的变量
 */
using ValueCallbackAfterWrite = std::function<void(ServerView, const NodeId &, const Variable &)>;

/**
 * @brief 数据源回调函数，Read 函数指针定义
 *
 * @param[in] server_view OPC UA 服务器视图，指代当前服务器
 * @param[in] nodeid 待读取的变量节点的 `NodeId`
 * @return 向服务器提供的待读取的变量
 */
using DataSourceRead = std::function<Variable(ServerView, const NodeId &)>;

/**
 * @brief 数据源回调函数，Write 函数指针定义
 *
 * @param[in] server_view OPC UA 服务器视图，指代当前服务器
 * @param[in] nodeid 待写入的变量节点的 `NodeId`
 * @param[in] value 从服务器接收到的变量，一般用于写入外部数据
 */
using DataSourceWrite = std::function<void(ServerView, const NodeId &, const Variable &)>;

//! OPC UA 服务器
class RMVL_EXPORTS_W Server
{
public:
    using ValueCallbackWrapper = std::pair<ValueCallbackBeforeRead, ValueCallbackAfterWrite>;
    using DataSourceCallbackWrapper = std::pair<DataSourceRead, DataSourceWrite>;

    /****************************** 通用配置 ******************************/

    /**
     * @brief 创建 OPC UA 服务器
     *
     * @param[in] port OPC UA 服务器端口号，一般设置为 `4840U`
     * @param[in] name OPC UA 服务器名称，为空则采用默认值 `open62541-based OPC UA Application`
     * @param[in] users 用户列表 @see UserConfig
     */
    RMVL_W Server(uint16_t port, std::string_view name = {}, const std::vector<UserConfig> &users = {});

    /**
     * @brief 从服务器配置函数指针创建 OPC UA 服务器
     * @brief
     * - 服务器配置函数指针需提供 `*.xml` 文件，并由 `nodeset_compiler` 生成
     * - 关于 `*.xml` 文件的编写，参考 @ref opcua_nodeset_compiler
     *
     * @param[in] on_config 由 `nodeset_compiler` 生成的服务器配置函数指针
     * @param[in] port OPC UA 服务器端口号，一般设置为 `4840U`
     * @param[in] name OPC UA 服务器名称，为空则采用默认值 `open62541-based OPC UA Application`
     * @param[in] users 用户列表 @see UserConfig
     */
    Server(UA_StatusCode (*on_config)(UA_Server *), uint16_t port, std::string_view name = {}, const std::vector<UserConfig> &users = {});

    //! @cond
    Server(const Server &) = delete;
    Server(Server &&srv) = delete;

    Server &operator=(const Server &) = delete;
    Server &operator=(Server &&srv) = delete;
    //! @endcond

    RMVL_W operator ServerView() const { return _server; }

    /**
     * @brief 启动服务器并阻塞
     * @note
     * - 当 `rm::para::opcua_param.SERVER_WAIT` 为 `false` 时，该函数会立即返回，因此可在多线程环境下使用
     * @note
     * - 创建单线程事件循环，并在此事件循环中不断处理网络事件
     */
    void spin();

    /**
     * @brief 单次处理网络层中的重复回调和事件（单次启动服务器）
     * @note
     * - 当 `rm::para::opcua_param.SERVER_WAIT` 为 `false` 时，该函数会立即返回，因此可在多线程环境下使用
     * @note
     * - 这一部分可以在事件驱动的单线程架构中由外部主循环驱动
     */
    RMVL_W void spinOnce();

    //! 停止服务器
    RMVL_W inline void shutdown() { _running = false; }

    // 完成所有回调，停止网络层，进行清理并释放资源
    ~Server();

    /****************************** 路径搜索 ******************************/

    /**
     * @brief 获取路径搜索必要信息
     * @brief 需要配合管道运算符 `|` 完成路径搜索
     * @code{.cpp}
     * auto dst_mode = src_node | srv.node("person") | srv.node("name");
     * @endcode
     *
     * @param[in] browse_name 浏览名
     * @param[in] ns 命名空间索引，默认为 `1`
     * @return 目标节点信息
     * @retval fnis `[_client, browse_name]` 元组
     */
    inline FindNodeInServer node(std::string_view browse_name, uint16_t ns = 1U) const { return {_server, browse_name, ns}; }

    /**
     * @brief 通过 BrowseName 的路径搜索命名空间 `ns` 为 `1` 的节点
     *
     * @param[in] browse_path BrowseName 路径，使用 `/` 分隔
     * @param[in] src_nd 源节点 ID，默认为 `rm::nodeObjectsFolder`
     * @return 节点 ID
     *
     * @code{.cpp}
     * auto node = srv.find("person/name", src_nd);
     * // 等效于 auto node = src_nd | srv.node("person") | srv.node("name");
     * @endcode
     */
    RMVL_W NodeId find(std::string_view browse_path, const NodeId &src_nd = nodeObjectsFolder) const noexcept;

    /****************************** 功能配置 ******************************/

    /**
     * @brief 添加变量类型节点 VariableTypeNode 至 `BaseDataVariableType` 中
     *
     * @param[in] vtype `rm::VariableType` 表示的变量
     * @return 添加至服务器后，对应变量类型节点的唯一标识 `NodeId`
     */
    RMVL_W NodeId addVariableTypeNode(const VariableType &vtype);

    /**
     * @brief 添加变量节点 VariableNode 至指定父节点中，并指定引用类型
     *
     * @param[in] val `rm::Variable` 表示的变量
     * @param[in] parent_nd 指定父节点的 `NodeId`，默认为 `rm::nodeObjectsFolder`
     * @return 添加至服务器后，对应变量节点的唯一标识 `NodeId`
     */
    RMVL_W NodeId addVariableNode(const Variable &val, const NodeId &parent_nd = nodeObjectsFolder) noexcept;

    /**
     * @brief 为既有的变量节点 VariableNode 添加值回调
     * @brief 值回调表示在对 **服务器中的** 变量节点进行读写的时候，会在读之前执行 `beforeRead`，在写之后执行 `afterWrite`
     *
     * @param[in] nd 既有的变量节点的 `NodeId`，因变量节点可能位于任意一个父节点下，因此可以使用 **路径搜索** 进行查找
     * @param[in] before_read `ValueCallBackBeforeRead` 可调用对象
     * @param[in] after_write `ValueCallBackAfterWrite` 可调用对象
     * @return 是否添加成功
     */
    RMVL_W bool addVariableNodeValueCallback(NodeId nd, ValueCallbackBeforeRead before_read, ValueCallbackAfterWrite after_write) noexcept;

    /**
     * @brief 添加数据源变量节点 VariableNode 至指定父节点中
     * @brief 数据源变量节点不同于变量节点的值回调
     * @brief
     * - 值回调是在现有变量节点之上添加读取 **前** 和写入 **后** 的回调函数，本质上仍然是从服务器中获取数据
     * @brief
     * - 数据源变量节点会把每次 IO 都绑定到各自的回调函数中，即可以重定向到一个实际的物理过程中，从而跟服务器本身的数据读写脱离关系
     *
     * @param[in] val `rm::Variable` 表示的变量，仅取 `browse_name`、`description`、`display_name`、`access_level`
     *                以及 `ns` 字段，以及对应的变量类型节点
     * @param[in] on_read 重定向的读取回调函数
     * @param[in] on_write 重定向的写入回调函数
     * @param[in] parent_nd 指定父节点的 `NodeId`，默认为 `rm::nodeObjectsFolder`
     * @return 添加至服务器后，对应数据源变量节点的唯一标识 `NodeId`
     */
    RMVL_W NodeId addDataSourceVariableNode(const Variable &val, DataSourceRead on_read, DataSourceWrite on_write, NodeId parent_nd = nodeObjectsFolder) noexcept;

    /**
     * @brief 从指定的变量节点读数据
     *
     * @param[in] node 既存的变量节点的 `NodeId`
     * @return 读出的用 `rm::Variable` 表示的数据，未成功读取则返回空
     */
    RMVL_W Variable read(const NodeId &node) const;

    /**
     * @brief 给指定的变量节点写数据
     *
     * @param[in] node 既存的变量节点的 `NodeId`
     * @param[in] val 待写入的数据
     * @return 是否写入成功
     */
    RMVL_W bool write(const NodeId &node, const Variable &val);

    /**
     * @brief 添加方法节点 MethodNode 至指定父节点中
     *
     * @param[in] method `rm::Method` 表示的方法
     * @param[in] parent_nd 指定父节点的 `NodeId`，默认为 `rm::nodeObjectsFolder`
     * @return 添加至服务器后，对应方法节点的唯一标识 `NodeId`
     */
    RMVL_W NodeId addMethodNode(const Method &method, const NodeId &parent_nd = nodeObjectsFolder);

    /**
     * @brief 为既有的方法节点 MethodNode 设置方法的回调函数
     *
     * @param[in] nd 既有的方法节点的 `NodeId`，因方法节点可能位于任意一个父节点下，因此可以使用 **路径搜索** 进行查找
     * @param[in] on_method `MethodCallback` 可调用对象
     * @return 是否设置成功
     */
    RMVL_W bool setMethodNodeCallBack(const NodeId &nd, MethodCallback on_method);

    /**
     * @brief 添加对象类型节点 ObjectTypeNode 至 `rm::nodeBaseObjectType` 中
     *
     * @param[in] otype `rm::ObjectType` 表示的对象类型
     * @return 添加至服务器后，对应对象类型节点的唯一标识 `NodeId`
     */
    RMVL_W NodeId addObjectTypeNode(const ObjectType &otype);

    /**
     * @brief 添加对象节点 ObjectNode 至指定的父节点中
     *
     * @param[in] obj `rm::Object` 表示的对象
     * @param[in] parent_nd 指定的父节点 `NodeId`，默认为 `rm::nodeObjectsFolder`
     * @return 添加至服务器后，对应对象节点的唯一标识 `NodeId`
     */
    RMVL_W NodeId addObjectNode(const Object &obj, NodeId parent_nd = nodeObjectsFolder);

    /**
     * @brief 添加视图节点 ViewNode 至 `rm::nodeViewsFolder` 中
     *
     * @param[in] view `rm::View` 表示的视图
     * @return 添加至服务器后，对应视图节点的唯一标识 `NodeId`
     */
    RMVL_W NodeId addViewNode(const View &view);

    /**
     * @brief 添加事件类型至 `BaseEventType` 中
     *
     * @param[in] etype `rm::EventType` 表示的事件类型
     * @return 添加至服务器后，对应事件类型的唯一标识 `NodeId`
     */
    RMVL_W NodeId addEventTypeNode(const EventType &etype);

    /**
     * @brief 创建并触发事件
     *
     * @param[in] nd 触发事件的节点 `NodeId`
     * @param[in] event `rm::Event` 表示的事件
     * @return 是否创建并触发成功？
     */
    RMVL_W bool triggerEvent(const NodeId &nd, const Event &event) const;

protected:
    UA_Server *_server;          //!< OPC UA 服务器指针
    std::atomic_bool _running{}; //!< 服务器运行状态

private:
    mutable std::vector<std::unique_ptr<ValueCallbackWrapper>> _vcb_gc;       //!< 值回调函数
    mutable std::vector<std::unique_ptr<DataSourceCallbackWrapper>> _dscb_gc; //!< 数据源回调函数
    mutable std::vector<std::unique_ptr<MethodCallback>> _mcb_gc;             //!< 方法回调函数
};

//! OPC UA 服务器定时器
class RMVL_EXPORTS_W ServerTimer final
{
public:
    /**
     * @brief 创建 OPC UA 服务器定时器
     *
     * @param[in] sv 服务器视图
     * @param[in] period 定时器周期，单位：毫秒 `ms`
     * @param[in] callback 定时器回调函数
     */
    RMVL_W ServerTimer(ServerView sv, double period, std::function<void(ServerView)> callback);

    ServerTimer(const ServerTimer &) = delete;
    ServerTimer(ServerTimer &&) = default;

    ServerTimer &operator=(const ServerTimer &) = delete;
    ServerTimer &operator=(ServerTimer &&) = default;

    ~ServerTimer() { cancel(); }

    //! 取消定时器
    RMVL_W void cancel();

private:
    ServerView _sv;                      //!< 服务器视图
    std::function<void(ServerView)> _cb; //!< 定时器回调函数
    uint64_t _id{};                      //!< 定时器 ID
};

//! @} opcua

} // namespace rm
