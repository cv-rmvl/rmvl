/**
 * @file client.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 客户端
 * @version 2.1
 * @date 2024-03-07
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <open62541/client_subscriptions.h>

#include "object.hpp"
#include "view.hpp"

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
     * @brief 需要配合管道运算符 `|` 完成路径搜索
     * @code{.cpp}
     * auto dst_mode = src_node | clt.find("person") | clt.find("name");
     * @endcode
     *
     * @param[in] browse_name 浏览名
     * @return 目标节点信息
     * @retval fnic `[_client, browse_name]` 元组
     */
    inline FindNodeInClient find(const std::string &browse_name) { return {_client, browse_name}; }

    /****************************** 功能配置 ******************************/

    //! 是否成功创建客户端
    inline bool ok() const { return _client != nullptr; }

    /**
     * @brief 在网络上监听并处理到达的异步响应，同时进行内部维护、安全通道的更新和订阅管理
     * @brief
     * - 执行事件循环，等效于 ROS/ROS2 工具包中的 `ros::spin()` 以及 `rclcpp::spin()`
     */
    void spin();

    /**
     * @brief 在网络上监听并处理到达的异步响应，同时进行内部维护、安全通道的更新和订阅管理
     * @brief
     * - 处理当前已到来的事件，等效于 ROS/ROS2 工具包中的 `ros::spinOnce()` 以及 `rclcpp::spin_some()`
     */
    void spinOnce();

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
     * @brief 在客户端调用指定对象节点中的方法
     *
     * @param[in] obj_node 对象节点
     * @param[in] name 方法名
     * @param[in] inputs 输入参数列表
     * @param[out] outputs 输出参数列表
     * @return 是否成功完成当前操作
     */
    bool call(const UA_NodeId &obj_node, const std::string &name, const std::vector<Variable> &inputs, std::vector<Variable> &outputs);

    /**
     * @brief 在客户端调用 ObjectsFolder 中的方法
     *
     * @param[in] name 方法名
     * @param[in] inputs 输入参数列表
     * @param[out] outputs 输出参数列表
     * @return 是否成功完成当前操作
     */
    inline bool call(const std::string &name, const std::vector<Variable> &inputs, std::vector<Variable> &outputs)
    {
        return call(rm::nodeObjectsFolder, name, inputs, outputs);
    }

    /**
     * @brief 添加视图节点 ViewNode 至 `ViewsFolder` 中
     * 
     * @param[in] view `rm::View` 表示的视图
     * @return 添加至服务器后，对应视图节点的唯一标识 `UA_NodeId`
     */
    UA_NodeId addViewNode(const View &view);

    /**
     * @brief 创建变量节点监视项，以实现订阅节点的功能
     * @brief
     * - 服务器在设定的采样频率 `opcua_param.SAMPLING_INTERVAL`
     *   下监视变量，若发生更改会尝试发出通知，通知的发送频率受到
     *   `opcua_param.PUBLISHING_INTERVAL` 控制。当客户端收到通知时，执行
     *   `on_change` 回调函数
     * @brief
     * - 类似于 ROS 中的订阅话题，这里是订阅变量节点
     * @code{.cpp}
     * // on_change 回调函数的用法示例，假设订阅的变量节点为 Int32 类型
     * void on_change(UA_Client *client, UA_UInt32 sub_id, void *sub_context,
     *                UA_UInt32 mon_id, void *mon_context, UA_DataValue *value)
     * {
     *     UA_Int32 current_value = *reinterpret_cast<UA_Int32 *>(value->value.data);
     *     // 显示当前值
     *     UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "current value: %d", current_value);
     * }
     * @endcode
     *
     * @param[in] node 待监视节点的 `UA_NodeId`
     * @param[in] on_change 数据变更回调函数
     * @param[in] queue_size 通知存放的队列大小，若队列已满，新的通知会覆盖旧的通知
     * @return 变量节点监视创建成功？
     */
    bool monitor(UA_NodeId node, UA_Client_DataChangeNotificationCallback on_change, uint32_t queue_size);

    /**
     * @brief 创建事件监视项，以实现事件的订阅功能
     *
     * @param[in] node 待监视节点的 `UA_NodeId`
     * @param[in] names 关注的事件属性名列表，参考 Event::data()
     * @param[in] on_event 事件回调函数
     * @return 事件监视创建成功？
     */
    bool monitor(UA_NodeId node, const std::vector<std::string> &names, UA_Client_EventNotificationCallback on_event);

private:
    /**
     * @brief 发起订阅请求，并得到订阅 ID
     *
     * @param[out] response 订阅请求的响应
     * @return 是否成功完成当前操作
     */
    bool createSubscription(UA_CreateSubscriptionResponse &responce);
};

//! @} opcua

} // namespace rm