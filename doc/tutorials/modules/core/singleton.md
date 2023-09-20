单例模板 {#tutorial_modules_singleton}
============

@author 赵曦
@date 2022/02/09

@next_tutorial{tutorial_modules_aggregate_reflect}

@tableofcontents

------

相关类 rm::GlobalSingleton

### 1. 如何配置

在 CMakeLists.txt 中链接目标库
  
```cmake
target_link_libraries(
    xxx
    rmvl_singleton
)
```

### 2. 如何使用

#### 2.1 包含头文件
  
```cpp
#include <rmvl/singleton.hpp>
```
  
#### 2.2 定义或使用现有的类
  
```cpp
class MyClass
{
    int _a;
    double _b;
    const char *_c;

public:
    MyClass(int a, double b, const char *c) :
        _a(a), _b(b), _c(c) {}
    
    inline int getA() { return _a; }
    inline double getB() { return _b; }
    inline const char *getC() { return _c; }
};
```
  
#### 2.3 创建单例
  
```cpp
GlobalSingleton<MyClass> my_class;
// Pass in the arguments specified by the constructor
my_class.New(1, 3.14, "hello");
```

#### 2.4 获取单例

```cpp
MyClass* tmp = my_class.Get();
std::cout << "a = " << tmp->getA() << std::endl;
std::cout << "b = " << tmp->getB() << std::endl;
std::cout << "c = " << tmp->getC() << std::endl;
```

#### 2.5 销毁单例

```cpp
my_class.Delete();
```