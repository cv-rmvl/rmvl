                                        第三方模块

本文件夹包含 extra 模块中涉及到的一些流行的视觉模块的第三方库，包括一些常见的视觉标签的定位、解码。

在 Unix 系统上，所有的库都是通过 CMake 脚本自动检测的，为了在 Unix 系统上使用这些版本的库而不是系统
库，应该使用 BUILD_<library_name> 的 CMake 标志 (例如，apriltag 库的 BUILD_APRILTAG)。

---------------------------------------------------------------------------------------------------
apriltag            描述        用于定位的视觉基准系统
                    许可与版权  apriltag 由 BSD 2-Clause 许可证覆盖，参见 apriltag/LICENSE.md
                    官网主页    https://april.eecs.umich.edu/software/apriltag
                    CMake 选项  BUILD_APRILTAG 开启后可构建此模块（默认开启）
                                WITH_APRILTAG 启用该选项来为 tag_detector 模块提供 apriltag 支持


