如何使用/开发 combo 模块 {#tutorial_extra_how_to_use_combo}
============

@author 赵曦
@date 2023/07/07
@brief 包含 combo 模块的创建、信息获取以及开发时的注意事项

@prev_tutorial{tutorial_extra_april_tag}

@next_tutorial{tutorial_extra_how_to_use_tracker}

@tableofcontents

------

基类 rm::combo

## 1. 如何使用

### 1.1 有关构造与创建

特征组合 `combo` 对象创建方法与 `feature` 基本一致，都是使用静态工厂函数，这里是使用 `make_combo` 完成构建，同样会在最初进行能否构造的判断。

构造一个装甲板的代码如下

```cpp
auto p_combo = rm::Armor::make_combo(p_left, p_right, gyro_data, tick);
```

rm::combo 提供了 `clone` 纯虚拟函数，用于完全复制一份数据，适合于强制构造无视匹配要求的情况。

### 1.2 信息获取

@ref combo 与 @ref feature 在信息获取上基本一致，但 @ref combo 内部维护了图像特征相关的信息，这一类数据在获取上与 STL 容器的操作基本一致。

#### 1.2.1 数据信息

`combo` 记录了空间上彼此关联的一组 `feature`，内部使用 `std::vector<feature::ptr>` 表示。`combo` 提供了与序列式容器类似的访问操作，包括

<center>

表 1: combo 信息获取接口

| 信息获取接口 |                     实现功能                      |
| :----------: | :-----------------------------------------------: |
|  `at(idx)`   |                获取指定的下标`idx`                |
|   `data()`   | 获取`std::vector<feature::ptr>`（特征列表）的数据 |
|   `size()`   |                获取特征列表的大小                 |
|  `empty()`   |               判断特征列表是否为空                |

</center>

#### 1.2.2 通用属性与派生类属性

包含高度、宽度、角度、中心点、角点列表、类型信息、相机外参等内容，访问方式与 `feature` 完全一致，这里不再赘述。

## 2. 如何开发

### 2.1 基本准则

开发与使用是一脉相承的，设计一个新的 rm::combo 派生类对象也要满足使用上的条件，一个新的派生类对象（假设定义为`MyCombo`）需要满足以下准则。

1. 必须定义在 `namespace rm` 中，下文不再赘述；
2. 必须 public 继承于 `rm::feature` 基类；
3. 必须定义 `MyCombo::ptr` 作为 `std::shared_ptr<MyCombo>` 的别名；
4. 必须实现以 `MyCombo::ptr` 为返回值的 `MyCombo::make_feature` 静态工厂函数；
5. 不得定义公开数据成员，避免对数据成员的直接操作，设置、获取操作应该使用形如 `setXXX` 或 `getXXX` 的成员方法；
6. 需要定义好 `ptr`、`const_ptr` 智能指针类型别名，并分别实现 `cast` 转换函数
