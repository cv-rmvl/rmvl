/**
 * @file io.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 数据 IO 与通信模块
 * @version 1.0
 * @date 2025-07-31
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

//! @defgroup io 数据 IO 与通信模块
//! @{
//!   @defgroup io_ipc 进程间通信
//!   @{
//!     @brief 提供跨平台的同步异步 **命名管道** 与 **消息队列** 通信功能
//!   @} io_ipc
//!   @defgroup io_serial 串口通信
//!   @{
//!     @brief 提供跨平台的同步异步 **串口** 通信功能
//!   @} io_serial
//!   @defgroup io_net 网络通信
//!   @{
//!     @brief 提供跨平台的同步异步 Socket 通信、HTTP 请求、HTTP Web 后端框架功能
//!   @} io_net
//! @} io

#include "io/util.hpp"

#include "io/ipc.hpp"
#include "io/netapp.hpp"
#include "io/serial.hpp"
#include "io/socket.hpp"
