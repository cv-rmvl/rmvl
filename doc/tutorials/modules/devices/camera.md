相机设备 {#tutorial_modules_camera}
============

@author 赵曦
@date 2023/03/15
@brief 相机模块的基本使用，包括初始化、参数设置以及运行示例

@prev_tutorial{tutorial_modules_ort}

@tableofcontents

------

相关类
- 迈德威视工业相机 rm::MvCamera
- 海康机器人工业相机 rm::HikCamera
- 奥普特机器视觉工业相机 rm::OptCamera

## 1. 如何使用

使用前需安装相机驱动，详情参考：@ref tutorial_install

### 1.1 初始化

@add_toggle{Mv}

创建 MvCamera 对象即可初始化相机，例如：

```cpp
rm::MvCamera capture1(rm::CameraConfig::create(rm::GrabMode::Continuous, rm::RetrieveMode::SDK), "0123456789");
rm::MvCamera capture2(rm::CameraConfig::create(rm::GrabMode::Software, rm::RetrieveMode::OpenCV), "0123456789");
```

@end_toggle

@add_toggle{Hik}

创建 HikCamera 对象即可初始化相机，例如：

```cpp
rm::HikCamera capture1(rm::CameraConfig::create(rm::GrabMode::Continuous, rm::RetrieveMode::SDK), "0123456789");
rm::HikCamera capture2(rm::CameraConfig::create(rm::GrabMode::Software, rm::RetrieveMode::OpenCV), "0123456789");
```

@end_toggle

@add_toggle{Opt}

创建 OptCamera 对象即可初始化相机，例如：

```cpp
rm::OptCamera capture1(rm::CameraConfig::create(rm::HandleMode::IP,
                                                rm::GrabMode::Continuous,
                                                rm::RetrieveMode::SDK),
                       "192.168.1.100");
rm::OptCamera capture2(rm::CameraConfig::create(rm::HandleMode::Index,
                                                rm::GrabMode::Continuous,
                                                rm::RetrieveMode::SDK),
                       "1");
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
    <td class="markdownTableBodyCenter"><code>TriggerChannel::Chn0</code></td>
    <td class="markdownTableBodyCenter">通道 0 生效</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyCenter">通道 1</td>
    <td class="markdownTableBodyCenter"><code>TriggerChannel::Chn1</code></td>
    <td class="markdownTableBodyCenter">通道 1 生效</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter">通道 2</td>
    <td class="markdownTableBodyCenter"><code>TriggerChannel::Chn2</code></td>
    <td class="markdownTableBodyCenter">通道 2 生效</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyCenter">通道 3</td>
    <td class="markdownTableBodyCenter"><code>TriggerChannel::Chn3</code></td>
    <td class="markdownTableBodyCenter">通道 3 生效</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter" rowspan="3">采集方式</td>
    <td class="markdownTableBodyCenter">连续采集</td>
    <td class="markdownTableBodyCenter"><code>GrabMode::Continuous</code></td>
    <td class="markdownTableBodyCenter">连续触发相机，当 <code>grab</code> 方法被执行后，相机将开始连续采集，一般调用 <code>read</code>
      或在相机构造之初可自动开启相机 Grabbing。</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyCenter">软触发</td>
    <td class="markdownTableBodyCenter"><code>GrabMode::Software</code></td>
    <td class="markdownTableBodyCenter">软件触发，需要手动设置触发帧，当相机开始取流（Grabbing）时，只有在被设置触发帧之后的下一次执行有效，否则会阻塞以等待接收到触发帧，RMVL 相机库目前仅支持同步的相机数据处理，在指定时间内如果没有收到触发帧，则会被认为相机读取 <code>read</code> 失败。</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter">硬触发</td>
    <td class="markdownTableBodyCenter"><code>GrabMode::Hardware</code></td>
    <td class="markdownTableBodyCenter">
      硬件触发，通过向相机的航空接头等串行通信接口传输高低电平信号来设置触发帧，信号的有效性可通过软件设置，例如设置高电平、低电平、上升边沿、下降边沿的一种为触发方式，有关相机取流的细节同软触发。</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyCenter" rowspan="4">相机句柄模式</td>
    <td class="markdownTableBodyCenter">索引号</td>
    <td class="markdownTableBodyCenter"><code>HandleMode::Index</code></td>
    <td class="markdownTableBodyCenter">相机的索引号 `(0, 1, 2 ...)`</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter">序列号</td>
    <td class="markdownTableBodyCenter"><code>HandleMode::Key</code></td>
    <td class="markdownTableBodyCenter">制造商：序列号 S/N</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyCenter">ID</td>
    <td class="markdownTableBodyCenter"><code>HandleMode::ID</code></td>
    <td class="markdownTableBodyCenter">手动设置的相机 ID</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter">IP</td>
    <td class="markdownTableBodyCenter"><code>HandleMode::IP</code></td>
    <td class="markdownTableBodyCenter">IP 地址，形如 <code>192.168.1.100</code></td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyCenter" rowspan="2">相机数据处理模式</td>
    <td class="markdownTableBodyCenter">使用 OpenCV</td>
    <td class="markdownTableBodyCenter"><code>RetrieveMode::OpenCV</code></td>
    <td class="markdownTableBodyCenter">OpenCV 的 <code>imgproc</code> 模块提供了有关数据处理的接口，例如 <code>cv::cvtColor</code> 可以用于色彩空间转换</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyCenter">使用厂商 SDK</td>
    <td class="markdownTableBodyCenter"><code>RetrieveMode::SDK</code></td>
    <td class="markdownTableBodyCenter">SDK 中提供了有关数据处理的接口，主要用于相机解码、色彩空间转换等操作</td>
  </tr>
</table>

详细的触发方式 @see

- <a href="https://vision.scutbot.cn/HikRobot/index.html" target="_blank"> HIKROBOT 工业相机用户手册</a>

- <a href="https://vision.scutbot.cn/Mv/mv.pdf" target="_blank"> 迈德威视工业相机用户手册</a>

### 1.2 光学参数设置

#### 1.2.1 曝光设置

手动/自动设置曝光

```cpp
capture.set(CAMERA_MANUAL_EXPOSURE); // 手动曝光
capture.set(CAMERA_AUTO_EXPOSURE);   // 自动曝光
```

设置曝光值

```cpp
capture.set(CAMERA_EXPOSURE, 600); // 设置曝光值为 600
```

#### 1.2.2 白平衡设置

手动/自动设置白平衡

```cpp
capture.set(CAMERA_MANUAL_WB); // 手动白平衡
capture.set(CAMERA_AUTO_WB);   // 自动白平衡
```

设置各通道增益，并生效（在手动设置白平衡模式下有效）

```cpp
capture.set(CAMERA_WB_RGAIN, 102); // 红色通道增益设置为 102
capture.set(CAMERA_WB_GGAIN, 101); // 绿色通道增益设置为 101
capture.set(CAMERA_WB_BGAIN, 100); // 蓝色通道增益设置为 100
```

#### 1.2.3 其余光学参数设置

```cpp
capture.set(CAMERA_GAIN, 64);        // 设置模拟增益为 64
capture.set(CAMERA_GAMMA, 80);       // 设置 Gamma 为 80
capture.set(CAMERA_CONTRAST, 120);   // 设置对比度为 120
capture.set(CAMERA_SATURATION, 100); // 设置饱和度为 100
capture.set(CAMERA_SHARPNESS, 100);  // 设置锐度为 100
```

@note Hik 工业相机暂不支持修改 Gamma

### 1.3 处理参数设置

```cpp
// 设置硬触发采集延迟为 1000 μs，仅在硬触发模式下有效
capture.set(CAMERA_TRIGGER_DELAY, 1000);
// 设置单次触发时的触发帧数为 5 帧，即一次触发能触发 5 帧画面，仅在触发模式下有效
capture.set(CAMERA_TRIGGER_COUNT, 5);
// 设置单次触发时多次采集的周期为 100 μs，即一次触发信号能触发多帧画面，每帧间隔为 100 μs
capture.set(CAMERA_TRIGGER_PERIOD, 100);
```

### 1.4 事件设置

```cpp
capture.set(CAMERA_ONCE_WB);      // 执行一次白平衡操作，仅在手动白平衡模式下有效
capture.set(CAMERA_SOFT_TRIGGER); // 执行一次软触发，仅在软触发模式下有效
```

## 2. para 参数加载

RMVL 提供了全局的相机参数对象: para::camera_param ，详情可参考类 para::CameraParam

## 3. 示例程序

在构建 RMVL 时，需开启 `BUILD_EXAMPLES` 选项（默认开启）

```bash
cmake -DBUILD_EXAMPLES=ON ..
make -j4
cd build
```

### 3.1 单相机

单相机例程，在 build 文件夹下执行以下命令

@add_toggle{Mv}
```bash
bin/sample_mv_mono
```
@end_toggle
@add_toggle{Hik}
```bash
bin/sample_hik_mono
```
@end_toggle

相机按照连续采样、`cvtColor` 处理方式运行，程序运行中，`cv::waitKey(1)` 接受到 `s` 键被按下时，可将参数保存到 `out_para.yml` 文件中。

键入一次 `Esc` 可暂停程序，按其余键可恢复。键入两次 `Esc` 可退出程序。

### 3.2 多相机

多相机例程，在 `build` 文件夹下执行以下命令

@add_toggle{Mv}
```bash
bin/sample_mv_multi
```
@end_toggle
@add_toggle{Hik}
```bash
bin/sample_hik_multi
```
@end_toggle

相机按照连续采样、`cvtColor` 处理方式运行，程序会枚举所有的相机设备，并可视化的显示出来，指定一个序列号来启动某一相机。

程序运行过程中，相机参数会自动从 `out_para.yml` 中加载，若没有则会按照默认值运行。

键入一次 `Esc` 可暂停程序，按其余键可恢复。键入两次 `Esc` 可退出程序。

### 3.3 相机录屏

相机录屏例程，在 build 文件夹下执行以下命令

@add_toggle{Mv}
```bash
bin/sample_mv_writer
```
@end_toggle
@add_toggle{Hik}
```bash
bin/sample_hik_writer
```
@end_toggle

相机按照连续采样、`cvtColor` 处理方式运行，`-o` 可指定输出文件名，否则默认输出到 `ts.avi`，例如

@add_toggle{Mv}
```bash
bin/sample_mv_writer -o=aaa.avi
```
@end_toggle
@add_toggle{Hik}
```bash
bin/sample_hik_writer -o=aaa.avi
```
@end_toggle

程序运行过程中，相机参数会自动从 `out_para.yml` 中加载，若没有则会按照默认值运行。

### 3.4 相机标定

相机标定程序，在 `build` 文件夹下执行以下命令

@add_toggle{Mv}
```bash
bin/sample_mv_calibration -w=<?> -h=<?> -s=<?> -d=<?> -n=<?>
```
@end_toggle
@add_toggle{Hik}
```bash
bin/sample_hik_calibration -w=<?> -h=<?> -s=<?> -d=<?> -n=<?>
```
@end_toggle

`<?>` 表示可调节，具体帮助可直接执行以下命令

@add_toggle{Mv}
```bash
bin/sample_mv_calibration -help
```
@end_toggle
@add_toggle{Hik}
```bash
bin/sample_hik_calibration -help
```
@end_toggle

## 4. 使用 Demo

@note 下面以 MvCamera 为例， HikCamera 相机使用方式与之完全相同

连续采样：

```cpp
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

软触发：

```cpp
int main()
{
    auto camera_config = rm::CameraConfig::create(rm::GrabMode::Software, rm::RetrieveMode::OpenCV)
    rm::MvCamera capture(camera_config);

    bool run = true;
    std::thread th([&run]() {
        while (run)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            capture.set(rm::CAMERA_SOFT_TRIGGER); // 触发
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
