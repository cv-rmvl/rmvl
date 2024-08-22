光源控制器 {#tutorial_modules_light}
============

@author 赵曦
@date 2023/10/05

@prev_tutorial{tutorial_modules_camera}

@next_tutorial{tutorial_modules_interpolation}

@tableofcontents

------

相关类 rm::OPTLightController .

### 如何使用

使用前需安装光源控制器驱动，详情参考：@ref tutorial_install

### 示例代码

@add_toggle_cpp

```cpp
#include <rmvl/light/opt_light_control.h>

int main()
{
    // 创建光源控制器对象
    auto light_controller = rm::OPTLightController();
    // 连接光源控制器
    auto lipc = rm::LightIpConfig{"192.168.1.100", "192.168.1.1", "255.255.255.0"};
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
cfg = rm.LightIpConfig()
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
