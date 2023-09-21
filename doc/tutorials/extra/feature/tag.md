AprilTag 视觉标签 {#tutorial_extra_april_tag}
============

@author 赵曦
@date 2023/09/20

@prev_tutorial{tutorial_extra_how_to_use_feature}

@next_tutorial{tutorial_extra_how_to_use_combo}

@tableofcontents

------

相关类 rm::Tag

### AprilTag 数据集

AprilTag 视觉标签数据集采用[机甲大师 RoboMaster S1 视觉标签](https://dl.djicdn.com/downloads/robomaster-s1/20190620/RoboMaster_S1_Vision_Markers_44pcs_15_15cm_updated.pdf)的内容，包含 `0~9` 共 10 个阿拉伯数字，`A~Z` 共 26 个拉丁字母，这些均可在 rm::TagType 枚举类型中找到。

数据集的所有视觉标签均为 Tag25h9 格式，可在 Github 中搜索 apriltag-generation 寻找视觉标签的生成程序。 @cite apriltag-gen 

@warning
- 目前暂不支持任意修改标签类型，使用上请确保标签类型正确
- 暂不支持除 Tag25h9以外的标签格式
