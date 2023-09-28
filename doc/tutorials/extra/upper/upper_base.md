基本使用流程{#tutorial_extra_upper_base}
============

@author 赵曦
@date 2023/09/28

@prev_tutorial{tutorial_extra_how_to_use_decider}

@next_tutorial{tutorial_extra_upper_init}

@tableofcontents

------

### 写在前面

首先要清楚顶层模块主要负责实际项目中各个功能逻辑的安排与操控，具有确定的可执行程序，一般就是 `main.cpp` 文件所产生。在 @ref YAT 中可以得知 SRVL 2.x 系列及之前版本，顶层模块主要由 SRVL 本身维护，但视觉库本身不应该管理庞大的逻辑需求，应该只提供各类功能即可。因此，用户在使用 RMVL 的时候需要配套使用自己的顶层模块。

### 基本流程


