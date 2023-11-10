/**
 * @file server.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 服务器
 * @version 1.0
 * @date 2023-10-21
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <thread>
#include <unordered_set>

#include <open62541/nodeids.h>

#include "object.hpp"

namespace rm
{

//! @addtogroup opcua
//! @{

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

//! OPC UA 服务器
class Server final
{
    UA_Server *_server; //!< OPC UA 服务器指针
    bool _running{};    //!< 服务器运行状态
    std::thread _run;   //!< 服务器运行线程

public:
    /****************************** 通用配置 ******************************/

    /**
     * @brief 创建 OPC UA 服务器
     *
     * @param[in] port OPC UA 服务器端口号，一般设置为 `4840U`
     * @param[in] users 用户列表 @see UserConfig
     */
    explicit Server(uint16_t port, const std::vector<UserConfig> &users = {});

    Server(const Server &) = delete;
    Server(Server &&) = delete;

    //! 运行服务器
    void start();

    //! 停止服务器
    inline void stop() { _running = false; }

    /**
     * @brief 阻塞
     * @brief
     * - 调用方线程阻塞，直到服务器停止后才继续运行
     */
    inline void join() { _run.join(); }

    ~Server();

    /****************************** 路径搜索 ******************************/

    /**
     * @brief 获取路径搜索必要信息
     *
     * @note 需要配合管道运算符 `|` 完成路径搜索
     * @code {.cpp}
     * auto dst_mode = src_node | svr.find("person") | svr.find("name");
     * @endcode
     *
     * @param[in] browse_name 浏览名
     * @return 目标节点信息
     * @retval `[_server, browse_name]`
     */
    inline FindNodeInServer find(const std::string &browse_name) { return {_server, browse_name}; }

    /****************************** 功能配置 ******************************/

    /**
     * @brief 添加变量类型节点 VariableTypeNode 至 `BaseDataVariableType` 中
     *
     * @param[in] vtype `rm::VariableType` 表示的变量
     * @return 添加后服务器中对应的变量类型节点唯一标识 `UA_NodeId`
     */
    UA_NodeId addVariableTypeNode(const VariableType &vtype);

    /**
     * @brief 添加变量节点 VariableNode 至指定父节点中
     *
     * @param[in] val `rm::Variable` 表示的变量
     * @param[in] parent_id 指定父节点的 `UA_NodeId`
     * @return 添加后服务器中对应的变量节点唯一标识 `UA_NodeId`
     */
    UA_NodeId addVariableNodeEx(const Variable &val, UA_NodeId parent_id);

    /**
     * @brief 添加变量节点 VariableNode 至 `ObjectsFolder` 中
     *
     * @tparam Tp 变量节点数据类型
     * @param[in] val `rm::Variable` 表示的变量
     * @return 添加后服务器中对应的变量节点唯一标识 `UA_NodeId`
     */
    inline UA_NodeId addVariableNode(const Variable &val) { return addVariableNodeEx(val, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER)); }

    /**
     * @brief 为既有的变量节点 VariableNode 添加值回调
     *
     * @note 值回调表示在对 **服务器中的** 变量节点进行读写的时候，会在读之前执行 `beforeRead`，在写之后执行 `afterWrite`
     * @param[in] id 既有的变量节点的 `UA_NodeId`
     * - 因变量节点可能位于任意一个父节点下，因此可以使用 **路径搜索** 进行查找
     * @param[in] before_read 可隐式转换为 `ValueCallBackBeforeRead` 函数指针类型的可调用对象
     * @param[in] after_write 可隐式转换为 `ValueCallBackAfterWrite` 函数指针类型的可调用对象
     * @return 是否添加成功
     */
    bool addVariableNodeValueCallBack(UA_NodeId id, ValueCallBackBeforeRead before_read, ValueCallBackAfterWrite after_write);

    /**
     * @brief 添加数据源变量节点 VariableNode 至指定父节点中
     *
     * @note
     * 数据源变量节点不同于变量节点的值回调
     * @note
     * - 值回调是在现有变量节点之上添加读取 **前** 和写入 **后** 的回调函数，本质上仍然是从服务器中获取数据
     * @note
     * - 数据源变量节点会把每次 IO 都绑定到各自的回调函数中，即可以重定向到一个实际的物理过程中，从而跟服务器本身的数据读写脱离关系
     * @param[in] val `rm::Variable` 表示的变量
     * @param[in] on_read 重定向的读取回调函数
     * @param[in] on_write 重定向的写入回调函数
     * @param[in] parent_id 指定父节点的 `UA_NodeId`
     * @return 添加后服务器中对应的数据源变量节点唯一标识 `UA_NodeId`
     */
    UA_NodeId addDataSourceVariableNodeEx(const Variable &val, DataSourceRead on_read, DataSourceWrite on_write, UA_NodeId parent_id);

    /**
     * @brief 添加数据源变量节点 VariableNode 至 `ObjectsFolder` 中
     *
     * @see addDataSourceVariableNodeEx
     * @param[in] val `rm::Variable` 表示的变量
     * @param[in] on_read 重定向的读取回调函数
     * @param[in] on_write 重定向的写入回调函数
     * @return 添加后服务器中对应的数据源变量节点唯一标识 `UA_NodeId`
     */
    inline UA_NodeId addDataSourceVariableNode(const Variable &val, DataSourceRead on_read, DataSourceWrite on_write)
    {
        return addDataSourceVariableNodeEx(val, on_read, on_write, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));
    }

    /**
     * @brief 从指定的变量节点读数据
     * 
     * @param[in] node 既存的变量节点的 `UA_NodeId`
     * @param[out] val 读出的用 `rm::Variable` 表示的数据，未成功读取则返回空
     * @return 是否读取成功
     */
    bool read(const UA_NodeId &node, Variable &val);

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
     * @param[in] parent_id 指定父节点的 `UA_NodeId`
     * @return 添加后服务器中对应的方法节点唯一标识 `UA_NodeId`
     */
    UA_NodeId addMethodNodeEx(const Method &method, UA_NodeId parent_id);

    /**
     * @brief 添加方法节点 MethodNode 至 `ObjectsFolder` 中
     *
     * @param[in] method `rm::Method` 表示的方法
     * @return 添加后服务器中对应的方法节点唯一标识 `UA_NodeId`
     */
    inline UA_NodeId addMethodNode(const Method &method) { return addMethodNodeEx(method, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER)); };

    /**
     * @brief 添加对象类型节点 ObjectTypeNode 至 `BaseObjectType` 中
     *
     * @param[in] otype `rm::ObjectType` 表示的对象类型
     * @return 添加后服务器中对应的对象类型节点唯一标识 `UA_NodeId`
     */
    UA_NodeId addObjectTypeNode(const ObjectType &otype);

    /**
     * @brief 添加对象节点 ObjectNode 至指定的父节点中
     *
     * @param[in] obj `rm::Object` 表示的对象
     * @param[in] parent_id 指定的父节点 `UA_NodeId`
     *
     * @return 添加后服务器中对应的对象节点唯一标识 `UA_NodeId`
     */
    UA_NodeId addObjectNodeEx(const Object &obj, UA_NodeId parent_id);

    /**
     * @brief 添加对象节点 ObjectNode 至 `ObjectsFolder` 中
     *
     * @param[in] obj `rm::Object` 表示的对象
     *
     * @return 添加后服务器中对应的对象节点唯一标识 `UA_NodeId`
     */
    inline UA_NodeId addObjectNode(const Object &obj) { return addObjectNodeEx(obj, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER)); }
};

//! @} opcua

} // namespace rm
