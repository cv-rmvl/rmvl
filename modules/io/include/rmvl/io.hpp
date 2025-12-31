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
//!     @brief 提供跨平台的同步异步 **命名管道** 、 **消息队列** 以及 **共享内存** 的通信功能
//!     @details 进程间通信（Inter-Process Communication, IPC）是指在不同进程之间传递数据和消息的机制。常见的
//!              IPC 方式包括命名管道（Named Pipes）、消息队列（Message Queues）和共享内存（Shared Memory）。本模块提供了跨平台的
//!              IPC 功能，支持同步和异步操作，方便开发者在多进程应用中实现高效的数据交换与协作。
//!   @} io_ipc
//!   @defgroup io_serial 串口通信
//!   @{
//!     @brief 提供跨平台的同步异步 **串口** 通信功能
//!     @details 串口通信（Serial Communication）是一种通过串行接口进行数据传输的通信方式，广泛应用于嵌入式系统、工业自动化和设备控制等领域。
//!              本模块提供了跨平台的串口通信功能，支持同步和异步操作，方便开发者与各种串行设备进行高效的数据交换与控制。
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
