消息模块使用教程 {#tutorial_table_of_content_rmvlmsg}
============

@prev_tutorial{tutorial_table_of_content_rmvlpara}

@next_tutorial{tutorial_table_of_content_extra}

@tableofcontents

------

此模块主要为 @ref lpss 提供支持，详细使用说明请参考 @ref tutorial_modules_lpss 。

### 1 概述

RMVL 消息描述文件（`*.msg`）用于定义消息的数据结构和字段类型，类似于 ROS 消息定义文件。通过定义消息描述文件，用户可以方便地在 RMVL 中进行数据传输，这在分布式系统、网络通信中尤为重要。 RMVL 提供了一套简洁的消息定义语法，与 ROS/ROS 2 的定义语法大致兼容，同样支持多种数据类型和较为复杂的数据结构。

### 2 内置消息类型

以下是一些常用的内置消息类型，分为 5 个主要分组：`std`、`geometry`、`sensor`、`motion` 和 `viz`。用户可以根据需要在自定义的 `*.msg` 文件中引用这些内置消息类型。

<div class="tabbed">

- <b class="tab-title">std 消息分组</b>

  `std` 消息包含了一些基本的数据类型，包含 `Header`、`string` 以及其他基本的数据类型，嵌套使用时<span style="color: green">无需</span>使用 `std/` 前缀，除 `Header` 和 `ColorRGBA` 存储多值的消息类型外，其他均为单值存储。

  <div class="full_width_table">
  <table class="markdownTable">
  <tr class="markdownTableHead">
    <th class="markdownTableHeadCenter">类型</th>
    <th class="markdownTableHeadCenter">*.msg 定义</th>
    <th class="markdownTableHeadCenter">描述</th>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>Bool</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">bool</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示布尔值数据</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>Char</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">char</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示字符数据</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>ColorRGBA</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">float32</span> r</div>
      <div class="line"><span class="keywordtype">float32</span> g</div>
      <div class="line"><span class="keywordtype">float32</span> b</div>
      <div class="line"><span class="keywordtype">float32</span> a</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示颜色的红、绿、蓝和透明度分量</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>Float32</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">float32</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示 32 位单精度浮点数数据，采用 `float` 存储</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>Float64</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">float64</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示 64 位双精度浮点数数据，采用 `double` 存储</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>Header</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">uint32</span> seq</div>
      <div class="line"><span class="keywordtype">time</span> stamp</div>
      <div class="line"><span class="keywordtype">string</span> frame_id</div>
    </div></td>
    <td class="markdownTableBodyLeft">包含序列号、时间戳和坐标系 ID 的标准消息头</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>Int8</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">int8</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示 8 位有符号整数数据</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>Int16</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">int16</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示 16 位有符号整数数据</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>Int32</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">int32</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示 32 位有符号整数数据</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>Int64</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">int64</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示 64 位有符号整数数据</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>String</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">string</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示字符串数据，底层采用 `std::string` 存储</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>Time</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">time</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">记录了自 1970 年 1 月 1 日以来的时间，一般采用毫秒时间戳</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>UInt8</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">uint8</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示 8 位无符号整数数据</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>UInt16</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">uint16</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示 16 位无符号整数数据</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>UInt32</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">uint32</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示 32 位无符号整数数据</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>UInt64</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">uint64</span> data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示 64 位无符号整数数据</td>
  </tr>
  </table>
  </div>

- <b class="tab-title">geometry 消息分组</b>

  `geometry` 消息用于表示空间中的几何概念，如点、向量、姿态和变换，嵌套使用时<span style="color: red">需要</span>使用 `geometry/` 前缀。

  <div class="full_width_table">
  <table class="markdownTable">
  <tr class="markdownTableHead">
    <th class="markdownTableHeadCenter">类型</th>
    <th class="markdownTableHeadCenter">*.msg 定义</th>
    <th class="markdownTableHeadCenter">描述</th>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>Point</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">float64</span> x</div>
      <div class="line"><span class="keywordtype">float64</span> y</div>
      <div class="line"><span class="keywordtype">float64</span> z</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示空间中的一个点，采用双精度浮点数存储</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>Point32</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">float32</span> x</div>
      <div class="line"><span class="keywordtype">float32</span> y</div>
      <div class="line"><span class="keywordtype">float32</span> z</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示空间中的一个点，采用单精度浮点数存储</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>Polygon</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">geometry/Point32</span>[] points</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示空间中的一个多边形，由多个点组成，采用单精度浮点数存储</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>Pose</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">geometry/Point</span> position</div>
      <div class="line"><span class="keyword">geometry/Quaternion</span> orientation</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示空间中的位姿（位置 + 姿态）</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>Quaternion</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">float64</span> x</div>
      <div class="line"><span class="keywordtype">float64</span> y</div>
      <div class="line"><span class="keywordtype">float64</span> z</div>
      <div class="line"><span class="keywordtype">float64</span> w</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示空间中的旋转姿态（四元数）</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>Transform</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">geometry/Vector3</span> translation</div>
      <div class="line"><span class="keyword">geometry/Quaternion</span> rotation</div></div></td>
    <td class="markdownTableBodyLeft">表示两个坐标系之间的变换关系（平移 + 旋转）</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>TransformStamped</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">Header</span> header</div>
      <div class="line"><span class="keywordtype">string</span> child_frame_id</div>
      <div class="line"><span class="keyword">geometry/Transform</span> transform</div>
    </div></td>
    <td class="markdownTableBodyLeft">带时间戳的坐标变换，表示从 <code>header.frame_id</code> 到 <code>child_frame_id</code> 的变换</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>Twist</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">geometry/Vector3</span> linear</div> 
      <div class="line"><span class="keyword">geometry/Vector3</span> angular</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示物体的线速度和角速度</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>Vector3</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">float64</span> x</div>
      <div class="line"><span class="keywordtype">float64</span> y</div>
      <div class="line"><span class="keywordtype">float64</span> z</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示空间中的一个 3D 向量</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>Wrench</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">geometry/Vector3</span> force</div>
      <div class="line"><span class="keyword">geometry/Vector3</span> torque</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示作用在物体上的力和力矩</td>
  </tr>
  </table>
  </div>

  在自定义 `*.msg` 文件中使用 `geometry` 分组的消息时，需要手动指定分组前缀，即要手动添加 `geometry/` 前缀，例如：

  <div class="fragment">
  <div class="line"><span class="comment"># 自定义消息类型 PoseX.msg</span></div>
  <div class="line"><span class="keyword">Header</span> header</div>
  <div class="line"></div>
  <div class="line"><span class="keyword">geometry/Pose</span> pose</div>
  <div class="line"><span class="keywordtype">string</span> pose_name</div>
  </div>

- <b class="tab-title">sensor 消息分组</b>

  `sensor` 消息用于表示来自传感器的原始数据，例如惯性测量单元（IMU）和相机，嵌套使用时<span style="color: red">需要</span>使用 `sensor/` 前缀。

  <div class="full_width_table">
  <table class="markdownTable">
  <tr class="markdownTableHead">
    <th class="markdownTableHeadCenter">类型</th>
    <th class="markdownTableHeadCenter">*.msg 定义</th>
    <th class="markdownTableHeadCenter">描述</th>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>CameraInfo</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">Header</span> header</div>
      <div class="line"><span class="keywordtype">uint32</span> height</div>
      <div class="line"><span class="keywordtype">uint32</span> width</div>
      <div class="line"><span class="keywordtype">float64</span>[5] D</div>
      <div class="line"><span class="keywordtype">float64</span>[9] K</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示相机的校准和配置参数</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>Image</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">Header</span> header</div>
      <div class="line"><span class="keywordtype">uint32</span> height</div>
      <div class="line"><span class="keywordtype">uint32</span> width</div>
      <div class="line"><span class="keywordtype">string</span> encoding</div>
      <div class="line"><span class="keywordtype">uint8</span> is_bigendian</div>
      <div class="line"><span class="keywordtype">uint32</span> step</div>
      <div class="line"><span class="keywordtype">uint8</span>[] data</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示图像数据</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>Imu</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">Header</span> header</div>
      <div class="line"><span class="keyword">geometry/Quaternion</span> orientation</div>
      <div class="line"><span class="keywordtype">float64</span>[9] orientation_covariance</div>
      <div class="line"><span class="keyword">geometry/Vector3</span> angular_velocity</div>
      <div class="line"><span class="keywordtype">float64</span>[9] angular_velocity_covariance</div>
      <div class="line"><span class="keyword">geometry/Vector3</span> linear_acceleration</div>
      <div class="line"><span class="keywordtype">float64</span>[9] linear_acceleration_covariance</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示来自 IMU 的数据，包括姿态、角速度和线加速度</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>JointState</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">Header</span> header</div>
      <div class="line"><span class="keywordtype">string</span>[] name</div>
      <div class="line"><span class="keywordtype">float64</span>[] position</div>
      <div class="line"><span class="keywordtype">float64</span>[] velocity</div>
      <div class="line"><span class="keywordtype">float64</span>[] effort</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示单自由度关节的状态信息，例如机械臂、机器人的关节角度、速度和力矩</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>MultiDOFJointState</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">Header</span> header</div>
      <div class="line"><span class="keywordtype">string</span>[] joint_names</div>
      <div class="line"><span class="keywordtype">geometry/Transform</span>[] transforms</div>
      <div class="line"><span class="keywordtype">geometry/Twist</span>[] twist</div>
      <div class="line"><span class="keywordtype">geometry/Wrench</span>[] wrench</div>
    </div></td>
    <td class="markdownTableBodyLeft">表示多自由度关节的状态信息，例如包含球形关节、飞行器的6自由度基座关节的位姿、速度和力矩</td>
  </tr>
  </table>
  </div>

  与 `geometry` 分组的消息类似，在自定义 `*.msg` 文件中使用 `sensor` 分组的消息时，需要手动指定分组前缀，即要手动添加 `sensor/` 前缀，例如：

  <div class="fragment">
  <div class="line"><span class="comment"># 自定义消息类型 SCARA.msg</span></div>
  <div class="line"><span class="keyword">Header</span> header</div>
  <div class="line"></div>
  <div class="line"><span class="keywordtype">string</span> robot_name</div>
  <div class="line"><span class="keywordtype">uint8</span>[4] ip</div>
  <div class="line"><span class="keyword">sensor/JointState</span> joint_state</div>
  </div>

- <b class="tab-title">motion 消息分组</b>

  `motion` 消息用于表示运动相关的数据，例如轨迹、坐标变换树等内容，嵌套使用时<span style="color: red">需要</span>使用 `motion/` 前缀。

  <div class="full_width_table">
  <table class="markdownTable">
  <tr class="markdownTableHead">
    <th class="markdownTableHeadCenter">类型</th>
    <th class="markdownTableHeadCenter">*.msg 定义</th>
    <th class="markdownTableHeadCenter">描述</th>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>JointTrajectory</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">Header</span> header</div>
      <div class="line"><span class="keywordtype">string</span>[] joint_names</div>
      <div class="line"><span class="keyword">motion/JointTrajectoryPoint</span>[] points</div>
    </div></td>
    <td class="markdownTableBodyLeft">关节轨迹，包含轨迹点序列，描述了一组关节随时间的运动规划</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>JointTrajectoryPoint</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">float64</span>[] positions</div>
      <div class="line"><span class="keywordtype">float64</span>[] velocities</div>
      <div class="line"><span class="keywordtype">float64</span>[] accelerations</div>
      <div class="line"><span class="keywordtype">float64</span>[] effort</div>
      <div class="line"><span class="keywordtype">int64</span> time_from_start</div>
    </div></td>
    <td class="markdownTableBodyLeft">关节轨迹中的单个轨迹点，包含各关节的位置、速度、加速度、力矩以及从轨迹起点到达此点的期望时间</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>TF</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">geometry/TransformStamped</span>[] transforms</div>
    </div></td>
    <td class="markdownTableBodyLeft">坐标变换树，包含一组带时间戳的坐标系变换关系</td>
  </tr>
  </table>
  </div>

  与 `geometry` 分组的消息类似，在自定义 `*.msg` 文件中使用 `motion` 分组的消息时，需要手动指定分组前缀，即要手动添加 `motion/` 前缀，例如：

  <div class="fragment">
  <div class="line"><span class="comment"># 自定义消息类型 ArmPlan.msg</span></div>
  <div class="line"><span class="keyword">Header</span> header</div>
  <div class="line"></div>
  <div class="line"><span class="keywordtype">string</span> robot_name</div>
  <div class="line"><span class="keyword">motion/JointTrajectory</span> trajectory</div>
  </div>

- <b class="tab-title">viz 消息分组</b>

  `viz` 消息用于表示可视化相关的数据，例如标记、路径和交互式控制，嵌套使用时<span style="color: red">需要</span>使用 `viz/` 前缀。

  <div class="full_width_table">
  <table class="markdownTable">
  <tr class="markdownTableHead">
    <th class="markdownTableHeadCenter">类型</th>
    <th class="markdownTableHeadCenter">*.msg 定义</th>
    <th class="markdownTableHeadCenter">描述</th>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>Marker</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">Header</span> header</div>
      <div class="line"><span class="keywordtype">uint32</span> id</div>
      <div class="line"><span class="keywordtype">uint8</span> type</div>
      <div class="line"><span class="keywordtype">uint8</span> action</div>
      <div class="line"><span class="keyword">geometry/Pose</span> pose</div>
      <div class="line"><span class="keyword">geometry/Vector3</span> scale</div>
      <div class="line"><span class="keyword">ColorRGBA</span> color</div>
      <div class="line"><span class="keyword">geometry/Point</span>[] points</div>
      <div class="line"><span class="keyword">ColorRGBA</span>[] colors</div>
      <div class="line"></div>
      <div class="line"><span class="keyword">geometry/Point</span>[] points</div>
      <div class="line"><span class="keyword">ColorRGBA</span>[] colors</div>
    </div></td>
    <td class="markdownTableBodyLeft">标记显示，允许以编程方式向 LViz 3D 视图添加各种基本形状</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>MarkerArray</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">Header</span> header</div>
      <div class="line"><span class="keyword">viz/Marker</span>[] markers</div>
    </div></td>
    <td class="markdownTableBodyLeft">标记显示数组，允许一次发布多个标记以提高效率</td>
  </table>
  </div>

</div>

### 3 自动代码生成

RMVL 提供了 `rmvl_generate_msg` 的 CMake 函数，用于生成消息类型的 C++ 代码文件，用于生成独立模块的消息类型。用户只需在模块的 CMakeLists.txt 文件中调用该函数，并提供消息类型的名称和路径，RMVL 将自动生成相应的 C++ 代码文件。

<div class="fragment">
<div class="line"><span class="comment"># 根据 msg/test.msg 文件生成消息类型代码</span></div>
<div class="line"><span class="comment"># 将生成 rmvlmsg/test.hpp 头文件</span></div>
<div class="line"><span class="keyword">rmvl_generate_msg</span>(test)</div>
<div class="line"></div>
<div class="line"><span class="comment"># 根据 msg/dir/test.msg 文件生成消息类型代码</span></div>
<div class="line"><span class="comment"># 将生成 rmvlmsg/dir/test.hpp 头文件</span></div>
<div class="line"><span class="keyword">rmvl_generate_msg</span>(dir/test)</div>
<div class="line"></div>
<div class="line"><span class="comment"># 根据 msg/test2.msg 文件生成消息类型代码，并指定其绑定的子模块 sub</span></div>
<div class="line"><span class="comment"># 将生成 rmvlmsg/test2.hpp 头文件</span></div>
<div class="line"><span class="keyword">rmvl_generate_msg</span>(</div>
<div class="line">&nbsp;&nbsp;test2</div>
<div class="line">&nbsp;&nbsp;<span class="keyword">MODULE</span> sub</div>
<div class="line">)</div>
<div class="line"></div>
<div class="line"><span class="comment"># 根据 msg/dir/test2.msg 文件生成消息类型代码，并指定其绑定的子模块 sub</span></div>
<div class="line"><span class="comment"># 将生成 rmvlmsg/dir/test2.hpp 头文件</span></div>
<div class="line"><span class="keyword">rmvl_generate_msg</span>(</div>
<div class="line">&nbsp;&nbsp;dir/test2</div>
<div class="line">&nbsp;&nbsp;<span class="keyword">MODULE</span> sub</div>
<div class="line">)</div>
</div>
