/**
 * @file test_fmt.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief fmt 测试
 * @version 1.0
 * @date 2025-11-19
 * 
 * @copyright Copyright 2025 (c), zhaoxi
 * 
 */

#include "fmt/format.h"

int main() {
    std::string name = "World";
    int year = 2025;
    auto res = fmt::format("Hello, {}! Welcome to the year {}.", name, year);
    if (res == "Hello, World! Welcome to the year 2025.")
        return 0; // 成功
    else
        return 1; // 失败
}