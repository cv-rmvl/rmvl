视觉库的打包与安装 {#tutorial_install}
============

@prev_tutorial{tutorial_build}
@next_tutorial{tutorial_use}

@tableofcontents

------

### 1. 安装

在构建完成之后，可以通过 `make install` 来安装 RMVL 的头文件、库文件、和 CMake 配置文件

```shell
sudo make install
```

同样可以选择线程数来加速安装过程

```shell
sudo make install -j4
```

### 2. 测试

在任意一个地方打开终端，输入

```shell
rmvl_version
```

如果显示了对应的版本号，则安装成功。同时，也可查看构建时的配置情况

```shell
rmvl_version -v
```
