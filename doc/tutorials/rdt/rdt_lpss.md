LPSS CLI 工具 {#tutorial_rdt_lpss}
============

@author 赵曦
@date 2026/07/31
@version 1.1
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
@dl_item{service,服务 CLI 工具}
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
@dl_item{list,列出所有节点，可使用 `-c` 仅显示数量}
@dl_end

#### list 子命令

列出当前发现的所有 LPSS 节点。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> node list [<span class="comment">-c</span>]</div>
</div>

@dl_begin{选项}
@dl_item{\-c,仅输出节点数量}
@dl_end

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 列出节点名称</span></div>
<div class="line"><span class="keywordflow">lpss</span> node list</div>
<div class="line"><span class="comment"># 仅输出节点数量</span></div>
<div class="line"><span class="keywordflow">lpss</span> node list <span class="comment">-c</span></div>
</div>

#### info 子命令

查看指定节点的信息，输出形如以下的内容

```
Node: xxx

Publish Topics:
  xxx

Subscribe Topics:
  xxx

Server Services:
  xxx

Client Services:
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
<div class="line"><span class="keywordflow">lpss</span> topic <span class="comment">[help | info | list | find | echo | pub | type | hz | bw]</span> [args...]</div>
</div>

@dl_begin{命令}
@dl_item{help,显示此帮助信息}
@dl_item{info,显示话题信息}
@dl_item{list,列出所有话题，可使用 `-c` 仅显示数量}
@dl_item{find,按消息类型查找话题，可使用 `-c` 仅显示数量}
@dl_item{echo,显示话题内容}
@dl_item{pub,发布话题}
@dl_item{type,显示话题类型}
@dl_item{hz,测量话题发布频率，单位为 Hz}
@dl_item{bw,测量话题带宽，单位为 MB/s、kB/s 或 B/s}
@dl_end

#### list 子命令

列出当前发现的所有话题，话题名称按字典序输出。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> topic list [<span class="comment">-c</span>]</div>
</div>

@dl_begin{选项}
@dl_item{\-c,仅输出话题数量}
@dl_end

#### find 子命令

按完整消息类型查找话题，查询结果按话题名称的字典序输出。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> topic find <span class="comment">&lt;msg_type&gt;</span> [<span class="comment">-c</span>]</div>
</div>

@param msg_type 消息类型，例如 `std/String`

@dl_begin{选项}
@dl_item{\-c,仅输出匹配的话题数量}
@dl_end

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 查找所有使用 std/String 消息类型的话题</span></div>
<div class="line"><span class="keywordflow">lpss</span> topic find std/String</div>
<div class="line"><span class="comment"># 仅输出匹配话题的数量</span></div>
<div class="line"><span class="keywordflow">lpss</span> topic find std/String <span class="comment">-c</span></div>
</div>

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

### service 命令

服务工具，用于查看已发现的 LPSS 服务，并使用 JSON 请求调用内置服务。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> service <span class="comment">[help | info | list | type | find | call]</span> [args...]</div>
</div>

@dl_begin{命令}
@dl_item{help,显示此帮助信息}
@dl_item{info,显示服务信息}
@dl_item{list,列出所有服务，可使用 `-c` 仅显示数量}
@dl_item{type,显示服务类型}
@dl_item{find,按服务类型查找服务，可使用 `-c` 仅显示数量}
@dl_item{call,使用 JSON 请求调用内置服务}
@dl_end

#### list 子命令

列出当前发现的所有服务，服务名称按字典序输出。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> service list [<span class="comment">-c</span>]</div>
</div>

@dl_begin{选项}
@dl_item{\-c,仅输出服务数量}
@dl_end

#### info 子命令

查看指定服务的类型、服务端节点和客户端节点，输出形如以下内容。

```
Type: std/SetBool

Server Node:
  server_node

Client Node:
  client_node
```

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> service info <span class="comment">&lt;service_name&gt;</span></div>
</div>

@param service_name 服务名称

**示例**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> service info /set_enabled</div>
</div>

#### type 子命令

显示指定服务的服务类型。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> service type <span class="comment">&lt;service_name&gt;</span></div>
</div>

@param service_name 服务名称

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 输出 std/SetBool</span></div>
<div class="line"><span class="keywordflow">lpss</span> service type /set_enabled</div>
</div>

#### find 子命令

按类型精确匹配服务。`service_type` 可以是完整服务类型，也可以是对应的请求或响应消息类型。查询结果按服务名称的字典序输出。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> service find <span class="comment">&lt;service_type&gt;</span> [<span class="comment">-c</span>]</div>
</div>

@param service_type 服务类型，例如 `std/SetBool`；也可使用 `std/SetBool_Request` 或 `std/SetBool_Response`

@dl_begin{选项}
@dl_item{\-c,仅输出匹配的服务数量}
@dl_end

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 列出所有 std/SetBool 服务</span></div>
<div class="line"><span class="keywordflow">lpss</span> service find std/SetBool</div>
<div class="line"><span class="comment"># 仅输出匹配服务的数量</span></div>
<div class="line"><span class="keywordflow">lpss</span> service find std/SetBool <span class="comment">-c</span></div>
</div>

#### call 子命令

使用 JSON 对象作为请求调用指定服务，并将响应输出为 JSON。`json_request` 省略或为空时按 `{}` 处理，命令等待响应的超时时间为 3 秒。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> service call <span class="comment">&lt;service_name&gt;</span> [<span class="comment">json_request</span>]</div>
</div>

@param service_name 服务名称
@param json_request JSON 对象形式的请求，建议使用单引号包围，避免 Shell 改写引号等字符

目前 `call` 支持以下内置服务类型。

| 服务类型 | JSON 请求 |
| --- | --- |
| `std/Empty` | `{}`，可省略 |
| `std/Trigger` | `{}`，可省略 |
| `std/SetBool` | 必须包含布尔字段 `data` |
| `sensor/SetCameraInfo` | 必须包含对象字段 `camera_info`；其中 `D` 和 `K` 如果出现，必须分别包含 5 个和 9 个数字 |

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 调用无请求字段的 Trigger 服务</span></div>
<div class="line"><span class="keywordflow">lpss</span> service call /trigger</div>
<div class="line"><span class="comment"># 使用 JSON 请求调用 SetBool 服务</span></div>
<div class="line"><span class="keywordflow">lpss</span> service call /set_enabled <span class="stringliteral">'{"data":true}'</span></div>
<div class="line"><span class="comment"># 调用 SetCameraInfo 服务，未填写的 CameraInfo 字段使用默认值</span></div>
<div class="line"><span class="keywordflow">lpss</span> service call /set_camera_info <span class="stringliteral">'{"camera_info":{"height":1080,"width":1920}}'</span></div>
</div>

@note `call` 暂不支持自定义服务类型；发现到不支持的类型时会输出 `unsupported service type`。

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

节点图工具 LGraph，用于在浏览器中查看 LPSS 通信拓扑，包括节点、话题、服务及其发布/订阅、服务端/客户端关系。也可直接使用 `lgraph` 命令启动。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> graph [<span class="comment">name_subfix</span>]</div>
<div class="line"><span class="comment"># 等价命令</span></div>
<div class="line"><span class="keywordflow">lgraph</span> [<span class="comment">name_subfix</span>]</div>
</div>

@param name_subfix 可选的实例名称，用于生成 LPSS 节点名 `lgraph_<name_subfix>`；省略时自动生成 5 位随机标识

启动成功后，终端会输出本机和局域网访问地址。LGraph 默认监听 `17493` 端口，本机可访问：

<div class="fragment">
<div class="line"><span class="keywordflow">http://localhost:17493</span></div>
</div>

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 使用自动生成的实例名启动</span></div>
<div class="line"><span class="keywordflow">lpss</span> graph</div>
<div class="line"><span class="comment"># LPSS 节点名为 lgraph_debug</span></div>
<div class="line"><span class="keywordflow">lgraph</span> debug</div>
</div>

@note Web 服务使用固定端口 `17493`，同一主机上不能同时启动多个 LGraph 实例。按 `Ctrl+C` 可停止工具。

@note 如果提示 `%lpss graph 工具尚未安装`，请在 `rmvl-dev-tools` 中重新运行 `install.bash`。

### viz 命令

3D 可视化工具 LViz，也可直接使用 `lviz` 命令启动。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">lpss</span> viz [<span class="comment">name_subfix</span>]</div>
<div class="line"><span class="comment"># 等价命令</span></div>
<div class="line"><span class="keywordflow">lviz</span> [<span class="comment">name_subfix</span>]</div>
</div>

@param name_subfix 可选的实例名称，用于生成 LPSS 节点名 `lviz_node_<name_subfix>`；省略时自动生成 5 位随机标识

LViz 默认监听 `17492` 端口，启动后可通过 `http://localhost:17492` 访问，按 `Ctrl+C` 停止工具。
