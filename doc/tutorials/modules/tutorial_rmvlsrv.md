服务模块使用教程 {#tutorial_table_of_content_rmvlsrv}
============

@prev_tutorial{tutorial_table_of_content_rmvlmsg}

@next_tutorial{tutorial_table_of_content_extra}

@tableofcontents

---

此模块主要为 @ref lpss 提供支持，详细使用说明请参考 @ref tutorial_modules_lpss 。

### 1 概述

RMVL 服务描述文件 `*.srv` 用于定义一次服务调用的请求和响应类型，类似于 ROS 服务定义文件。请求和响应使用单独一行 `---` 分隔，两部分均遵循与 @ref tutorial_table_of_content_rmvlmsg "RMVL 消息描述文件" 相同的字段语法和二进制序列化规则，也都允许为空。通过服务描述文件，用户可以为 LPSS Service/Client 定义结构化的调用接口。

### 2 内置服务类型

以下是 RMVL 提供的内置服务类型，分为 `std` 和 `sensor` 两个主要分组。用户可以直接在 LPSS Service/Client 中使用这些类型。

<div class="tabbed">

- <b class="tab-title">std 服务分组</b>

  `std` 服务包含无参数调用、布尔值设置以及通用触发等基础服务。

  <div class="full_width_table">
  <table class="markdownTable">
  <tr class="markdownTableHead">
    <th class="markdownTableHeadCenter">类型</th>
    <th class="markdownTableHeadCenter">*.srv 定义</th>
    <th class="markdownTableHeadCenter">描述</th>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>Empty</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="comment">\-\-\-</span></div>
    </div></td>
    <td class="markdownTableBodyLeft">请求和响应均为空的服务</td>
  </tr>
  <tr class="markdownTableRowEven">
    <td class="markdownTableBodyLeft"><code>SetBool</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keywordtype">bool</span> data</div>
      <div class="line"><span class="comment">\-\-\-</span></div>
      <div class="line"><span class="keywordtype">bool</span> success</div>
      <div class="line"><span class="keywordtype">string</span> message</div>
    </div></td>
    <td class="markdownTableBodyLeft">设置布尔状态，并返回执行结果和提示信息</td>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>Trigger</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="comment">\-\-\-</span></div>
      <div class="line"><span class="keywordtype">bool</span> success</div>
      <div class="line"><span class="keywordtype">string</span> message</div>
    </div></td>
    <td class="markdownTableBodyLeft">触发无参数操作，并返回执行结果和提示信息</td>
  </tr>
  </table>
  </div>

- <b class="tab-title">sensor 服务分组</b>

  `sensor` 服务用于配置传感器相关的数据和参数。

  <div class="full_width_table">
  <table class="markdownTable">
  <tr class="markdownTableHead">
    <th class="markdownTableHeadCenter">类型</th>
    <th class="markdownTableHeadCenter">*.srv 定义</th>
    <th class="markdownTableHeadCenter">描述</th>
  </tr>
  <tr class="markdownTableRowOdd">
    <td class="markdownTableBodyLeft"><code>SetCameraInfo</code></td>
    <td class="markdownTableBodyLeft"><div class="fragment">
      <div class="line"><span class="keyword">sensor/CameraInfo</span> camera_info</div>
      <div class="line"><span class="comment">\-\-\-</span></div>
      <div class="line"><span class="keywordtype">bool</span> success</div>
      <div class="line"><span class="keywordtype">string</span> status_message</div>
    </div></td>
    <td class="markdownTableBodyLeft">保存相机标定信息，并返回执行结果和状态信息</td>
  </tr>
  </table>
  </div>

</div>

### 3 自动代码生成

RMVL 提供了 `rmvl_generate_srv` 的 CMake 函数，用于生成服务类型的 C++ 代码文件。用户只需在模块的 CMakeLists.txt 文件中调用该函数，并提供服务类型的名称和路径，RMVL 将自动生成相应的 C++ 代码文件。

<div class="fragment">
<div class="line"><span class="comment"># 根据 srv/Test.srv 文件生成服务类型代码</span></div>
<div class="line"><span class="comment"># 将生成 rmvlsrv/test.hpp 头文件</span></div>
<div class="line"><span class="keyword">rmvl_generate_srv</span>(Test)</div>
<div class="line"></div>
<div class="line"><span class="comment"># 根据 srv/dir/Test.srv 文件生成服务类型代码</span></div>
<div class="line"><span class="comment"># 将生成 rmvlsrv/dir/test.hpp 头文件</span></div>
<div class="line"><span class="keyword">rmvl_generate_srv</span>(dir/Test)</div>
<div class="line"></div>
<div class="line"><span class="comment"># 根据 srv/Test2.srv 文件生成服务类型代码，并指定其绑定的子模块 sub</span></div>
<div class="line"><span class="comment"># 将生成 rmvlsrv/test2.hpp 头文件</span></div>
<div class="line"><span class="keyword">rmvl_generate_srv</span>(</div>
<div class="line">&nbsp;&nbsp;Test2</div>
<div class="line">&nbsp;&nbsp;<span class="keyword">MODULE</span> sub</div>
<div class="line">)</div>
<div class="line"></div>
<div class="line"><span class="comment"># 根据 srv/dir/Test2.srv 文件生成服务类型代码，并指定其绑定的子模块 sub</span></div>
<div class="line"><span class="comment"># 将生成 rmvlsrv/dir/test2.hpp 头文件</span></div>
<div class="line"><span class="keyword">rmvl_generate_srv</span>(</div>
<div class="line">&nbsp;&nbsp;dir/Test2</div>
<div class="line">&nbsp;&nbsp;<span class="keyword">MODULE</span> sub</div>
<div class="line">)</div>
</div>

生成的服务类型位于 `rm::srv` 命名空间。以 `Test` 为例，`Test::Request` 和 `Test::Response` 分别表示请求和响应类型，二者均提供与消息类型相同的 `serialize()`、`deserialize()` 和 `compact_size()` 接口。
