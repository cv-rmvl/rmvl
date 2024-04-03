为 RMVL 撰写说明文档 {#tutorial_document}
============

@prev_tutorial{tutorial_other_arm}

@tableofcontents

------

Doxygen 概述 {#tutorial_documentation_overview}
============

介绍 {#tutorial_documentation_intro}
-----

[Doxygen] 是用于构建文档系统的工具，包含非常多的实用特性，包括：
- 通过解析程序源代码来生成正确的文档
- 检查文档中的错误
- 插入图像和公式
- 使用 Markdown 标记语法，以及纯粹的 HTML 文本格式
- 以不同的格式生成文档，例如 HTML、LaTeX、man 等

安装 {#tutorial_documentation_install}
-----

可在官方网站的[下载][Doxygen download]和[安装][Doxygen installation]页面进行查看，一些 Linux 发行版也提供了有关 Doxygen 的包，例如 Ubuntu 可通过以下命令进行安装。
```shell
sudo apt install doxygen doxygen-gui graphviz
```

生成文档 {#tutorial_documentation_generate}
-----

- 获取 RMVL 源码
- 在项目文件夹顶层创建 `build` 文件夹，并 `cd build`
- 运行 `cmake`，或使用 `cmake-gui`
  @code{.sh}
  cmake -DBUILD_DOCS=ON ..
  @endcode
- 运行 `make`
  @code{.sh}
  make doxygen
  @endcode
- 在你的浏览器中打开 <i>'doc/doxygen/html/index.html'</i> 文件

快速开始 {#tutorial_documentation_quick_start}
============

@note 以下内容为 RMVL 特有的文档，若是自己书写的其他项目，文档的布局、协议都可以视需求情况而定。

文档布局 {#tutorial_documentation_layout}
-----

有关文档的一切内容均放置在 `<path-to-rmvl>/doc` 中，下面列举一些关于文档生成重要的部分

- **Doxyfile.in** - 由 cmake 自动生成 Doxyfile，来为 `make doxygen` 提供文档构建指南。内部定义了有关文档构建范式的所有内容，包括构建类型（HTML、LaTeX 等）、构建工作空间、源代码中的构建内容等；

- **doxygen-awesome.css** - 样式文件，源自于 GitHub 的 [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) 项目；

- **header.html, footer.html** - 定义了页眉与页脚的格式，其中包含一些 js 脚本的导入；

- **root.markdown.in** - 由 cmake 自动生成 root.markdown，作为文档首页

此外，RMVL 的文档在构建中会自动递归索引项目顶级目录下的所有文件，最终生成包含以下几个部分

- **源代码** 中的实体，包括类、函数、枚举等内容，这些内容需要被定义在相应的头文件中。

  @note 自动的代码解析会寻找所有的头文件（<i>".h, .hpp"</i>）以及部分源文件（<i>"sample_*.cpp"</i>）的内容

- **页面** 是放置与任何源代码实体不直接连接的图像和代码示例的大块文本的区域，页面应该位于单独的文件中，并包含在几个预定义的位置，比如说本教程就是这样一个页面的例子。

- **图像** 可以用来说明所描述的事物，这些文件通常位于与页面相同的位置，可以插入到文档的任何位置。

- **示例代码** 展示了如何在实际应用程序中使用该库，每个示例都是代表一个简单应用程序的自包含文件。这些文件的部分内容可以包含在文档和教程中，以演示函数调用和对象协作。

- **参考书目** 用于创建一个公共的书目，所有作为 RMVL 功能基础的科学书籍、文章和论文集都应列入此参考书目。

详细内容 {#tutorial_documentation_content}
-----

为了给类、函数或其他实体生成文档，需要使用特定的注释方式，比如说

@verbatim
/**
 * @brief 计算相机中心与目标中心之间的相对角度
 * @note 公式推导参考函数 @ref calculateRelativeCenter() ，现直接给出结果 \f[\begin{bmatrix}
 *       \tan{yaw}\\\tan{pitch}\\1\end{bmatrix}=\begin{bmatrix}f_x&0&u_0\\0&f_y&v_0\\0&0&1
 *       \end{bmatrix}^{-1}\begin{bmatrix}u\\v\\1\end{bmatrix}\f]
 *
 * @param[in] cameraMatrix 相机内参矩阵
 * @param[in] center 像素坐标系下的目标中心
 * @return 相对角度，目标在图像右方，point.x 为正，目标在图像下方，point.y 为正
 */
cv::Point2f calculateRelativeAngle(const cv::Matx33f &cameraMatrix, cv::Point2f center);
@endverbatim

### 1. 基本命令

- __brief__ - 表示一段函数、类、枚举、结构的简要信息

- __param__ - 通常表示函数的参数，包含传入、传出、传入兼传出参数 3 种，可分别用 `[in]`、`[out]` 和 `[in out]` 做区分，例如
  @verbatim
  @param[in] cameraMatrix 相机内参矩阵
  @endverbatim
  - 结果如下
    @param[in] cameraMatrix 相机内参矩阵

- __note__ - 注解，可包含对类、函数的详细介绍或者使用注意事项，也可包含一些公式信息，具体的公式写法在后续的 __f__ 会进行介绍，note 的使用如下
  @verbatim
  @note 公式推导参考函数 @ref rm::calculateRelativeCenter
  @endverbatim
  - 结果如下
    @note 公式推导参考函数 @ref rm::calculateRelativeCenter

- __return__ - 表示返回值
  @verbatim
  @return 相对角度
  @endverbatim
  - 结果如下
    @return 相对角度

- __f__ - 公式
  内联公式使用 `f$` 命令开始
  @verbatim
  \f$ ... \f$
  @endverbatim
  块公式使用 `f[` 和 `f]` 命令
  @verbatim
  \f[ ... \f]
  @endverbatim
  示例
  @verbatim
  若状态向量包含以下内容：\f$[p, v, a]\f$ ，然而观测向量仅包含 \f$[p, v]\f$，
  在这种情况下，需要使用一个观测转换矩阵 \f$H_{2\times3}\f$。在上述例子中可表示为
  \f[\begin{bmatrix}p\\v\end{bmatrix}=\begin{bmatrix}1&0&0\\0&1&0\end{bmatrix}
  \begin{bmatrix}p\\v\\a\end{bmatrix}\f]
  @endverbatim
  - 结果如下
    
    若状态向量包含以下内容：\f$[p, v, a]\f$ ，然而观测向量仅包含 \f$[p, v]\f$，
    在这种情况下，需要使用一个观测转换矩阵 \f$H_{2\times3}\f$。在上述例子中可表示为
    \f[\begin{bmatrix}p\\v\end{bmatrix}=\begin{bmatrix}1&0&0\\0&1&0\end{bmatrix}
    \begin{bmatrix}p\\v\\a\end{bmatrix}\f]

- __defgroup__ 与 __addtogroup__ - 

### 2. 页面相关

#### 2.1 Markdown 支持

Doxygen 支持部分 Markdown 语法与扩展，本篇教程页面同样使用了很多的 Markdown 语法。

#### 2.2 标题 ID

每个文档页面都应该为自己设置一个 ID，比如本篇教程的标题和 ID 如下

@verbatim
为 RMVL 撰写说明文档 {#tutorial_document}
@endverbatim

#### 2.3 子页面与跳转

使用以下命令进行子页面的跳转

@verbatim
@subpage xxx
@endverbatim

使用以下命令进行页面内或其他地方的引用

@verbatim
@ref xxx
@endverbatim

### 3. 代码块

使用以下命令创建代码块

@verbatim
@code{.cpp}
std::cout << "Hello World" << std::endl;
@endcode
@endverbatim

渲染结果如下

@code{.cpp}
std::cout << "Hello World" << std::endl;
@endcode

### 4. 按钮

使用以下命令创建按钮

@verbatim

### 按钮示例

@add_toggle{C++}

    This is C++.

    @code{.cpp}
    std::cout << "Hello World" << std::endl;
    @endcode

@end_toggle

@add_toggle{Python}

    This is Python.

    @code{.py}
    print("Hello World")
    @endcode

@end_toggle

@endverbatim

渲染结果如下

### 按钮示例

@add_toggle_cpp

  This is C++.
  ```cpp
  std::cout << "Hello World" << std::endl;
  ```

@end_toggle

@add_toggle_python

  This is Python.
  ```py
  print("Hello World")
  ```

@end_toggle

参考 {#tutorial_documentation_refs}
==========
- [Doxygen] - Doxygen 主页面
- [Documenting basics] - 如何在代码中包含文档内容
- [Markdown support] - 支持的标记与扩展
- [Formulas support] - 如何包含公式

<!-- invisible references list -->
[Doxygen]: http://www.doxygen.nl
[Doxygen download]: http://doxygen.nl/download.html
[Doxygen installation]: http://doxygen.nl/manual/install.html
[Documenting basics]: http://www.doxygen.nl/manual/docblocks.html
[Markdown support]: http://www.doxygen.nl/manual/markdown.html
[Formulas support]: http://www.doxygen.nl/manual/formulas.html
