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
     * auto dst_mode = src_node | clt.find(1, "person") | clt.find(1, "name");
     * @endcode
     *
     * @param[in] browse_name 浏览名
     * @return 目标节点信息
     * @retval `[_client, browse_name]`
     */
    inline findNodeInClient find(const std::string &browse_name) { return {_client, browse_name}; }

    /****************************** 功能配置 ******************************/
};

//! @} opcua

} // namespace rm