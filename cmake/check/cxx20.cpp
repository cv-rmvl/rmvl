#if __cplusplus >= 202002L || (defined(_MSC_VER) && _MSC_VER >= 1920)
// OK
#else
#error "C++20 is not supported"
#endif

#include <coroutine>
#include <ranges>
#include <vector>

namespace stdv = std::views;

// 位域的默认成员初始化器
struct S
{
    unsigned int a : 4 {0};
    unsigned int b : 4 {1};
};

// 嵌套内联命名空间
namespace A::inline B
{
};

// 概念
template <typename T>
concept HasSize = requires(T t) { t.size(); };

template <HasSize T>
inline std::size_t f(const T &t) { return t.size(); }

void test()
{
    std::vector<int> arr{1, 2, 3};
    f(arr);
}

// 三路运算符
void test2()
{
    int a = 1, b = 2;
    auto ret = a <=> b;
}

// 立即函数
consteval int add(int a, int b) { return a + b; }

// 协程
struct Task
{
    struct promise_type
    {
        Task get_return_object() { return {std::coroutine_handle<promise_type>::from_promise(*this)}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
    std::coroutine_handle<promise_type> h;
};

Task foo()
{
    co_await std::suspend_never{}; }

void test3()
{
    /* code 1 */
    auto t = foo();
    /* code 2 */
    t.h.resume();
}

// 范围
void test4()
{
    std::vector arr = {1, 2, 3, 4};
    auto even = [](int i) {
        return i % 2 == 0;
    };
    for (auto v : arr | stdv::filter(even))
        ;
}

int main()
{
    return 0;
}