如何使用/开发 tracker 模块 {#tutorial_extra_how_to_use_tracker}
============

@author 赵曦
@date 2023/07/07

@prev_tutorial{tutorial_extra_how_to_use_tracker}

@next_tutorial{tutorial_extra_how_to_use_group}

@tableofcontents

------

基类 rm::tracker

## 1. 如何使用

### 1.1 有关构造与创建

追踪器 `tracker` 对象创建方法与特征组合 `combo` 以及图像特征 `feature` 基本一致，均使用静态工厂函数，这里是使用 `make_tracker` 完成构建。

构造一个平面目标追踪器的代码如下

```cpp
auto p_tracker = rm::PlanarTracker::make_tracker(p_combo); // p_combo 是已经存在的追踪器，并且不能为 nullptr
```

rm::tracker 提供了 `clone` 纯虚拟函数，用于完全复制一份数据。

### 1.2 信息获取

和低层数据组件类似， @ref tracker 内部维护了各时间下特征组合的相关信息，即特征组合的时间序列。

#### 1.2.1 数据信息

`tracker` 记录了时间上彼此关联的一组 `combo`，内部使用 `std::deque<combo::ptr>` 表示。`tracker` 提供了与 `std::deque` 容器类似的访问操作，包括

<center>

表 1: tracker 信息获取接口

| 信息获取接口 |                        实现功能                        |
| :----------: | :----------------------------------------------------: |
|  `at(idx)`   |                  获取指定的下标`idx`                   |
|  `front()`   |             获取特征组合时间序列最新的元素             |
|   `back()`   |             获取特征组合时间序列最旧的元素             |
|   `data()`   | 获取`std::deque<combo::ptr>`（特征组合时间序列）的数据 |
|   `size()`   |               获取特征组合时间序列的大小               |
|  `empty()`   |              判断特征组合时间序列是否为空              |

</center>

#### 1.2.2 通用属性与派生类属性

和低层数据组件类似一样，包含高度、宽度、角度、中心点、角点列表、类型信息、相机外参等内容，但主要表示经过修正后的数据，例如 `height` 成员方法表示追踪器在通过对时间序列中所有特征组合进行处理，得到的修正后的高度信息（譬如加权平均）。访问方式与 `combo`、`feature` 完全一致，这里不再赘述。

## 2. 如何开发

### 2.1 基本准则

开发与使用是一脉相承的，设计一个新的 rm::tracker 派生类对象也要满足使用上的条件，一个新的派生类对象（假设定义为`Mytracker`）需要满足以下准则。

1. 必须定义在 `namespace rm` 中，下文不再赘述；
2. 必须 public 继承于 `rm::combo` 基类；
3. 必须定义 `Mytracker::ptr` 作为 `std::shared_ptr<Mytracker>` 的别名；
4. 必须实现以 `Mytracker::ptr` 为返回值的 `Mytracker::make_feature` 静态工厂函数；
5. 不得定义公开数据成员，避免对数据成员的直接操作，设置、获取操作应该使用形如 `setXXX` 或 `getXXX` 的成员方法；
6. 需要定义好 `ptr`、`const_ptr` 智能指针类型别名，并分别实现 `cast` 转换函数

