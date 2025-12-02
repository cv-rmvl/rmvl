Run In —— 一键进入全功能 RMVL 编译镜像{#tutorial_run_in}
============

@prev_tutorial{tutorial_use}
@next_tutorial{tutorial_other_arm}

@tableofcontents

------

## 基本用法

Run In 是一个用于快速进入 RMVL 编译镜像的工具，它可以根据 RMVL 项目根目录下的 platforms 文件中的 `images.yml` 文件，自动生成能够运行 Docker 编译镜像的目标。首先需要再 CMake 中开启该功能，使用

<div class="fragment">
<div class="line"><span class="keywordflow">cmake</span> <span class="comment">-D</span> ENABLE_RUNIN=ON ..</div>
</div>

即可开启 Run In 功能。

@note 后文假设使用 Makefile 作为构建工具，其他构建工具也一样使用。

开启 Run In 功能后，在终端输入 `make run_in_list` 即可列出所有可用的编译镜像。在终端输入 `make run_in_<name>` 即可运行对应的容器，并 **自动** 执行以下命令

<div class="fragment">
<div class="line"><span class="keywordflow">docker</span> <span class="keywordflow">run</span> <span class="comment">-it --rm </span>\\</div>
<div class="line">&nbsp;&nbsp;<span class="comment">-v</span> /path/to/rmvl:/path/to/rmvl \\</div>
<div class="line">&nbsp;&nbsp;<span class="comment">-v</span> /path/to/rmvl/build/docker_images/\<name\>:/path/to/rmvl/build \\</div>
<div class="line">&nbsp;&nbsp;<span class="comment">-w</span> /path/to/rmvl [options] \<image\> [cmd]</div>
</div>

上文的 `name`、`options`、`image` 和 `cmd` 都需要在 `images.yml` 的编译镜像 YAML 配置文件中定义，其中

- `name` 是必需参数，类型为 `str`，表示 YAML 文件中定义的名称
- `image` 是必需参数，类型为 `str`，表示 Docker 镜像的完整 URL
- `options` 是可选参数，类型为 `list[str]`，表示 Docker 运行时的选项，其中已经包含了 `-it` 以及 `--rm` 选项
- `cmd` 是可选参数，类型为 `str`，表示容器内执行的命令，默认为 `/bin/bash`

例如，YAML 配置文件中包含有

<div class="fragment">
<div class="line">- <span class="keywordtype">name</span>: <span class="stringliteral">dev</span></div>
<div class="line">&nbsp;&nbsp;<span class="keywordtype">image</span>: <span class="stringliteral">xxx/xxx/dev:v1</span></div>
<div class="line">&nbsp;&nbsp;<span class="keywordtype">options</span>:</div>
<div class="line">&nbsp;&nbsp;- <span class="stringliteral">--gpus all</span></div>
<div class="line">&nbsp;&nbsp;- <span class="stringliteral">-v /usr/local/cuda:/usr/local/cuda</span></div>
<div class="line">&nbsp;&nbsp;- <span class="stringliteral">-e LD_LIBRARY_PATH=/usr/local/cuda/lib64</span></div>
<div class="line">&nbsp;&nbsp;<span class="keywordtype">cmd</span>: <span class="stringliteral">/bin/zsh</span></div>
</div>

的内容，在终端输入 `make run_in_dev` 时，会执行

<div class="fragment">
<div class="line"><span class="keywordflow">docker</span> <span class="keywordflow">run</span> <span class="comment">-it --rm </span>\\</div>
<div class="line">&nbsp;&nbsp;<span class="comment">-v</span> /path/to/rmvl:/path/to/rmvl \\</div>
<div class="line">&nbsp;&nbsp;<span class="comment">-v</span> /path/to/rmvl/build/docker_images/dev:/path/to/rmvl/build \\</div>
<div class="line">&nbsp;&nbsp;<span class="comment">-w</span> /path/to/rmvl \\</div>
<div class="line">&nbsp;&nbsp;--gpus all \\</div>
<div class="line">&nbsp;&nbsp;-v /usr/local/cuda:/usr/local/cuda \\</div>
<div class="line">&nbsp;&nbsp;-e LD_LIBRARY_PATH=/usr/local/cuda/lib64 \\</div>
<div class="line">&nbsp;&nbsp;xxx/xxx/dev:v1 /bin/zsh</div>
</div>

一个最简单的配置项仅包含 `name` 和 `image` 两个字段，例如

<div class="fragment">
<div class="line">- <span class="keywordtype">name</span>: <span class="stringliteral">u22</span></div>
<div class="line">&nbsp;&nbsp;<span class="keywordtype">image</span>: <span class="stringliteral">ubuntu:22.04</span></div>
</div>

## 使用示例

在进入编译镜像前，可以执行

<div class="fragment">
<div class="line"><span class="keywordflow">make</span> run_in_list</div>
</div>

来查看当前可用的编译镜像，部分编译镜像会提示可用功能、交叉编译支持等信息，例如

```
3. run_in_tlt507: Tronlong T507 的 EtherCAT 环境支持，请使用 ~/LinuxSDK/toolchain.cmake 进行交叉编译
   image: registry.cn-hangzhou.aliyuncs.com/cv-rmvl/ethercat:latest
   options: -e LANG=C.UTF-8
   cmd: /bin/bash
```

然后可以使用

<div class="fragment">
<div class="line"><span class="keywordflow">make</span> run_in_tlt507</div>
</div>

来进入该编译镜像，进入后即可使用提示的交叉编译工具链文件进行交叉编译，例如

<div class="fragment">
<div class="line"><span class="keywordflow">cmake</span> <span class="comment">-D</span> CMAKE_TOOLCHAIN_FILE=~/LinuxSDK/toolchain.cmake ..</div>
<div class="line"><span class="keywordflow">make</span></div>
</div>

即可完成针对 Tronlong T507 平台的交叉编译，构建完成后，生成的文件会保存在宿主机的 `build/docker_images/tlt507` 目录下，可自行查看。