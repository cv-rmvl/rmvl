参数模块使用教程 {#tutorial_table_of_content_rmvlpara}
============

@prev_tutorial{tutorial_table_of_content_modules}

@next_tutorial{tutorial_table_of_content_extra}

@tableofcontents

------

### 1 参数模块简介

RMVL 中，一部分模块（涉及主要模块和扩展模块）的功能在不同运行环境或者不同需求的情况下需要有不同的参数配置，为此 RVML 将这些运行时可变的参数提炼出来，并且使用统一的管理方式，完成了参数类型、参数默认值、参数名、参数运行时加载的定义。

原先若有计划使用运行时参数，在

- 调整参数名、参数默认值
- 新增某一参数类（结构体）

的时候，需要在头文件以及对应源文件的位置写上大量重复的代码，管理极其不方便。为此 RMVL 参数模块定义了新的参数规范文件，这类文件以 `*.para` 为后缀，以及提供了 CMake 配置期间将 `*.para` 转换为对应 `*.h(hpp)` 以及 `*.cpp` 的 CMake 函数 `rmvl_generate_para`，该函数定义在 `<project>/cmake/RMVLCodeGenerate.cmake` 文件中。

### 2 参数规范文件

这里举一个 `*.para` 文件的示例，写法与 ROS Message 的 `*.msg` 文件类似，不过参数的类型均使用 C/C++ 中的类型，并且添加了注释功能。

<div class="fragment">
<div class="line"><span class="comment">#################### 光学参数 ####################</span></div>
<div class="line"><span class="keywordtype">float</span> exposure = 1500&nbsp;&nbsp;<span class="comment"># 相机曝光</span></div>
<div class="line"><span class="keywordtype">float</span> saturation = 100&nbsp;<span class="comment"># 相机饱和度</span></div>
<div class="line"><span class="keywordtype">float</span> gain = 64&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"># 相机数字增益</span></div>
<div class="line"><span class="keywordtype">uint32_t</span> b_gain = 1500&nbsp;<span class="comment"># 相机蓝色增益</span></div>
<div class="line"><span class="keywordtype">uint32_t</span> g_gain = 1500&nbsp;<span class="comment"># 相机绿色增益</span></div>
<div class="line"><span class="keywordtype">uint32_t</span> r_gain = 1500&nbsp;<span class="comment"># 相机红色增益</span></div>
<div class="line"><span class="keywordtype">int</span> auto_exposure = 0&nbsp;&nbsp;<span class="comment"># 相机自动曝光模式</span></div>
<div class="line"><span class="keywordtype">int</span> auto_wb = 0&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"># 相机自动白平衡模式</span></div>
<div class="line"></div>
<div class="line"><span class="comment">#################### 设备参数 ####################</span></div>
<div class="line"><span class="keywordtype">int</span> grab_mode = 1&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"># 相机 grab 模式</span></div>
<div class="line"><span class="keywordtype">int</span> retrieve_mode = 1&nbsp;<span class="comment"># 相机 retrieve 模式</span></div>
<div class="line"><span class="keywordtype">string</span> serial_number&nbsp;&nbsp;<span class="comment"># 相机序列号</span></div>
</div>

这里列出参数规范文件的参数类型表

<div class="full_width_table">

<center>

表1：参数类型表

</center>

| 数据类型 | 含义                                                         |
| :------: | :----------------------------------------------------------- |
| 基本类型 | 1. 包括 `int`、`uint8_t`、`double`、`float`、`string` 等<br />2. 对标 C++ 的基础类型和 `std::string` |
| 矩阵类型 | 1. 包括形如 `Matx??`、`Vec?` 的类型，例如 `Matx22f`<br>2. 对标 OpenCV 的 `cv::Matx` 和 `cv::Vec`<br>3. 可使用列表初始化和相关静态函数初始化，例如 `Matx22f::eye()` |
| 复合类型 | 1. 包括 `vector` 和形如 `Point?` 的类型<br>2. 对标 C++ 的 `std::vector` 以及 OpenCV 的`cv::Point2?` 和 `cv::Point3?`<br>3. 只能使用列表初始化，例如 `{1, 2, 3}` |
| 枚举类型 | 1. 需要用户自定以 `enum` 开头和 `endenum` 结尾的数据类型声明<br />2. 对标 C++ 的有作用域枚举类型 `enum class`<br />3. 变量的定义上与有作用域枚举类型一致，例如 `Color COLOR_MODE = Color::RED` |

</div>

如果要给某一条参数设置默认值，需要使用 `=` 完成，不需要默认参数（仅由运行时加载设置）则不需要使用 `=` 进行赋值，例如

<div class="fragment">
<div class="line"><span class="keywordtype">int</span> grab_mode = 1</div>
<div class="line"><span class="keywordtype">string</span> serial_number</div>
</div>

如果要给某一参数设置注释信息，请<span style="color: red">尾置</span> `#` 并输入相应注释信息，例如

<div class="fragment">
<div class="line"><span class="keywordtype">float</span> exposure = 1500&nbsp;<span class="comment"># 相机曝光</span></div>
<div class="line"><span class="keywordtype">float</span> gain = 64&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"># 相机数字增益</span></div>
</div>

@note
- `#` 和注释信息之间<u>**不必**</u>使用空格分隔，例如 `#相机曝光`

枚举类型可在每个枚举项后加上具体的值，也能使用 `#` 为枚举项添加注释，例如

<div class="fragment">
<div class="line"><span class="keyword">enum</span>&nbsp;<span class="keywordtype">ColorMode</span>&nbsp;<span class="comment"># 颜色类型</span></div>
<div class="line">&nbsp;&nbsp;RED&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"># 红色</span></div>
<div class="line">&nbsp;&nbsp;GREEN&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"># 绿色</span></div>
<div class="line">&nbsp;&nbsp;BLUE = 4&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"># 蓝色</span></div>
<div class="line">&nbsp;&nbsp;GRAY = 5</div>
<div class="line"><span class="keyword">endenum</span></div>
<div class="line"></div>
<div class="line"><span class="keywordtype">ColorMode</span> COLOR = ColorMode::RED&nbsp;<span class="comment"># 颜色信息</span></div>
</div>

### 3 C++ 代码生成

从 `*.para` 到对应的 C++ 代码生成的过程依赖 RMVL 提供的两条 CMake 函数，分别是

- `rmvl_generate_para`
- `rmvl_generate_module_para`

#### 3.1 rmvl_generate_para

根据指定的目标名在 param 文件夹下对应的 *.para 参数规范文件和可选的模块名生成对应的 C++ 代码。

**用法**

<div class="fragment">
<div class="line"><span class="keyword">rmvl_generate_para</span>(</div>
<div class="line">&nbsp;&nbsp;&lt;target_name&gt;</div>
<div class="line">&nbsp;&nbsp;[<span class="keyword">MODULE</span> module_name]</div>
<div class="line">)</div>
</div>

**示例**

<div class="fragment">
<div class="line"><span class="keyword">rmvl_generate_para</span>(</div>
<div class="line">&nbsp;&nbsp;mytarget&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"># 目标名称</span></div>
<div class="line">&nbsp;&nbsp;<span class="keyword">MODULE</span> mymodule&nbsp;<span class="comment"># 模块名称为 mymodule</span></div>
<div class="line">)</div>
</div>

表示解析当前目录下的 `param/mytarget.para` 文件，并作为模块 `mymodule` 的子模块。最终生成的文件是

- `include/rmvlpara/mymodule/mytarget.h`，`include/rmvlpara` 后文简记为 `<prefix>`
- `src/para/mytarget.cpp`

这样的话会为参数模块汇总的头文件 `<prefix>/mymodule.hpp` 添加 `<prefix>/mymodule/mytarget.h` 文件的包含，即会在 `<prefix>/mymodule.hpp` 中添加

```cpp
#include "mymodule/mytarget.h"
```

如果 `mytarget` 需要自成一个模块的话，那么在调用 `rmvl_generate_para` 函数的时候应该写为

<div class="fragment">
<div class="line"><span class="keyword">rmvl_generate_para</span>(mytarget)</div>
</div>

这样就会生成一个名为 `mytarget` 的参数模块了，最终生成的文件是

- `include/rmvlpara/mytarget.hpp`
- `src/para/mytarget.cpp`

#### 3.2 rmvl_generate_module_para

根据给定模块下所有的 para 目标，生成对应的 C++ 代码

**用法**

<div class="fragment">
<div class="line"><span class="keyword">rmvl_generate_module_para</span>(\<module_name\>)</div>
</div>

**示例**

<div class="fragment">
<div class="line"><span class="keyword">rmvl_generate_module_para</span>(mymodule)</div>
</div>

表示将模块 `mymodule` 中的所有子模块打包至参数模块汇总头文件 `<prefix>/mymodule.hpp` ，例如，如果在 `CMakeLists.txt` 中写入

<div class="fragment">
<div class="line"><span class="keyword">rmvl_generate_para</span>(a1&nbsp;<span class="keyword">MODULE</span>&nbsp;m)</div>
<div class="line"><span class="keyword">rmvl_generate_para</span>(a2&nbsp;<span class="keyword">MODULE</span>&nbsp;m)</div>
<div class="line"><span class="keyword">rmvl_generate_para</span>(a3&nbsp;<span class="keyword">MODULE</span>&nbsp;m)</div>
<div class="line"></div>
<div class="line"><span class="keyword">rmvl_generate_module_para</span>(m)</div>
</div>

最后生成的 `<prefix>/m.hpp` 中就会包含

```cpp
#include "m/a1.h"
#include "m/a2.h"
#include "m/a3.h"
```

以上内容均是在 CMake 配置期间自动完成，只需定义好参数规范文件 `*.para` 以及相应的 CMake 函数即可。

### 4 运行时加载

#### 4.1 加载方式

例如，对于以下的 `test.para` 文件

<div class="fragment">
<div class="line"><span class="keywordtype">int</span> abc = 10&nbsp;<span class="comment"># 测试数</span></div>
</div>

在使用 `rmvl_generate_para(test)` 之后，生成的头文件代码中，类相关的代码如下

<div class="fragment">
<div class="line"><span class="comment">//! TestParam 参数模块</span></div>
<div class="line"><span class="keyword">class</span>&nbsp;RMVL_EXPORTS_W TestParam {</div>
<div class="line"><span class="keyword">public</span>:</div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment">//! 测试数 \@note 默认值：\`10\`</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="keywordtype">int</span> abc = 10;</div>
<div class="line"></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment">//! 创建 TestParam 参数对象 \@warning 不建议手动创建对象</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;RMVL_W TestParam() = <span class="keywordflow">default</span>;</div>
<div class="line"></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment">/**</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"> * \@brief 从指定 \`YAML\` 文件中加载 \`TestParam\`</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"> *</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"> * \@note \`YAML\` 文件的后缀允许是 \`*.yml\` 和 \`*.yaml\`</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"> * \@param[in] path 参数路径</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"> * \@return 是否加载成功</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"> */</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;RMVL_W&nbsp;<span class="keywordtype">bool</span> read(<span class="keyword">const</span> std::string &path);</div>
<div class="line"></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment">/**</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"> * \@brief 将 \`TestParam\` 的数据写入指定的 \`YAML\` 文件中</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"> *</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"> * \@note \`YAML\` 文件的后缀允许是 \`*.yml\` 和 \`*.yaml\`</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"> * \@param[in] path 参数路径</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"> * \@return 是否写入成功</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"> */</span></div>
<div class="line">&nbsp;&nbsp;&nbsp;&nbsp;RMVL_W&nbsp;<span class="keywordtype">bool</span> write(<span class="keyword">const</span> std::string &path) <span class="keyword">const</span>;</div>
<div class="line">};</div>
<div class="line"></div>
<div class="line"><span class="comment">//! TestParam 参数模块</span></div>
<div class="line"><span class="keyword">inline</span>&nbsp;TestParam test_param;</div>
</div>

其中包含了运行时参数导入和导出的接口 `read` 和 `write`，以 @ref para_algorithm 为例，`rm::para::AlgorithmParam::read` 用于运行时读取 YAML 文件并配置 algorithm 模块的参数，`rm::para::AlgorithmParam::write` 用于将当前 algorithm 模块的参数写入到 YAML 文件中。

用户可以通过使用类似以下的代码完成运行时加载

```cpp
/* code */
rm::para::algorithm_param.read(prefix_path + "algorithm.yml");
/* code */
```

#### 4.2 YAML 文件格式

对于某个 `*.para` 文件中的参数，可以通过 YAML 文件进行运行时配置，例如

<div class="fragment">
<div class="line"><span class="keywordtype">int</span> num = 10&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"># Test number</span></div>
<div class="line"><span class="keywordtype">float</span> value = 4.2&nbsp;<span class="comment"># Test value</span></div>
<div class="line"><span class="keywordtype">string</span> name&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="comment"># Test name</span></div>
</div>

对应的 YAML 文件可以写为

<div class="fragment">
<div class="line"><span class="comment"># test.yml</span></div>
<div class="line">%YAML:1.0</div>
<div class="line">\-\-\-</div>
<div class="line"></div>
<div class="line"><span class="keywordtype">num</span>: 20</div>
<div class="line"><span class="keywordtype">name</span>: <span class="stringliteral">"Hello, RMVL"</span></div>
</div>

在程序调用

```cpp
rm::para::test_param.read(prefix_path + "test.yml");
```

时，会实时加载该 YAML 文件，此时

- `num` 的值会被设置为 `20`
- `name` 的值会被设置为 `Hello, RMVL`
- `value` 在 YAML 文件中为设置该值，因此 `value` 的值会保持不变，即为默认的 `4.2`
