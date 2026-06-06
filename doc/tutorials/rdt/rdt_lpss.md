LPSS CLI 工具 {#tutorial_rdt_lpss}
============

@author 赵曦
@date 2026/06/06
@version 1.0
@brief LPSS 命令行工具的使用教程

@prev_tutorial{tutorial_rdt_rdt}

@tableofcontents

---

### 前言

LPSS 是一个轻量级的发布订阅通信框架，采用去中心化设计，提供 NDP、EDP 两层服务发现机制，以及 MTP 话题消息传输协议，提供类似 ROS2 的 `*.msg` 消息接口，由 RMVL 提供支持。本工具提供了 LPSS 相关的命令行工具，使用方法如下。

`rdt` 提供的 LPSS CLI 工具可通过输入

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> help</div>
</div>

来查看具体帮助。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> <span class="comment">&lt;command&gt;</span> [args...]</div>
</div>

@dl_begin{命令}
@dl_item{help,显示详细帮助信息}
@dl_item{create,创建一个依赖 lpss 的新项目}
@dl_item{node,节点 CLI 工具}
@dl_item{topic,话题 CLI 工具}
@dl_item{interface,内置消息接口查看工具}
@dl_item{graph,节点图工具}
@dl_item{viz,3D 可视化工具 LViz}
@dl_end

更多信息请参考官方手册:

- [使用教程](https://cv-rmvl.github.io/docs/2.x/d3/d8e/tutorial_modules_lpss.html)
- [API 文档](https://cv-rmvl.github.io/docs/2.x/d7/de3/group__lpss.html)

### create 命令

创建一个依赖 LPSS 的新项目

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> create <span class="comment">&lt;project_name&gt;</span> [options]</div>
</div>

@param project_name 待创建的项目名称

@dl_begin{选项}
@dl_item{\-\-deps,&lt;list&gt;,指定项目依赖的 RMVL 模块，逗号或空格分隔，默认为空}
@dl_item{\-\-exts,&lt;list&gt;,指定项目使用的非 RMVL 库，逗号或空格分隔，默认为空}
@dl_item{\-\-cpp,&lt;version&gt;,指定项目使用的 C++ 标准版本，默认为 `20`}
@dl_end

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 创建一个名为 demo_node 的新项目</span></div>
<div class="line"><span class="keywordflow">lpss</span> create demo_node</div>
<div class="line"><span class="comment"># 创建一个名为 demo_node 的新项目，依赖 anchor 和 hik_camera 模块，使用 C++17 标准</span></div>
<div class="line"><span class="keywordflow">lpss</span> create demo_node <span class="comment">\-\-deps</span> anchor hik_camera <span class="comment">\-\-cpp</span> 17</div>
<div class="line"><span class="comment"># 创建一个名为 demo_node 的新项目，依赖 anchor 和 hik_camera 模块，并且使用 json 和 fmt 两个第三方库</span></div>
<div class="line"><span class="keywordflow">lpss</span> create demo_node <span class="comment">\-\-deps</span> hik_camera <span class="comment">\-\-exts</span> json fmt</div>
</div>

### node 命令

节点工具

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> node <span class="comment">[help | info | list]</span></div>
</div>

@dl_begin{命令}
@dl_item{help,显示此帮助信息}
@dl_item{info,显示节点信息}
@dl_item{list,列出所有节点}
@dl_end

#### info 子命令

查看指定节点的信息，输出形如以下的内容

```
Node: xxx

Publish Topics:
  xxx

Subscribe Topics:
  xxx
```

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> node info <span class="comment">&lt;node_name&gt;</span></div>
</div>

@param node_name 指定要查看的节点名称

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 查看节点 lpss_node_1 的信息</span></div>
<div class="line"><span class="keywordflow">lpss</span> node info lpss_node_1</div>
</div>

### topic 命令

话题工具

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> topic <span class="comment">[help | info | list | echo | pub | type | hz | bw]</span> [args...]</div>
</div>

@dl_begin{命令}
@dl_item{help,显示此帮助信息}
@dl_item{info,显示话题信息}
@dl_item{list,列出所有话题}
@dl_item{echo,显示话题内容}
@dl_item{pub,发布话题}
@dl_item{type,显示话题类型}
@dl_item{hz,测量话题发布频率，单位为 Hz}
@dl_item{bw,测量话题带宽，单位为 MB/s、kB/s 或 B/s}
@dl_end

#### info 子命令

查看指定话题的信息，输出形如以下的内容

```
Type: xxx

Publisher Node:
  xxx

Subscriber Node:
  xxx
```

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> topic info <span class="comment">&lt;topic_name&gt;</span></div>
</div>

@param topic_name 指定要查看的话题名称

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 查看 /cur/joint_states 话题的信息</span></div>
<div class="line"><span class="keywordflow">lpss</span> topic info /cur/joint_states</div>
</div>

#### echo 子命令

显示话题内容，并以 JSON 文本进行输出

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> topic echo <span class="comment">&lt;topic_name&gt;</span></div>
</div>

@param topic_name 指定的话题名称

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 以 JSON 格式打印 /point 话题的内容</span></div>
<div class="line"><span class="keywordflow">lpss</span> topic echo /point</div>
<div class="line"></div>
<div class="line"><span class="comment"># 假设是 geometry/Point 的消息类型，可以配合 jq 工具进行输出，记得 apt install jq 来下载 jq</span></div>
<div class="line"><span class="comment"># 打印 /point 话题的 x 坐标</span></div>
<div class="line"><span class="keywordflow">lpss</span> topic echo /point | <span class="keywordflow">jq</span> .x</div>
<div class="line"></div>
<div class="line"><span class="comment"># Linux 下一般是按 \\n 刷新缓冲区，通过 | 运算符传给 jq 可能没法很好的实时输出内容，通常可以额外</span></div>
<div class="line"><span class="comment"># 配合 stdbuf 工具自动刷新缓冲区，例如</span></div>
<div class="line"><span class="keywordflow">stdbuf</span> <span class="comment">-oL</span> <span class="keywordflow">lpss</span> topic echo /point | <span class="keywordflow">jq</span> .x</div>
</div>

#### pub 子命令

发布话题内容

@warning 未完成，敬请期待

#### type 子命令

显示话题类型

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> topic type <span class="comment">&lt;topic_name&gt;</span></div>
</div>

@param topic_name 指定的话题名称

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 显示 /point 话题的类型</span></div>
<div class="line"><span class="keywordflow">lpss</span> topic type /point</div>
</div>

#### hz 子命令

测量话题发布频率，单位为 Hz

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> topic hz <span class="comment">&lt;topic_name&gt;</span></div>
</div>

@param topic_name 指定的话题名称

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 显示 /str 话题的发布频率</span></div>
<div class="line"><span class="keywordflow">lpss</span> topic hz /str</div>
</div>

#### bw 子命令

测量话题带宽，单位为 MB/s、kB/s 或 B/s

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> topic bw <span class="comment">&lt;topic_name&gt;</span></div>
</div>

@param topic_name 指定的话题名称

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 显示 /str 话题的带宽使用情况</span></div>
<div class="line"><span class="keywordflow">lpss</span> topic bw /str</div>
</div>

### interface 命令

内置消息接口查看工具

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> interface <span class="comment">[help | list | group | groups | show]</span> [args...]</div>
</div>

@dl_begin{命令}
@dl_item{help,显示此帮助信息}
@dl_item{list,列出所有内置消息接口}
@dl_item{group,显示指定的消息分组包含的接口}
@dl_item{groups,列出所有消息分组}
@dl_item{show,显示接口详细信息}
@dl_end

#### group 子命令

显示指定的消息分组包含的接口

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> interface group <span class="comment">&lt;name&gt;</span></div>
</div>

@param name 消息分组名称

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 显示 geometry 分组包含的接口</span></div>
<div class="line"><span class="keywordflow">lpss</span> interface group geometry</div>
</div>

例如会显示如下内容

```
Point
Point32
Polygon
Pose
Quaternion
Transform
TransformStamped
Twist
Vector3
Wrench
```

#### show 子命令

显示接口详细信息

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> interface show <span class="comment">&lt;interface&gt;</span></div>
</div>

@param interface 消息接口名称，格式为 `<%group>/<name>`

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 显示 geometry/Pose 接口的详细信息</span></div>
<div class="line"><span class="keywordflow">lpss</span> interface show geometry/Pose</div>
</div>

例如会显示如下内容

```
geometry/Point position
    float64 x
    float64 y
    float64 z
geometry/Quaternion orientation
    float64 x
    float64 y
    float64 z
    float64 w
```

### graph 命令

节点图工具

@warning 未完成，敬请期待

### viz 命令

3D 可视化工具 LViz，也可直接使用 `lviz` 命令来启动。
