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

#include <thread>
#include <unordered_set>

#include <open62541/nodeids.h>

#include "event.hpp"
#include "object.hpp"
#include "view.hpp"

namespace rm
{

//! @addtogroup opcua
//! @{

//! @example samples/opcua/opcua_server.cpp OPC UA 服务器例程

//! 值回调函数，Read 函数指针定义
using ValueCallBackBeforeRead = void (*)(UA_Server *, const UA_NodeId *, void *, const UA_NodeId *, void *,
                                         const UA_NumericRange *, const UA_DataValue *);
//! 值回调函数，Write 函数指针定义
using ValueCallBackAfterWrite = void (*)(UA_Server *, const UA_NodeId *, void *, const UA_NodeId *, void *,
                                         const UA_NumericRange *, const UA_DataValue *);
//! 数据源回调函数，Read 函数指针定义
using DataSourceRead = UA_StatusCode (*)(UA_Server *, const UA_NodeId *, void *, const UA_NodeId *, void *,
                                         UA_Boolean, const UA_NumericRange *, UA_DataValue *);
//! 数据源回调函数，Write 函数指针定义
using DataSourceWrite = UA_StatusCode (*)(UA_Server *, const UA_NodeId *, void *, const UA_NodeId *, void *,
                                          const UA_NumericRange *, const UA_DataValue *);
//! 服务器配置函数指针，由 `nodeset_compiler` 生成
using ServerUserConfig = UA_StatusCode (*)(UA_Server *);

//! OPC UA 服务器
class Server
{
protected:
    UA_Server *_server; //!< OPC UA 服务器指针
    bool _running{};    //!< 服务器运行状态
    std::thread _run;   //!< 服务器运行线程

public:
    /****************************** 通用配置 ******************************/

    /**
     * @brief 创建 OPC UA 服务器
     *
     * @param[in] port OPC UA 服务器端口号，一般设置为 `4840U`
     * @param[in] name OPC UA 服务器名称，为空则采用默认值 `open62541-based OPC UA Application`
     * @param[in] users 用户列表 @see UserConfig
     */
    Server(uint16_t port, std::string_view name = {}, const std::vector<UserConfig> &users = {});

    /**
     * @brief 从服务器配置函数指针创建 OPC UA 服务器
     * @brief
     * - 服务器配置函数指针需提供 `*.xml` 文件，并由 `nodeset_compiler` 生成
     * - 关于 `*.xml` 文件的编写，参考 @ref opcua_nodeset_compiler
     *
     * @param[in] on_config 服务器配置函数指针
     * @param[in] port OPC UA 服务器端口号，一般设置为 `4840U`
     * @param[in] name OPC UA 服务器名称，为空则采用默认值 `open62541-based OPC UA Application`
     * @param[in] users 用户列表 @see UserConfig
     */
    Server(ServerUserConfig on_config, uint16_t port, std::string_view name = {}, const std::vector<UserConfig> &users = {});

    Server(const Server &) = delete;
    Server(Server &&svr) = default;

    //! 运行服务器，调用方线程不阻塞
    void start();

    //! 停止服务器
    inline void stop() { _running = false; }

    /**
     * @brief 阻塞
     * @brief
     * - 调用方线程阻塞，直到服务器执行 `stop()` 或以外停止后才继续运行
     */
    inline void join() { _run.join(); }

    //! 释放服务器资源
    inline ~Server() { deleteServer(); }

    /****************************** 路径搜索 ******************************/

    /**
     * @brief 获取路径搜索必要信息
     * @brief 需要配合管道运算符 `|` 完成路径搜索
     * @code{.cpp}
     * auto dst_mode = src_node | svr.find("person") | svr.find("name");
     * @endcode
     *
     * @param[in] browse_name 浏览名
     * @param[in] ns 命名空间索引，默认为 `1`
     * @return 目标节点信息
     * @retval fnis `[_client, browse_name]` 元组
     */
    inline FindNodeInServer find(const std::string &browse_name, uint16_t ns = 1U) { return {_server, browse_name, ns}; }

    /****************************** 功能配置 ******************************/

    /**
     * @brief 添加变量类型节点 VariableTypeNode 至 `BaseDataVariableType` 中
     *
     * @param[in] vtype `rm::VariableType` 表示的变量
     * @return 添加至服务器后，对应变量类型节点的唯一标识 `UA_NodeId`
     */
    UA_NodeId addVariableTypeNode(const VariableType &vtype);

    /**
     * @brief 添加变量节点 VariableNode 至指定父节点中，并指定引用类型
     *
     * @param[in] val `rm::Variable` 表示的变量
     * @param[in] parent_id 指定父节点的 `UA_NodeId`，默认为 `rm::nodeObjectsFolder`
     * @return 添加至服务器后，对应变量节点的唯一标识 `UA_NodeId`
     */
    UA_NodeId addVariableNode(const Variable &val, const UA_NodeId &parent_id = nodeObjectsFolder);

