Run In：通过简单的 YAML 文件构建全功能的 RMVL{#tutorial_run_in}
============

@prev_tutorial{tutorial_use}
@next_tutorial{tutorial_other_arm}

@tableofcontents

------

## 1. 基本用法

@note 后文假设使用 Makefile 作为构建工具，其他构建工具也一样使用。

在终端输入 `make run_in_<name>` 即可运行对应的容器，并执行以下命令

```bash
docker run -it --rm \
  -v /path/to/rmvl:/path/to/rmvl \
  -v /path/to/rmvl/build/docker_images/<name>:/path/to/rmvl/build \
  -w /path/to/rmvl [options] <image> [cmd]
```

其中

- `name` 是必需参数，类型为 `str`，表示 YAML 文件中定义的名称
- `image` 是必需参数，类型为 `str`，表示 Docker 镜像的完整 URL
- `options` 是可选参数，类型为 `list[str]`，表示 Docker 运行时的选项，其中已经包含了 `-it` 以及 `--rm` 选项
- `cmd` 是可选参数，类型为 `str`，表示容器内执行的命令，默认为 `/bin/bash`

例如，YAML 文件中包含有

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
