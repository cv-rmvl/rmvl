/**
 * @file client.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 客户端
 * @version 1.0
 * @date 2023-10-29
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include "object.hpp"

namespace rm
{

//! @addtogroup opcua
//! @{

//! OPC UA 客户端
class Client
{
    UA_Client *_client{nullptr}; //!< 客户端指针

public:
    /****************************** 通用配置 ******************************/

    /**
     * @brief 创建新的客户端对象，并建立连接
     *
     * @param[in] address 连接地址，形如 `opc.tcp://127.0.0.1:4840`
     * @param[in] usr 用户信息
     */
    Client(std::string_view address, UserConfig usr = {});

    ~Client();

    Client(const Client &) = delete;
    Client(Client &&) = delete;

    /****************************** 路径搜索 ******************************/

    /**
     * @brief 获取路径搜索必要信息
     *
     * @note 需要配合管道运算符 `|` 完成路径搜索
     * @code {.cpp}
     * auto dst_mode = src_node | clt.find("person") | clt.find("name");
     * @endcode
     *
     * @param[in] browse_name 浏览名
     * @return 目标节点信息
     * @retval `[_client, browse_name]`
     */
    inline findNodeInClient find(const std::string &browse_name) { return {_client, browse_name}; }

    /****************************** 功能配置 ******************************/

    /**
     * @brief 在网络上监听并处理到达的异步响应。同时进行内部维护、安全通道的更新和订阅管理。
     * @note 执行事件循环，等效于 ROS/ROS2 工具包中的 `ros::spin()` 以及 `rclcpp::spin()`
     */
    void spin();

    /**
     * @brief 在网络上监听并处理到达的异步响应。同时进行内部维护、安全通道的更新和订阅管理。
     * @note 处理当前已到来的事件，等效于 ROS/ROS2 工具包中的 `ros::spinOnce()` 以及 `rclcpp::spin_some()`
     */
    void spinOnce();

    /**
     * @brief 从指定的变量节点读数据
     * 
     * @param[in] node 既存的变量节点的 `UA_NodeId`
     * @param[out] val 读出的用 `rm::Variable` 表示的数据，未成功读取则返回空
     * @return 是否读取成功
     */
    bool read(UA_NodeId node, Variable &val);

    /**
     * @brief 给指定的变量节点写数据
     *
     * @param[in] node 既存的变量节点的 `UA_NodeId`
     * @param[in] val 待写入的数据
     * @return 是否写入成功
     */
    bool write(UA_NodeId node, const Variable &val);
};

//! @} opcua

} // namespace rm