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

namespace rm
{

void StateInfo::add(std::string_view type, double val) { _states[std::string(type)] = val; }

void StateInfo::add(std::string_view type, std::string_view str) { _states[std::string(type)] = std::string(str); }

bool StateInfo::remove(std::string_view key) { return _states.erase(std::string(key)) > 0; }

bool StateInfo::contains(std::string_view key) const noexcept { return _states.find(std::string(key)) != _states.end(); }

void StateInfo::clear() noexcept { _states.clear(); }

bool StateInfo::empty() const noexcept { return _states.empty(); }

const StateType &StateInfo::at(std::string_view key) const { return _states.at(std::string(key)); }

StateType &StateInfo::at(std::string_view key) { return _states.at(std::string(key)); }

double StateInfo::at_numeric(std::string_view key) const { return std::get<double>(_states.at(std::string(key))); }

const std::string &StateInfo::at_string(std::string_view key) const { return std::get<std::string>(_states.at(std::string(key))); }

StateType &StateInfo::operator[](std::string_view key) noexcept { return _states[std::string(key)]; }

} // namespace rm
