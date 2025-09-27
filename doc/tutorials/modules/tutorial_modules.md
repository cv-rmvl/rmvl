主要模块使用教程 {#tutorial_table_of_content_modules}
============

@prev_tutorial{tutorial_table_of_content_config}

@next_tutorial{tutorial_table_of_content_rmvlpara}

@tableofcontents

------

### 1. 工具类

#### 开发工具

- @subpage tutorial_modules_aggregate_reflect

#### 基础通信设施

- @subpage tutorial_modules_coro
- @subpage tutorial_modules_ipc
- @subpage tutorial_modules_serial
- @subpage tutorial_modules_socket
- @subpage tutorial_modules_netapp

#### 通信中间件

这是在分布式系统中，位于操作系统和应用程序之间的软件层，主要用于

1. 抽象化通信复杂性：隐藏底层网络通信细节，提供统一的 API 接口，处理网络连接、断线重连等
2. 标准化数据交换：定义统一的数据格式，处理数据序列化 / 反序列化，支持不同系统间的互操作
3. 增强系统可靠性：提供消息队列、持久化，支持事务处理，错误处理和恢复机制
4. 简化开发工作：提供高级编程接口，减少网络编程复杂度，支持多种编程语言和平台

- @subpage tutorial_modules_opcua
- @subpage tutorial_modules_mqtt

#### 硬件设备支持库

- @subpage tutorial_modules_camera
- @subpage tutorial_modules_light

### 2. 算法类

#### 数值计算

- @subpage tutorial_modules_interpolation
- @subpage tutorial_modules_least_square 和 @subpage tutorial_modules_lsqnonlin
- @subpage tutorial_modules_func_iteration
- @subpage tutorial_modules_runge_kutta
- @subpage tutorial_modules_auto_differential
- @subpage tutorial_modules_fminbnd
- @subpage tutorial_modules_fminunc

#### 数据与信号处理

- @subpage tutorial_modules_ew_topsis
- @subpage tutorial_modules_kalman 和 @subpage tutorial_modules_ekf
- @subpage tutorial_modules_dft 和 @subpage tutorial_modules_fft

#### 数据结构与算法

- @subpage tutorial_modules_union_find
- @subpage tutorial_modules_ra_heap

#### 机器学习与深度学习支持库

- @subpage tutorial_modules_ort
