#if __cplusplus >= 202303L || (defined(_MSC_VER) && _MSC_VER >= 1930)
// OK
#else
#error "C++23 is not supported"
#endif

// 显式对象成员函数
struct S
{
    int a{};

    void f(this S self, int i)
    {
        self.a = i;
    }
};

// 打印库
#include <print>

int main()
{
    std::print("{}, {}!\n", "Hello", "world");
    return 0;
}
