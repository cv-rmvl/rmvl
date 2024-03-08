#if __cplusplus >= 201703L || (defined(_MSC_VER) && _MSC_VER >= 1910)
// OK
#else
#error "C++17 is not supported"
#endif

// 嵌套命名空间
namespace A::B::C
{
};

// u8 字符字面量
char c = u8'c';

// 内联变量
inline int test = 0;

// 结构化绑定
#include <utility>
auto [x, y] = std::pair(1, 2);

// constexpr if
template <typename T>
constexpr int f()
{
    if constexpr (sizeof(T) == 4)
        return 4;
    else
        return 0;
}

// 折叠表达式
template <typename... Args>
inline auto sum_add_1(Args... args) { return (args + ... + 1); }

// 库支持
#include <filesystem>
#include <optional>
#include <variant>
#include <any>

int main()
{
    return 0;
}
