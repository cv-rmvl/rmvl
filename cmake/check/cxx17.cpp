#if __cplusplus >= 201703L || (defined(_MSC_VER) && _MSC_VER >= 1910)
// OK
#else
#error "C++17 is not supported"
#endif

inline int test = 0;

int main()
{
    auto res = test;
    return res;
}
