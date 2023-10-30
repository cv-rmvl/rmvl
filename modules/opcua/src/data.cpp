/**
 * @file data.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 对象（类型）
 * @version 1.0
 * @date 2023-10-23
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include "rmvl/opcua/object.hpp"

rm::VariableType &rm::VariableType::operator=(const rm::VariableType &val)
{
    browse_name = val.browse_name;
    display_name = val.display_name;
    description = val.description;
    _value = val._value;
    _data_type = val._data_type;
    _dims = val._dims;
    return *this;
}

rm::VariableType &rm::VariableType::operator=(rm::VariableType &&val)
{
    browse_name = std::move(val.browse_name);
    display_name = std::move(val.display_name);
    description = std::move(val.description);
    _value = std::move(val._value);
    _data_type = std::exchange(val._data_type, 0);
    _dims = std::exchange(val._dims, 0);
    return *this;
}

rm::Variable &rm::Variable::operator=(const rm::Variable &val)
{
    browse_name = val.browse_name;
    display_name = val.display_name;
    description = val.description;
    _type = val._type;
    _value = val._value;
    _data_type = val._data_type;
    _dims = val._dims;
    _access_level = val._access_level;
    return *this;
}

rm::Variable &rm::Variable::operator=(rm::Variable &&val)
{
    browse_name = std::move(val.browse_name);
    display_name = std::move(val.display_name);
    description = std::move(val.description);
    _type = std::exchange(val._type, nullptr);
    _value = std::move(val._value);
    _data_type = std::exchange(val._data_type, 0);
    _dims = std::exchange(val._dims, 0);
    _access_level = std::exchange(val._access_level, 0);
    return *this;
}

rm::Argument &rm::Argument::operator=(const rm::Argument &val)
{
    name = val.name;
    data_type = val.data_type;
    dims = val.dims;
    return *this;
}

rm::Argument &rm::Argument::operator=(rm::Argument &&val)
{
    name = std::move(val.name);
    data_type = std::exchange(val.data_type, 0);
    dims = std::exchange(val.dims, 0);
    return *this;
}

rm::Method &rm::Method::operator=(const rm::Method &val)
{
    browse_name = val.browse_name;
    display_name = val.display_name;
    description = val.description;
    func = val.func;
    iargs = val.iargs;
    oargs = val.oargs;
    return *this;
}

rm::Method &rm::Method::operator=(rm::Method &&val)
{
    browse_name = std::move(val.browse_name);
    display_name = std::move(val.display_name);
    description = std::move(val.description);
    func = std::exchange(val.func, nullptr);
    iargs = std::move(val.iargs);
    oargs = std::move(val.oargs);
    return *this;
}

rm::ObjectType &rm::ObjectType::operator=(const rm::ObjectType &val)
{
    browse_name = val.browse_name;
    display_name = val.display_name;
    description = val.description;
    _base = val._base;
    _variables = val._variables;
    _methods = val._methods;
    return *this;
}

rm::ObjectType &rm::ObjectType::operator=(rm::ObjectType &&val)
{
    browse_name = std::move(val.browse_name);
    display_name = std::move(val.display_name);
    description = std::move(val.description);
    _base = std::exchange(val._base, nullptr);
    _variables = std::move(val._variables);
    _methods = std::move(val._methods);
    return *this;
}
