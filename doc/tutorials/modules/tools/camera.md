相机设备 {#tutorial_modules_camera}
============

@author 赵曦
@date 2023/03/15
@brief 相机模块的基本使用，包括初始化、参数设置以及运行示例

@prev_tutorial{tutorial_modules_opcua}

@next_tutorial{tutorial_modules_light}

@tableofcontents

------

相关类
- 迈德威视 USB3.0 工业相机 rm::MvCamera
- 海康机器人 USB3.0/GigE 工业相机 rm::HikCamera
- 奥普特机器视觉 USB3.0/GigE 工业相机 rm::OptCamera
- 大恒图像 Galaxy USB3.0/GigE 工业相机 rm::GalaxyCamera

## 1. 如何使用

使用前需安装相机驱动，详情参考：@ref tutorial_install ，下面以 MvCamera 为例介绍如何使用相机模块，其余相机操作完全一致。

### 1.1 初始化

创建 MvCamera 对象即可初始化相机：

@add_toggle_cpp

```cpp
auto cam_cfg = rm::CameraConfig::create(rm::GrabMode::Continuous, rm::RetrieveMode::SDK);
rm::MvCamera capture(cam_cfg, "0123456789");
```

@end_toggle

@add_toggle_python

```python
cam_cfg = rm.CameraConfig()
cam_cfg.grab_mode = rm.GrabMode.Continuous
cam_cfg.retrieve_mode = rm.RetrieveMode.SDK

capture = rm.MvCamera(cam_cfg, "0123456789")
```
@end_toggle

第 1 个参数为 **相机初始化模式** ，包含

- 相机外部触发通道
- 相机采集模式
- 相机句柄创建方式
- 相机数据处理模式

4 个待配置的数据，见下表

<table class="markdownTable">
  <tr class="markdownTableHead">
    <th class="markdownTableHeadCenter" width="140">初始化配置模式</th>
    <th class="markdownTableHeadCenter" width="120">含义</th>
    <th class="markdownTableHeadCenter" width="190">标识符</th>
    <th class="markdownTableHeadCenter">功能</th>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter" rowspan="4">相机外部触发通道</td>
    <td class="markdownTableBodyCenter">通道 0</td>
    <td class="markdownTableBodyCenter"><code>rm::TriggerChannel::Chn0</code></td>
    <td class="markdownTableBodyCenter">通道 0 生效</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyCenter">通道 1</td>
    <td class="markdownTableBodyCenter"><code>rm::TriggerChannel::Chn1</code></td>
    <td class="markdownTableBodyCenter">通道 1 生效</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter">通道 2</td>
    <td class="markdownTableBodyCenter"><code>rm::TriggerChannel::Chn2</code></td>
    <td class="markdownTableBodyCenter">通道 2 生效</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyCenter">通道 3</td>
    <td class="markdownTableBodyCenter"><code>rm::TriggerChannel::Chn3</code></td>
    <td class="markdownTableBodyCenter">通道 3 生效</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter" rowspan="3">采集方式</td>
    <td class="markdownTableBodyCenter">连续采集</td>
    <td class="markdownTableBodyCenter"><code>rm::GrabMode::Continuous</code></td>
    <td class="markdownTableBodyCenter">连续触发相机，当 <code>grab</code> 方法被执行后，相机将开始连续采集，一般调用 <code>read</code>
      或在相机构造之初可自动开启相机 Grabbing。</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyCenter">软触发</td>
    <td class="markdownTableBodyCenter"><code>rm::GrabMode::Software</code></td>
    <td class="markdownTableBodyCenter">软件触发，需要手动设置触发帧，当相机开始取流（Grabbing）时，只有在被设置触发帧之后的下一次执行有效，否则会阻塞以等待接收到触发帧，RMVL 相机库目前仅支持同步的相机数据处理，在指定时间内如果没有收到触发帧，则会被认为相机读取 <code>read</code> 失败。</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter">硬触发</td>
    <td class="markdownTableBodyCenter"><code>rm::GrabMode::Hardware</code></td>
    <td class="markdownTableBodyCenter">
      硬件触发，通过向相机的航空接头等串行通信接口传输高低电平信号来设置触发帧，信号的有效性可通过软件设置，例如设置高电平、低电平、上升边沿、下降边沿的一种为触发方式，有关相机取流的细节同软触发。</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyCenter" rowspan="4">相机句柄模式</td>
    <td class="markdownTableBodyCenter">索引号</td>
    <td class="markdownTableBodyCenter"><code>rm::HandleMode::Index</code></td>
    <td class="markdownTableBodyCenter">相机的索引号 `(0, 1, 2 ...)`</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter">序列号</td>
    <td class="markdownTableBodyCenter"><code>rm::HandleMode::Key</code></td>
    <td class="markdownTableBodyCenter">制造商：序列号 S/N</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyCenter">MAC</td>
    <td class="markdownTableBodyCenter"><code>rm::HandleMode::MAC</code></td>
    <td class="markdownTableBodyCenter">相机的 MAC 地址</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter">IP 地址</td>
    <td class="markdownTableBodyCenter"><code>rm::HandleMode::IP</code></td>
    <td class="markdownTableBodyCenter">IP 地址，形如 <code>192.168.1.100</code></td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyCenter" rowspan="2">相机数据处理模式</td>
    <td class="markdownTableBodyCenter">使用 OpenCV</td>
    <td class="markdownTableBodyCenter"><code>rm::RetrieveMode::OpenCV</code></td>
    <td class="markdownTableBodyCenter">OpenCV 的 <code>imgproc</code> 模块提供了有关数据处理的接口，例如 <code>cv::cvtColor</code> 可以用于色彩空间转换</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter">使用厂商 SDK</td>
    <td class="markdownTableBodyCenter"><code>rm::RetrieveMode::SDK</code></td>
    <td class="markdownTableBodyCenter">SDK 中提供了有关数据处理的接口，主要用于相机解码、色彩空间转换等操作</td>
  </tr>
</table>

详细的触发方式 @see

- <a href="https://vision.scutbot.cn/hik/index.html" target="_blank"> HIKROBOT 工业相机用户手册</a>

- <a href="https://vision.scutbot.cn/mv/mv.pdf" target="_blank"> 迈德威视工业相机用户手册</a>

### 1.2 光学属性设置

#### 1.2.1 曝光设置

手动/自动设置曝光

@add_toggle_cpp

```cpp
capture.set(rm::CAMERA_MANUAL_EXPOSURE); // 手动曝光
capture.set(rm::CAMERA_AUTO_EXPOSURE);   // 自动曝光
```

@end_toggle

@add_toggle_python

```python
capture.set(rm.CAMERA_MANUAL_EXPOSURE) # 手动曝光
capture.set(rm.CAMERA_AUTO_EXPOSURE)   # 自动曝光
```

@end_toggle

设置曝光值

@add_toggle_cpp

```cpp
capture.set(rm::CAMERA_EXPOSURE, 600); // 设置曝光值为 600
```

@end_toggle

@add_toggle_python

```python
capture.set(rm.CAMERA_EXPOSURE, 600) # 设置曝光值为 600
```

@end_toggle

#### 1.2.2 白平衡设置

手动/自动设置白平衡

@add_toggle_cpp

```cpp
capture.set(rm::CAMERA_MANUAL_WB); // 手动白平衡
capture.set(rm::CAMERA_AUTO_WB);   // 自动白平衡
```

@end_toggle

@add_toggle_python

```python
capture.set(rm.CAMERA_MANUAL_WB) # 手动白平衡
capture.set(rm.CAMERA_AUTO_WB)   # 自动白平衡
```

@end_toggle

设置各通道增益，并生效（在手动设置白平衡模式下有效）

@add_toggle_cpp

```cpp
capture.set(rm::CAMERA_WB_RGAIN, 102); // 红色通道增益设置为 102
capture.set(rm::CAMERA_WB_GGAIN, 101); // 绿色通道增益设置为 101
capture.set(rm::CAMERA_WB_BGAIN, 100); // 蓝色通道增益设置为 100
```

@end_toggle

@add_toggle_python

```python
capture.set(rm.CAMERA_WB_RGAIN, 102) # 红色通道增益设置为 102
capture.set(rm.CAMERA_WB_GGAIN, 101) # 绿色通道增益设置为 101
capture.set(rm.CAMERA_WB_BGAIN, 100) # 蓝色通道增益设置为 100
```

@end_toggle

#### 1.2.3 其余光学参数设置

@add_toggle_cpp

```cpp
capture.set(rm::CAMERA_GAIN, 64);        // 设置模拟增益为 64
capture.set(rm::CAMERA_GAMMA, 80);       // 设置 Gamma 为 80
capture.set(rm::CAMERA_CONTRAST, 120);   // 设置对比度为 120
capture.set(rm::CAMERA_SATURATION, 100); // 设置饱和度为 100
capture.set(rm::CAMERA_SHARPNESS, 100);  // 设置锐度为 100
```

@end_toggle

@add_toggle_python

```python
capture.set(rm.CAMERA_GAIN, 64)        # 设置模拟增益为 64
capture.set(rm.CAMERA_GAMMA, 80)       # 设置 Gamma 为 80
capture.set(rm.CAMERA_CONTRAST, 120)   # 设置对比度为 120
capture.set(rm.CAMERA_SATURATION, 100) # 设置饱和度为 100
capture.set(rm.CAMERA_SHARPNESS, 100)  # 设置锐度为 100
```

@end_toggle

@note Hik 工业相机暂不支持修改 Gamma

### 1.3 触发属性设置

@add_toggle_cpp

```cpp
// 设置硬触发采集延迟为 1000 μs，仅在硬触发模式下有效
capture.set(rm::CAMERA_TRIGGER_DELAY, 1000);
// 设置单次触发时的触发帧数为 5 帧，即一次触发能触发 5 帧画面，仅在触发模式下有效
capture.set(rm::CAMERA_TRIGGER_COUNT, 5);
// 设置单次触发时多次采集的周期为 100 μs，即一次触发信号能触发多帧画面，每帧间隔为 100 μs
capture.set(rm::CAMERA_TRIGGER_PERIOD, 100);
// 执行一次白平衡操作，仅在手动白平衡模式下有效
capture.set(rm::CAMERA_ONCE_WB);
// 执行一次软触发，仅在软触发模式下有效
capture.set(rm::CAMERA_TRIGGER_SOFT);
```

@end_toggle

@add_toggle_python

```python
# 设置硬触发采集延迟为 1000 μs，仅在硬触发模式下有效
capture.set(rm.CAMERA_TRIGGER_DELAY, 1000)
# 设置单次触发时的触发帧数为 5 帧，即一次触发能触发 5 帧画面，仅在触发模式下有效
capture.set(rm.CAMERA_TRIGGER_COUNT, 5)
# 设置单次触发时多次采集的周期为 100 μs，即一次触发信号能触发多帧画面，每帧间隔为 100 μs
capture.set(rm.CAMERA_TRIGGER_PERIOD, 100)
# 执行一次白平衡操作，仅在手动白平衡模式下有效
capture.set(rm.CAMERA_ONCE_WB)
# 执行一次软触发，仅在软触发模式下有效
capture.set(rm.CAMERA_TRIGGER_SOFT)
```

@end_toggle

## 2. para 参数加载

RMVL 提供了全局的相机参数对象: para::camera_param ，详情可参考类 para::CameraParam

## 3. 示例程序

在构建 RMVL 时，需开启 `BUILD_EXAMPLES` 选项（默认开启）

```bash
cmake -DBUILD_EXAMPLES=ON ..
cmake --build . --parallel 4
cd build
```

### 3.1 单相机

单相机例程，在 build 文件夹下执行以下命令

```bash
bin/rmvl_mv_mono
```

相机按照连续采样、`cvtColor` 处理方式运行，程序运行中，`cv::waitKey(1)` 接受到 `s` 键被按下时，可将参数保存到 `out_para.yml` 文件中。

键入一次 `Esc` 可暂停程序，按其余键可恢复。键入两次 `Esc` 可退出程序。

### 3.2 多相机

多相机例程，在 `build` 文件夹下执行以下命令

```bash
bin/rmvl_mv_multi
```

相机按照连续采样、`cvtColor` 处理方式运行，程序会枚举所有的相机设备，并可视化的显示出来，指定一个序列号来启动某一相机。

程序运行过程中，相机参数会自动从 `out_para.yml` 中加载，若没有则会按照默认值运行。

键入一次 `Esc` 可暂停程序，按其余键可恢复。键入两次 `Esc` 可退出程序。

### 3.3 相机录屏

相机录屏例程，在 build 文件夹下执行以下命令

```bash
bin/rmvl_mv_writer
```

相机按照连续采样、`cvtColor` 处理方式运行，`-o` 可指定输出文件名，否则默认输出到 `ts.avi`，例如

```bash
bin/rmvl_mv_writer -o=aaa.avi
```

程序运行过程中，相机参数会自动从 `out_para.yml` 中加载，若没有则会按照默认值运行。

### 3.4 相机标定

MvCamera 相机自动标定程序，在 `build` 文件夹下执行以下命令

```bash
bin/rmvl_mv_auto_calib -w=<?> -h=<?> -s=<?> -d=<?> -n=<?>
```

`<?>` 表示可调节，具体帮助可直接执行以下命令

```bash
bin/rmvl_mv_calibration -help
```

另外还有相机手动标定程序，可执行以下命令

```bash
bin/rmvl_mv_manual_calib
```

## 4. 使用 Demo

### 4.1 连续采样

@add_toggle_cpp

```cpp
#include <opencv2/highgui.hpp>

#include <rmvl/camera/mv_camera.h>

int main()
{
    rm::MvCamera capture(rm::CameraConfig::create(rm::GrabMode::Continuous, rm::RetrieveMode::OpenCV));
    cv::Mat frame;
    while(capture.read(frame))
    {
        cv::imshow("frame", frame);
        if (cv::waitKey(1) == 27)
            break;
    }
}
```

@end_toggle

@add_toggle_python

```python
import rm
import cv2

cam_cfg = rm.CameraConfig()
cam_cfg.grab_mode = rm.GrabMode.Continuous
cam_cfg.retrieve_mode = rm.RetrieveMode.OpenCV

capture = rm.MvCamera(cam_cfg)
while True:
    res, frame = capture.read()
    if not res:
        break
    cv2.imshow("frame", frame)
    if cv2.waitKey(1) == 27:
        break
```

@end_toggle

### 4.2 软触发

@add_toggle_cpp

```cpp
#include <opencv2/highgui.hpp>

#include <rmvl/camera/mv_camera.h>
#include <rmvl/core/timer.hpp>

int main()
{
    auto camera_config = rm::CameraConfig::create(rm::GrabMode::Software, rm::RetrieveMode::OpenCV)
    rm::MvCamera capture(camera_config);

    bool run = true;
    std::thread th([&run]() {
        while (run)
        {
            Timer::sleep_for(10);
            capture.set(rm::CAMERA_TRIGGER_SOFT); // 触发
        }
    });

    cv::Mat frame;
    while (capture.read(frame))
    {
        cv::imshow("frame", frame);
        if (cv::waitKey(1) == 27)
        {
            run = false;
            break;
        }
    }
    th.join();
}
```

@end_toggle

@add_toggle_python

@todo
Python 软触发示例暂无

@end_toggle
