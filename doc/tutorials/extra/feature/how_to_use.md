如何使用/开发 feature 模块 {#tutorial_extra_how_to_use_feature}
============

@author 赵曦
@date 2023/07/07
@brief 包含 feature 模块的创建、信息获取以及开发时的注意事项

@next_tutorial{tutorial_extra_april_tag}

@tableofcontents

------

基类 rm::feature

## 1. 如何使用

### 1.1 有关构造与创建

对于对象的创建，应该使用 `make_feature` 静态工厂函数，尽量避免直接使用构造函数。`make_feature` 方法内部首先会进行能否构造的判断，而不是直接构造，如果条件满足则会创建对应的类对象，并返回其共享指针，如果条件不满足则直接返回 `nullptr`。

例如，要构造一个装甲板灯条对象，可以使用以下语句。

```cpp
// 入参是用 std::vector<cv::Point> 所表示的轮廓
rm::feature::ptr p_light_blob = rm::LightBlob::make_feature(contour);
```

rm::feature 提供了 `clone` 纯虚拟函数，用于完全复制一份数据，适合于强制构造无视匹配要求的情况。

### 1.2 信息获取

#### 1.2.1 通用属性

rm::feature 由于是扩展模块中最底层的数据组件，因此不存在数据结构类型的信息，仅有包含特征对应属性的信息。特征类中包含了众多属性，有高度、宽度、角度、中心点、角点列表等，以下是 rm::feature 对于这些方法的定义。

```cpp
// 获取特征面积
inline float area() const { return _height * _width; }
// 获取特征中心点
inline const cv::Point2f &center() const { return _center; }
// 获取特征宽度
inline float width() const { return _width; }
// 获取特征高度
inline float height() const { return _height; }
// 获取特征角度
inline float angle() const { return _angle; }
// 获取特征角点
inline const std::vector<cv::Point2f> &corners() const { return _corners; }
```

#### 1.2.2 派生类属性

对于一些派生的特征特有的属性，比如装甲板灯条 rm::LightBlob 的上顶点，使用上需要额外注意类型转换的内容。一般我们在一些功能模块中，例如 @ref detector 中直接操纵的都是抽象类的共享指针 `rm::feature::ptr`，如果需要转换成派生类对象，可利用共享指针 `std::shared_ptr` 中对于动态类型转换的函数：`std::dynamic_pointer_cast`，比如想得到 rm::LightBlob 的共享指针，可以使用以下命令。

```cpp
auto p_light_blob = std::dynamic_pointer_cast<rm::LightBlob>(p_feature);
```

代码比较冗长，不过好在所有的派生类都提供了一个转换接口 `cast`，为此我们可以像下面的代码一样，更方便的完成动态类型转换。

```cpp
auto p_light_blob = rm::LightBlob::cast(p_feature);
```

有了动态类型转换的接口就可以继续获取派生类属性，下面的代码展示了如何获取装甲板灯条的上顶点。

```cpp
auto p_light_blob = rm::LightBlob::cast(p_feature);
auto top_point = p_light_blob->getTopPoint();
```

## 2. 如何开发

### 2.1 基本准则

开发与使用是一脉相承的，设计一个新的 rm::feature 派生类对象也要满足使用上的条件，一个新的派生类对象（假设定义为`MyFeature`）需要满足以下准则。

1. 必须定义在 `namespace rm` 中，下文不再赘述；
2. 必须 public 继承于 `rm::feature` 基类；
3. 必须定义 `MyFeature::ptr` 作为 `std::shared_ptr<MyFeature>` 的别名；
4. 必须实现以 `MyFeature::ptr` 为返回值的 `MyFeature::make_feature` 静态工厂函数；
5. 不得在派生类中定义公开数据成员，避免对数据成员的直接操作，设置、获取操作应该使用形如 `setXXX` 或 `getXXX` 的成员方法；
6. 需要定义好 `ptr`、`const_ptr` 智能指针类型别名，并分别实现 `cast` 转换函数
