参数模块使用教程 {#tutorial_table_of_content_rmvlpara}
============

@prev_tutorial{tutorial_table_of_content_modules}

@next_tutorial{tutorial_table_of_content_extra}

@tableofcontents

------

### 1. 参数模块简介

RMVL 中，一部分模块（涉及主要模块和扩展模块）的功能在不同运行环境或者不同需求的情况下需要有不同的参数配置，为此 RVML 将这些运行时可变的参数提炼出来，并且使用统一的管理方式，完成了参数类型、参数默认值、参数名、参数运行时加载的定义。

原先若有计划使用运行时参数，在

- 调整参数名、参数默认值
- 新增某一参数类（结构体）

的时候，需要在头文件以及对应源文件的位置写上大量重复的代码，管理极其不方便。为此 RMVL 参数模块定义了新的参数规范文件，这类文件以 `*.para` 为后缀，以及提供了 CMake 配置期间将 `*.para` 转换为对应 `*.h(hpp)` 以及 `*.cpp` 的 CMake 函数 `rmvl_generate_para`，该函数定义在 `<project>/cmake/RMVLGenPara.cmake` 文件中。

### 2. 参数规范文件

这里举一个 `*.para` 文件的示例，写法与 ROS Message 的 `*.msg` 文件类似，不过参数的类型均使用 C/C++ 中的类型，并且添加了注释功能。

```
#################### 光学参数 ####################
float exposure = 1500  # 相机曝光
float saturation = 100 # 相机饱和度
float gain = 64        # 相机数字增益
uint32_t b_gain = 1500 # 相机蓝色增益
uint32_t g_gain = 1500 # 相机绿色增益
uint32_t r_gain = 1500 # 相机红色增益
int auto_exposure = 0  # 相机自动曝光模式
int auto_wb = 0        # 相机自动白平衡模式

#################### 设备参数 ####################
int grab_mode = 1     # 相机 grab 模式
int retrieve_mode = 1 # 相机 retrieve 模式
string serial_number  # 相机序列号
```

这里列出参数规范文件的参数类型表

<center>表1：参数类型表

| 数据类型 | 含义                                                         |
| :------: | :----------------------------------------------------------- |
| 基本类型 | 1. 包括 `int`、`uint8_t`、`double`、`float`、`string` 等<br />2. 对标 C++ 的基础类型和 `std::string` |
| 矩阵类型 | 1. 包括形如 `Matx??`、`Vec?` 的类型，例如 `Matx22f`<br>2. 对标 OpenCV 的 `cv::Matx` 和 `cv::Vec`<br>3. 可使用列表初始化和相关静态函数初始化，例如 `Matx22f::eye()` |
| 复合类型 | 1. 包括 `vector` 和形如 `Point?` 的类型<br>2. 对标 C++ 的 `std::vector` 以及 OpenCV 的`cv::Point2?` 和 `cv::Point3?`<br>3. 只能使用列表初始化，例如 `{1, 2, 3}` |
| 枚举类型 | 1. 需要用户自定以 `enum` 开头和 `endenum` 结尾的数据类型声明<br />2. 对标 C++ 的有作用域枚举类型 `enum class`<br />3. 变量的定义上与有作用域枚举类型一致，例如 `Color COLOR_MODE = Color::RED` |

</center>

如果要给某一条参数设置默认值，需要使用 `=` 完成，不需要默认参数（仅由运行时加载设置）则不需要使用 `=` 进行赋值，例如

```
int grab_mode = 1
string serial_number
```

如果要给某一参数设置注释信息，请<span style="color: red">尾置</span> `#` 并输入相应注释信息，例如

```
float exposure = 1500 # 相机曝光
float gain = 64       # 相机数字增益
```

@note
- `#` 和注释信息之间<u>**不必**</u>使用空格分隔，例如 `#相机曝光`

枚举类型可在每个枚举项后加上具体的值，也能使用 `#` 为枚举项添加注释，例如

```
enum ColorMode # 颜色类型
  RED          # 红色
  GREEN        # 绿色
  BLUE = 4     # 蓝色
  GRAY = 5
endenum

ColorMode COLOR = ColorMode::RED # 颜色信息
```

### 3. C++ 代码生成

从 `*.para` 到对应的 C++ 代码生成的过程依赖 RMVL 提供的两条 CMake 函数，分别是

- `rmvl_generate_para`
- `rmvl_generate_module_para`

#### 3.1 rmvl_generate_para

根据指定的目标名在 param 文件夹下对应的 *.para 参数规范文件和可选的模块名生成对应的 C++ 代码。

**用法**

```cmake
rmvl_generate_para(
  <target_name>
  [MODULE module_name]
)
```

**示例**

```cmake
rmvl_generate_para(
  mytarget        # 目标名称
  MODULE mymodule # 模块名称为 mymodule
)
```

表示解析当前目录下的 `param/mytarget.para` 文件，并作为模块 `mymodule` 的子模块。最终生成的文件是

- `include/rmvlpara/mymodule/mytarget.h`，`include/rmvlpara` 后文简记为 `<prefix>`
- `src/para/mytarget.cpp`

这样的话会为参数模块汇总的头文件 `<prefix>/mymodule.hpp` 添加 `<prefix>/mymodule/mytarget.h` 文件的包含，即会在 `<prefix>/mymodule.hpp` 中添加

```cpp
#include "mymodule/mytarget.h"
```

如果 `mytarget` 需要自成一个模块的话，那么在调用 `rmvl_generate_para` 函数的时候应该写为

```cmake
rmvl_generate_para(mytarget)
```

这样就会生成一个名为 `mytarget` 的参数模块了，最终生成的文件是

- `include/rmvlpara/mytarget.hpp`
- `src/para/mytarget.cpp`

#### 3.2 rmvl_generate_module_para

根据给定模块下所有的 para 目标，生成对应的 C++ 代码

**用法**

```cmake
rmvl_generate_module_para(<module_name>)
```

**示例**

```cmake
rmvl_generate_module_para(mymodule)
```

表示将模块 `mymodule` 中的所有子模块打包至参数模块汇总头文件 `<prefix>/mymodule.hpp` ，例如，如果在 `CMakeLists.txt` 中写入

```cmake
rmvl_generate_para(a1 MODULE m)
rmvl_generate_para(a2 MODULE m)
rmvl_generate_para(a3 MODULE m)

rmvl_generate_module_para(m)
```

最后生成的 `<prefix>/m.hpp` 中就会包含

```cpp
#include "m/a1.h"
#include "m/a2.h"
#include "m/a3.h"
```

以上内容均是在 CMake 配置期间自动完成，只需定义好参数规范文件 `*.para` 以及相应的 CMake 函数即可。

### 4. 运行时加载

#### 4.1 加载方式

例如，对于以下的 `test.para` 文件

```
int abc = 10 # 测试数
```

在使用 `rmvl_generate_para(test)` 之后，生成的头文件代码中，类相关的代码如下

```
//! TestParam 参数模块
struct TestParam
{
    //! 测试数 @note 默认值：`10`
    int abc = 10;

    /**
     * @brief 从指定 `YAML` 文件中加载 `TestParam`
     *
     * @note `YAML` 文件的后缀允许是 `*.yml` 和 `*.yaml`
     * @param[in] path 参数路径
     * @return 是否加载成功
     */
    bool load(const std::string &path);
};

//! TestParam 参数模块
inline TestParam test_param;
```

其中包含了运行时加载的接口 `load`，以 @ref para_core 为例，`rm::para::CoreParam::load` 用于运行时读取 YAML 文件并配置 core 模块的参数。

用户可以通过使用类似以下的代码完成运行时加载

```cpp
/* code */
rm::para::core_param.load(prefix_path + "core.yml");
/* code */
```

#### 4.2 YAML 文件格式

