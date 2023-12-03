/**
 * @file subscriber.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 订阅者
 * @version 1.0
 * @date 2023-11-30
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include "server.hpp"

#ifdef UA_ENABLE_PUBSUB

namespace rm
{

//! @addtogroup opcua
//! @{

//! OPC UA 订阅者
class Subscriber
{
};

//! @} opcua

} // namespace rm

#endif // UA_ENABLE_PUBSUB
