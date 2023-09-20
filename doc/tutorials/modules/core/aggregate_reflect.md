聚合类反射及其相关 API {#tutorial_modules_aggregate_reflect}
============

@author 赵曦
@date 2023/09/19

@prev_tutorial{tutorial_modules_singleton}

@next_tutorial{tutorial_modules_serial}

@tableofcontents

------

### 1. 聚合类

有关[聚合体](https://zh.cppreference.com/w/cpp/language/aggregate_initialization)的概念可直接参考 cppreference 手册，这里对聚合体的其中一个用法做介绍。

数组类型同样属于聚合体，这里仅探究符合聚合体定义的类类型，我们称其为聚合类，聚合类允许聚合初始化，即从初始化列表初始化聚合类，例如

```cpp
struct Person
{
    int age;
    double height;
    std::string name;
};

void init()
{
    Person p3{18, 1.78, "zhaoxi"};
}
```

cppreference 给出聚合初始化的一个效果是

按以下规则决定显式初始化元素:

- **性质 1** : 如果初始化列表是[指派初始化列表](https://zh.cppreference.com/w/cpp/language/aggregate_initialization#.E6.8C.87.E6.B4.BE.E5.88.9D.E5.A7.8B.E5.8C.96.E5.99.A8)（聚合体此时只能具有类类型），每个指派符中的标识符必须指名该类的一个非静态数据成员，并且聚合体中显式初始化元素是这些成员或包含这些成员的元素。 `(C++20 起)`
- **性质 2** : 否则，如果初始化列表不为空，那么显式初始化元素是聚合体中的前 n 个元素，其中 n 是初始化列表中的元素数量。
- **性质 3** : 否则初始化列表必须为空，并且不存在显式初始化元素。 
  
根据 **性质 2** ，聚合初始化的代码写成如下形式

```cpp
void init()
{
    Person p1{18};                 // OK
    Person p2{18, 1.78};           // OK
    Person p3{18, 1.78, "zhaoxi"}; // OK
}
```

这三种都是正确的，但是初始化列表中的元素个数不能超过聚合体的元素个数。

```cpp
void init()
{
    Person p4{18, 1.78, "zhaoxi", 'a'}; // 非良构
}
```

根据这一特点我们可以实现

- 编译期间得知任意聚合体 `T`（这里给出的是聚合体，说明数组也成立）的元素个数
- 遍历聚合体的所有元素

的编译期反射机制。

### 2. 获取聚合体元素个数

下文给出 C++17 和 C++20 两个版本的实现方法，点击对应按钮可查看详细内容

@see core/util.hpp

------

@add_toggle{C++17 实现}

由 **性质 2** 可以知道，对于元素个数为 \f$n\f$ 的任意聚合体类型 `T`，在 `T` 类实例化对象的时候，初始化列表元素个数 \f$m\f$ 需要满足 \f$m\le n\f$ ，如果 \f$m>n\f$ 则程序非良构。

我们可以有这样一个基本想法，可以给定一个比较大的 \f$m\f$ 作为初值进行构造尝试，如果构造不成功则继续构造 \f$m-1\f$ 的，直到成功构造为止，此时就满足 \f$m=n\f$，因此返回此时的 \f$m\f$ 值即可作为当前聚合体的元素个数 \f$n\f$。

由于 C++17 缺乏概念机制，因此我们需要通过人为制造具有若干重载版本的函数，并利用模板函数返回类型、形参类型的替换失败来实现该功能，一个最经典的做法是 [SFINAE](https://zh.cppreference.com/w/cpp/language/sfinae)。可以通过函数模板形参在发生替换时非良构，从而删除该函数的其中之一个特化版本。

有了以上语言特性的支撑，我们回到问题最开始的地方 **给定一个比较大的 m 作为初值进行构造尝试** 。对于任意聚合体类型 `T`，其元素类型也是任意的，我们通过一个包含不求值表达式 `decltype` 的后置返回类型的模板函数，来实现大小为 2 的函数特化版本，下面给出一个例子。

```cpp
template <typename T>
constexpr auto size() -> decltype(T{/* exp */, /* exp */}, 0u) { return 2; }
```

其中 `/* exp */` 暂时理解为可转化为任意类型的表达式。当 \f$m>2\f$ 时，会发生替换失败，当 \f$m=2\f$ 时可以正常返回 `2`，我们可以定义一个包含任意类型的[用户定义转换函数](https://zh.cppreference.com/w/cpp/language/cast_operator)的辅助类来完成 `/* exp */` 所代表的功能。

```cpp
struct init
{
    template <typename Tp>
    operator Tp();
};
```

该辅助类 `init` 提供了任意类型的用户定义转换函数，但无需实现，因为用在 `decltype` 不求值表达式中。

为此，汇总所有信息，可以得到以下代码。

```cpp
namespace helper
{

// Constructor helper
struct init
{
    template <typename Tp>
    operator Tp(); // No need to define
};

template <std::size_t N>
struct size_tag : size_tag<N - 1> {};
template <>
struct size_tag<0> {};

/* template <typename Tp> auto size(size_tag<more than 4>) ... */
/* template <typename Tp> auto size(size_tag<4>) ... */
template <typename Tp>
constexpr auto size(size_tag<3>) -> decltype(Tp{init{}, init{}, init{}}, 0u) { return 3u; }
template <typename Tp>
constexpr auto size(size_tag<2>) -> decltype(Tp{init{}, init{}}, 0u) { return 2u; }
template <typename Tp>
constexpr auto size(size_tag<1>) -> decltype(Tp{init{}}, 0u) { return 1u; }
template <typename Tp>
constexpr auto size(size_tag<0>) -> decltype(Tp{}, 0u) { return 0u; }

} // namespace helper

template <typename Tp>
constexpr std::size_t size()
{
    static_assert(std::is_aggregate_v<Tp>);
    return helper::size<Tp>(helper::size_tag<3>{});
}
```

其中涉及到了一个 `size_tag` 类，该类有一个 `<0>` 的全特化，`<N>` 继承于 `<N - 1>`，是为了在 `helper::size` 重载中，能够按 `N` 从大到小的顺序进行重载决议。下面给出一个简单的使用示例。

```cpp
#include <rmvl/core/util.hpp>

struct T
{
    int a{};
    std::string b{};
};

int main()
{
    std::cout << rm::size<T>() << std::endl; // 输出 2
}
```

@end_toggle

@add_toggle{C++20 实现}

C++20 有了概念的机制，可以不通过 SFINAE 机制完成。我们直接上代码

```cpp
template <typename Tp>
consteval std::size_t size(auto &&...args)
{
    static_assert(std::is_aggregate_v<Tp>);
    if constexpr (!requires { Tp{args...}; })
        return sizeof...(args) - 1;
    else
        return size<Tp>(args..., helper::init{});
}
```

首先描述一下 `size` 模板函数的实现，它是一个递归函数，并且是编译期的递归函数，因为使用了 `if constexpr`，并且这个函数是以 `consteval` 修饰的，是[立即函数](https://zh.cppreference.com/w/cpp/language/consteval)，即该函数必须在编译期运行并产生编译期常量，因此没有运行时开销。

`requires` 语句指明，如果 `Tp` 可以按照 `args...` 的方式进行构建，那么就进入 `else` 分支，否则返回 `形参包长度 - 1`。根据上文提到的 **性质 2** 可以知道，当 `Tp` 聚合体在初始化时，初始化列表的元素个数如果符合 `Tp` 构造的要求，即能够从 `args...` 形参包完成 `Tp` 的构造，那么在 `else` 分支中，会利用 C++17 部分提到的辅助类 `init` 再额外添加一个参数，并进行递归调用。当形参包长度刚好超过了能够构造的长度时，返回值则恰好是能够参与构造的最大长度，即 `Tp` 的元素个数。

我们通过一个例子来说明，并给出详细的运行步骤。

```cpp
struct X
{
    int a{}, b{};
};

int main()
{
    std::cout << rm::size<X>() << std::endl; // 输出 2
}
```

**运行步骤**

1. 进入 `size` 函数，`Tp` 是 `X` 类型，形参包 `args` 为空；
2. `constexpr-if` 语句中，条件表达式等价于 `!requires { Tp{}; }`，符合语法，`requires` 表达式返回 `true`，经 `!` 修饰后，则会进入 `else` 分支；
3. 进入 `else`，此时返回值相当于 `size<Tp>(init{});`；
4. **第 2 次** 进入 `size` 函数，此时形参包有 \f$1\f$ 个参数 `init{}`；
5. `constexpr-if` 语句中，条件表达式等价于 `!requires { Tp{init{}}; }`，符合语法，同步骤 2，进入 `else` 分支；
6. 进入 `else`，此时返回值相当于 `size<Tp>(init{}, init{});`；
7. **第 3 次** 进入 `size` 函数，此时形参包有 \f$2\f$ 个参数 `init{}`；
8. `constexpr-if` 语句中，条件表达式等价于 `!requires { Tp{init{}, init{}}; }`，符合语法，同步骤 2，进入 `else` 分支；
9. 进入 `else`，此时返回值相当于 `size<Tp>(init{}, init{}, init{});`；
10. **第 4 次** 进入 `size` 函数，此时形参包有 \f$3\f$ 个参数 `init{}`；
11. `constexpr-if` 语句中，条件表达式等价于 `!requires { Tp{init{}, init{}, init{}}; }`，不符合语法（`X` 只有两个成员），`requires` 表达式返回 `false`，经 `!` 修饰后，则会进入 `true` 分支；
12. 此时形参包长度为 \f$3\f$，返回 `形参包长度 - 1`，即返回 \f$2\f$。

@end_toggle

### 3. 其余聚合体反射工具

@todo 
- 此部分未实现
- 有关遍历成员，比较大小的功能文档

### 4. 聚合类对象作为散列表的键 (Key)

@todo
- 此部分未实现
- 为聚合类而定义的哈希生成函数对象 `hash_aggregate` 文档，及其 `traits` 技法文档
