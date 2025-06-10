Run In —— 一键进入全功能 RMVL 编译镜像{#tutorial_run_in}
============

@prev_tutorial{tutorial_use}
@next_tutorial{tutorial_other_arm}

@tableofcontents

------

## 基本用法

Run In 是一个用于快速进入 RMVL 编译镜像的工具，它可以根据 RMVL 项目根目录下的 platforms 文件中的 `images.yml` 文件，自动生成能够运行 Docker 编译镜像的目标。首先需要再 CMake 中开启该功能，使用

```bash
cmake -D ENABLE_RUNIN=ON ..
```

即可开启 Run In 功能。

@note 后文假设使用 Makefile 作为构建工具，其他构建工具也一样使用。

开启 Run In 功能后，在终端输入 `make run_in_list` 即可列出所有可用的编译镜像。在终端输入 `make run_in_<name>` 即可运行对应的容器，并 **自动** 执行以下命令

```bash
docker run -it --rm \
  -v /path/to/rmvl:/path/to/rmvl \
  -v /path/to/rmvl/build/docker_images/<name>:/path/to/rmvl/build \
  -w /path/to/rmvl [options] <image> [cmd]
```

上文的 `name`、`options`、`image` 和 `cmd` 都需要在 `images.yml` 的编译镜像 YAML 配置文件中定义，其中

- `name` 是必需参数，类型为 `str`，表示 YAML 文件中定义的名称
- `image` 是必需参数，类型为 `str`，表示 Docker 镜像的完整 URL
- `options` 是可选参数，类型为 `list[str]`，表示 Docker 运行时的选项，其中已经包含了 `-it` 以及 `--rm` 选项
- `cmd` 是可选参数，类型为 `str`，表示容器内执行的命令，默认为 `/bin/bash`

例如，YAML 配置文件中包含有

```yaml
- name: dev
  image: xxx/xxx/dev:v1
  options:
  - --gpus all
  - -v /usr/local/cuda:/usr/local/cuda
  - -e LD_LIBRARY_PATH=/usr/local/cuda/lib64
  cmd: /bin/zsh
```

的内容，在终端输入 `make run_in_dev` 时，会执行

```bash
docker run -it --rm \
  -v /path/to/rmvl:/path/to/rmvl \
  -v /path/to/rmvl/build/docker_images/dev:/path/to/rmvl/build \
  -w /path/to/rmvl \
  --gpus all \
  -v /usr/local/cuda:/usr/local/cuda \
  -e LD_LIBRARY_PATH=/usr/local/cuda/lib64 \
  xxx/xxx/dev:v1 /bin/zsh
```

一个最简单的配置项仅包含 `name` 和 `image` 两个字段，例如

```yaml
- name: u22
  image: ubuntu:22.04
```
