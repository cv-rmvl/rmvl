光源控制器 {#tutorial_modules_light}
============

@author 赵曦
@date 2023/10/05

@prev_tutorial{tutorial_modules_camera}

@next_tutorial{tutorial_modules_interpolation}

@tableofcontents

------

相关类

- 奥普特 GigE 光源控制器 rm::OPTLightController
- 海康机器人 RS-232 光源控制器 rm::HikLightController

### 1. 如何使用

海康机器人光源控制器使用 RS-232 串口进行数据传输，RMVL 在 Windows 和 Linux 平台上分别做了设计，可以很方便的开发，并且随附了 `rmvl_hik_lightctl` 命令行可执行程序。

奥普特 GigE 光源控制器在使用前需安装驱动，详情参考：@ref tutorial_install

### 2. 调试与开发

#### HikRobot 光源控制器可执行程序

`rmvl_hik_lightctl` 是一个命令行程序，用于控制海康机器人 RS-232 光源控制器，使用以下命令可以使用串口与光源控制器进行通信，其中 Windows 平台下的串口号为 `COM<?>`，Linux 平台下的串口号为 `/dev/ttyUSB<?>`，`<?>` 为具体的串口号，例如 `COM1` 或 `/dev/ttyUSB0`。

例如 Windows 平台可以使用以下命令：

```bash
rmvl_hik_lightctl COM1
```

而 Linux 平台可以使用以下命令：

```bash
rmvl_hik_lightctl /dev/ttyUSB0
```

在建立连接之后，可以输入 `help`、`h`、`usage` 或 `?` 来查看帮助信息，可以输入 `exit`、`quit` 或 `q` 来退出程序，具体的帮助信息如下：

<div class="fragment">
<div class="line"><span style="color: #569CD6">Common functions usage:</span></div>
<div class="line">&nbsp;&nbsp;h, help, ?, usage <span style="color: #6A9955"># show this help message</span></div>
<div class="line">&nbsp;&nbsp;exit, quit, q&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span style="color: #6A9955"># exit the program</span></div>
<div class="line"> </div>
<div class="line"><span style="color: #569CD6">Parameters control usage:</span></div>
<div class="line">&nbsp;&nbsp;ctl get delay&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span style="color: #6A9955"># get the delay time after writing</span></div>
<div class="line">&nbsp;&nbsp;ctl set delay &lt;val&gt; <span style="color: #6A9955"># set the delay time after writing</span></div>
<div class="line"> </div>
<div class="line"><span style="color: #569CD6">Commands usage:</span></div>
<div class="line">&nbsp;&nbsp;open&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span style="color: #6A9955"># open all the channels</span></div>
<div class="line">&nbsp;&nbsp;close&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span style="color: #6A9955"># close all the channels</span></div>
<div class="line">&nbsp;&nbsp;get &lt;chn&gt;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span style="color: #6A9955"># get the brightness of the specified channel</span></div>
<div class="line">&nbsp;&nbsp;set &lt;chn&gt; &lt;val&gt; <span style="color: #6A9955"># set the brightness of the specified channel</span></div>
</div>

#### OPT 光源控制器示例代码

不同的光源控制器有不同的使用方法，由于奥普特 GigE 光源控制器使用较复杂，这里以奥普特 GigE 光源控制器为例。

@add_toggle_cpp

```cpp
#include <rmvl/light/opt_light_control.h>

int main()
{
    // 创建光源控制器对象
    auto light_controller = rm::OPTLightController();
    // 连接光源控制器
    auto lipc = rm::OPTLightIpConfig{"192.168.1.100", "192.168.1.1", "255.255.255.0"};
    light_controller.connect(lipc);
    // 打开指定的通道
    light_controller.openChannels({1});
    // 设置光源强度
    light_controller.setIntensity(1, 100);
    // 触发光源 50×10ms = 500ms
    light_controller.trigger(1, 50);
}
```

@end_toggle

@add_toggle_python

```python
import rm

# 创建光源控制器对象
light_controller = rm.OPTLightController()
# 连接光源控制器
cfg = rm.OPTLightIpConfig()
cfg.ip = "192.168.1.100"
cfg.gateway = "192.168.1.1"
cfg.netmask = "255.255.255.0"
light_controller.connect(cfg)
# 打开指定的通道
light_controller.openChannels([1])
# 设置光源强度
light_controller.setIntensity(1, 100)
# 触发光源 50×10ms = 500ms
light_controller.trigger(1, 50)
```

@end_toggle
