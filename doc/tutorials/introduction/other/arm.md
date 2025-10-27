为基于 ARM 的 Linux 系统手动配置交叉编译{#tutorial_other_arm}
============

@prev_tutorial{tutorial_run_in}
@next_tutorial{tutorial_document}

@tableofcontents

------

@note 建议使用基于 YAML 文件的 Docker 编译镜像构建方式，详见 @ref tutorial_run_in 。

### 交叉编译应具备的先决条件

* Linux 主机

* ![img](https://img.shields.io/badge/CMake-3.16+-green)

* ![img](https://img.shields.io/badge/OpenCV_for_ARM-4.2.0+-red) [点击此处下载最新发行版](https://github.com/opencv/opencv/releases/latest)

* ARM 的交叉编译工具：gcc，libstdc++.so 等内容，根据目标平台的不同，您需要选择 `gneabi` 或 `gneabihf` 工具。

`gneabi` 安装命令如下

<div class="fragment">
<div class="line"><span class="keyword">sudo</span> <span class="keywordflow">apt</span> install gcc-arm-linux-gneabi</div>
</div>

`gneabihf` 安装命令如下

<div class="fragment">
<div class="line"><span class="keyword">sudo</span> <span class="keywordflow">apt</span> install gcc-arm-linux-gneabihf</div>
</div>

### 使用提供的工具链构建 RMVL

在 RMVL 目录下创建 `build` 文件夹，运行以下命令

<div class="fragment">
<div class="line"><span class="keywordflow">mkdir</span> build</div>
<div class="line"><span class="keywordflow">cd</span> build</div>
<div class="line"><span class="keywordflow">cmake</span> <span class="comment">-D</span> CMAKE_TOOLCHAIN_FILE=../platforms/linux/arm-gnueabi.toolchain.cmake ..</div>
</div>

上文代码中的 CMake 配置也可使用 `cmake-gui` 或者 `ccmake` 工具，例如使用 `ccmake` 则需要改写为

<div class="fragment">
<div class="line"><span class="keywordflow">ccmake</span> <span class="comment">-D</span> CMAKE_TOOLCHAIN_FILE=../platforms/linux/arm-gnueabi.toolchain.cmake ..</div>
</div>

运行构建命令

<div class="fragment">
<div class="line"><span class="keywordflow">cmake</span> <span class="comment">--build</span> . <span class="comment">--parallel</span> 4</div>
</div>

### 自定义工具链构建 RMVL

如果交叉编译器是通过编译内核源码获取到的，在使用这类指定的交叉编译器进行编译时，自定义工具链是个有效的方法。

工具链文件的一般写法如下

<div class="fragment">
<div class="line"><span class="comment"># toolchain.cmake</span></div>
<div class="line"></div>
<div class="line"><span class="keyword">set</span>(CMAKE_SYSTEM_NAME Linux)</div>
<div class="line"><span class="keyword">set</span>(CMAKE_SYSTEM_PROCESSOR aarch64)</div>
<div class="line"></div>
<div class="line"><span class="keyword">set</span>(CMAKE_C_COMPILER <span class="stringliteral">"/path/to/xxx-gcc"</span>)</div>
<div class="line"><span class="keyword">set</span>(CMAKE_CXX_COMPILER <span class="stringliteral">"/path/to/xxx-g++"</span>)</div>
<div class="line"></div>
<div class="line"><span class="keyword">set</span>(CMAKE_FIND_ROOT_PATH <span class="stringliteral">"/path/to/"</span>)</div>
<div class="line"><span class="keyword">set</span>(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM <span class="keyword">NEVER</span>)</div>
<div class="line"><span class="keyword">set</span>(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY <span class="keyword">ONLY</span>)</div>
<div class="line"><span class="keyword">set</span>(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE <span class="keyword">ONLY</span>)</div>
</div>

然后进入 RMVL 项目根目录，在终端中输入

<div class="fragment">
<div class="line"><span class="keywordflow">cmake</span> <span class="comment">-D</span> CMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake ..</div>
</div>

同样，CMake 配置也可使用 `cmake-gui` 或者 `ccmake` 工具。
