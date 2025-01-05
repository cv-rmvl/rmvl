/**
 * @file types.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2025-01-03
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include "rmvl/types.hpp"
#include "rmvl/core/str.hpp"

namespace rm
{

void StateInfo::add(std::string_view type)
{
    auto types = str::split(type, ",");
    for (const auto &val : types)
    {
        auto type = str::strip(val);
        auto pos = type.find(':');
        if (pos == std::string::npos)
            _states.insert_or_assign(std::string(type), "");
        else
        {
            auto key = str::strip(type.substr(0, pos));
            auto value = str::strip(type.substr(pos + 1));
            _states.insert_or_assign(std::string(key), std::string(value));
        }
    }
}

bool StateInfo::remove(std::string_view key) { return _states.erase(std::string(key)) > 0; }

bool StateInfo::contains(std::string_view key) const noexcept { return _states.find(std::string(key)) != _states.end(); }

void StateInfo::clear() noexcept { _states.clear(); }

bool StateInfo::empty() const noexcept { return _states.empty(); }

const std::string &StateInfo::at(std::string_view key) const { return _states.at(std::string(key)); }

std::string &StateInfo::at(std::string_view key) { return _states.at(std::string(key)); }

std::string &StateInfo::operator[](std::string_view key) noexcept { return _states[std::string(key)]; }

} // namespace rm
