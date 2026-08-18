/**
 * @file test_rapidyaml.cpp
 * @brief rapidyaml 基础解析与序列化测试
 */

#include <string>

#include "ryml.hpp"
#include "ryml_std.hpp"

int main() {
    std::string yaml = "camera:\n  width: 1280\n  enabled: true\n";
    auto tree = ryml::parse_in_place(ryml::to_substr(yaml));

    int width{};
    bool enabled{};
    tree["camera"]["width"].load(&width);
    tree["camera"]["enabled"].load(&enabled);
    if (width != 1280 || !enabled)
        return 1;

    const auto output = ryml::emitrs_yaml<std::string>(tree);
    return output.empty() ? 1 : 0;
}
