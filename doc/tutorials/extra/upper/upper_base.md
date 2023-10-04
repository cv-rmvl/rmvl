顶层模块开发基本流程{#tutorial_extra_upper_base}
============

@author 赵曦
@date 2023/09/28

@prev_tutorial{tutorial_extra_how_to_use_decider}

@next_tutorial{tutorial_extra_upper_init}

@tableofcontents

------

### 写在前面

首先要清楚顶层模块主要负责实际项目中各个功能逻辑的安排与操控，具有确定的可执行程序，一般就是 `main.cpp` 文件所产生。在 @ref YAT 了解到 SRVL 2.x 系列及之前版本，顶层模块主要由视觉库本身维护，但视觉库本身不应该管理庞大的逻辑需求，应该只提供各类功能即可。因此，用户在使用 RMVL 的时候需要自行实现顶层模块。

### 基本流程

以下为 RMVL 顶层模块大致要具备的功能，以流程图形式展现

![upper-base](upper_base.png)

**注意：**

- 这里下位机一般指控制端，平台不固定，可以是嵌入式设备，例如 STM32 系列单片机，也可以是基于 ROS 的 ros-control 控制层
- 程序处理部分
  | 符号  |      含义      |
  | :---: | :------------: |
  |  `i`  |    传入参数    |
  | `io`  | 传入兼传出参数 |
  | `ret` |   函数返回值   |
- 流程图中给出了 4 个程序处理的基本顺序，实际上可以根据具体的逻辑需求选择使用其中的一种或多种程序处理模块，这些程序处理模块则刚好对应于 4 个 **功能模块** 。

------

顶层模块将会按照顺序对其中涉及到的模块进行使用上的介绍，请继续阅读下一篇： @ref tutorial_extra_upper_init 。
