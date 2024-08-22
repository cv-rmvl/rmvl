/**
 * @file opcua.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 模块汇总头文件
 * @version 1.0
 * @date 2023-10-22
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

/**
 * @defgroup opcua OPC UA 模块
 * @{
 * @brief 本模块基于 `open62541` 开发
 * @details
 * - 提供了 `OPC UA` 服务器、客户端的封装，以及 `OPC UA` 支持的数据类型包括
 *   `Object`、`Variable`、`Method`、`View`、`Event` 的定义。
 * - 关于如何使用此模块，可参考 @ref tutorial_modules_opcua 说明文档
 * @} opcua
 */

#include <rmvl/rmvl_modules.hpp>

#ifdef HAVE_RMVL_OPCUA
////////////////// Client/Server //////////////////
#include "opcua/client.hpp"
#include "opcua/server.hpp"
///////////////////// Pub/Sub /////////////////////
#ifdef UA_ENABLE_PUBSUB
#include "opcua/publisher.hpp"
#include "opcua/subscriber.hpp"
#endif // UA_ENABLE_PUBSUB
#endif // HAVE_RMVL_OPCUA
