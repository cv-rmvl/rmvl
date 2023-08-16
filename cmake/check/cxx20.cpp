#if __cplusplus >= 202002L || (defined(_MSC_VER) && _MSC_VER >= 1920)
// OK
#else
#error "C++20 is not supported"
#endif

#include <ranges>
#include <vector>

namespace stdv = std::views;

int main()
{
    std::vector arr = {1, 2, 3, 4};
    auto even = [](int i)
    {
        return i % 2 == 0;
    };
    for (auto v : arr | stdv::filter(even))
        ;
    return 0;
}