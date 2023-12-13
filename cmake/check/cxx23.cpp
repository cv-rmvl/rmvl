#if __cplusplus >= 202303L || (defined(_MSC_VER) && _MSC_VER >= 1930)
// OK
#else
#error "C++23 is not supported"
#endif

#include <print>

int main()
{
    std::print("{}, {}!\n", "Hello", "world");
    return 0;
}
