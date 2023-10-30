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

#include "object.hpp"

namespace rm
{

//! @addtogroup opcua
//! @{

//! OPC UA 服务器
class Server final
{
    UA_Server *_server; //!< OPC UA 服务器指针
    bool _running{};    //!< 服务器运行状态
    std::thread _run;   //!< 服务器运行线程

    std::unordered_set<UA_Variant *> _variant_gc;   //!< UA_Variant 垃圾收集
    std::unordered_set<UA_Argument *> _argument_gc; //!< UA_Argument 垃圾收集

public:
    /****************************** 通用配置 ******************************/

    /**
     * @brief 创建 OPC UA 服务器
     *
     * @param[in] port OPC UA 服务器端口号，一般设置为 `4840U`
     * @param[in] users 用户列表 @see UserConfig
     */
    Server(uint16_t port, const std::vector<UserConfig> &users = {});

    Server(const Server &) = delete;
    Server(Server &&) = delete;

    //! 运行服务器
    void run();

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
     * auto dst_mode = src_node | svr.find(1, "person") | svr.find(1, "name");
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
     * @param[in] val `rm::Variable` 表示的变量
     * @return 添加后服务器中对应的变量类型节点唯一标识 `UA_NodeId`
     */
    UA_NodeId addVariableTypeNode(const VariableType &vtype);

    /**
     * @brief 添加变量节点 VariableNode 至指定父节点中
     *
     * @tparam Tp 变量节点数据类型
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
     * @brief 给指定的变量节点写数据
     * 
     * @param[in] node 既存的变量节点的 `NodeId`
     * @param[in] val 待写入的数据
     */
    void writeVariable(UA_NodeId node, const Variable &val);

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
    inline UA_NodeId addObjectNodeEx(const Object &obj) { return addObjectNodeEx(obj, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER)); }
};

//! @} opcua

} // namespace rm