    /**
     * @brief 为既有的变量节点 VariableNode 添加值回调
     * @brief 值回调表示在对 **服务器中的** 变量节点进行读写的时候，会在读之前执行 `beforeRead`，在写之后执行 `afterWrite`
     *
     * @param[in] id 既有的变量节点的 `UA_NodeId`，因变量节点可能位于任意一个父节点下，因此可以使用 **路径搜索** 进行查找
     * @param[in] before_read 可隐式转换为 `ValueCallBackBeforeRead` 函数指针类型的可调用对象
     * @param[in] after_write 可隐式转换为 `ValueCallBackAfterWrite` 函数指针类型的可调用对象
     * @return 是否添加成功
     */
    bool addVariableNodeValueCallBack(UA_NodeId id, ValueCallBackBeforeRead before_read, ValueCallBackAfterWrite after_write);

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
     * @param[in] parent_id 指定父节点的 `UA_NodeId`，默认为 `rm::nodeObjectsFolder`
     * @return 添加至服务器后，对应数据源变量节点的唯一标识 `UA_NodeId`
     */
    UA_NodeId addDataSourceVariableNode(const Variable &val, DataSourceRead on_read, DataSourceWrite on_write, UA_NodeId parent_id = nodeObjectsFolder);

    /**
     * @brief 从指定的变量节点读数据
     *
     * @param[in] node 既存的变量节点的 `UA_NodeId`
     * @return 读出的用 `rm::Variable` 表示的数据，未成功读取则返回空
     */
    Variable read(const UA_NodeId &node);

    /**
     * @brief 给指定的变量节点写数据
     *
     * @param[in] node 既存的变量节点的 `UA_NodeId`
     * @param[in] val 待写入的数据
     * @return 是否写入成功
     */
    bool write(const UA_NodeId &node, const Variable &val);

    /**
     * @brief 添加方法节点 MethodNode 至指定父节点中
     *
     * @param[in] method `rm::Method` 表示的方法
     * @param[in] parent_id 指定父节点的 `UA_NodeId`，默认为 `rm::nodeObjectsFolder`
     * @return 添加至服务器后，对应方法节点的唯一标识 `UA_NodeId`
     */
    UA_NodeId addMethodNode(const Method &method, const UA_NodeId &parent_id = nodeObjectsFolder);

    /**
     * @brief 为既有的方法节点 MethodNode 设置方法的回调函数
     *
     * @param[in] id 既有的方法节点的 `UA_NodeId`，因方法节点可能位于任意一个父节点下，因此可以使用 **路径搜索** 进行查找
     * @param[in] on_method 可隐式转换为 `UA_MethodCallback` 函数指针类型的可调用对象
     */
    void setMethodNodeCallBack(const UA_NodeId &id, UA_MethodCallback on_method);

    /**
     * @brief 添加对象类型节点 ObjectTypeNode 至 `rm::nodeBaseObjectType` 中
     *
     * @param[in] otype `rm::ObjectType` 表示的对象类型
     * @return 添加至服务器后，对应对象类型节点的唯一标识 `UA_NodeId`
     */
    UA_NodeId addObjectTypeNode(const ObjectType &otype);

    /**
     * @brief 添加对象节点 ObjectNode 至指定的父节点中
     *
     * @param[in] obj `rm::Object` 表示的对象
     * @param[in] parent_id 指定的父节点 `UA_NodeId`，默认为 `rm::nodeObjectsFolder`
     * @return 添加至服务器后，对应对象节点的唯一标识 `UA_NodeId`
     */
    UA_NodeId addObjectNode(const Object &obj, UA_NodeId parent_id = nodeObjectsFolder);

    /**
     * @brief 添加视图节点 ViewNode 至 `rm::nodeViewsFolder` 中
     *
     * @param[in] view `rm::View` 表示的视图
     * @return 添加至服务器后，对应视图节点的唯一标识 `UA_NodeId`
     */
    UA_NodeId addViewNode(const View &view);

    /**
     * @brief 添加事件类型至 `BaseEventType` 中
     *
     * @param[in] etype `rm::EventType` 表示的事件类型
     * @return 添加至服务器后，对应事件类型的唯一标识 `UA_NodeId`
     */
    UA_NodeId addEventTypeNode(const EventType &etype);

    /**
     * @brief 创建并触发事件
     *
     * @param[in] node_id 触发事件的节点 `UA_NodeId`
     * @param[in] event `rm::Event` 表示的事件
     * @note `Server` 的 `node_id` 是 `rm::nodeServer`
     * @return 是否创建并触发成功？
     */
    bool triggerEvent(const UA_NodeId &node_id, const Event &event);

protected:
    //! 释放服务器资源
    void deleteServer();
};

//! @} opcua

} // namespace rm
