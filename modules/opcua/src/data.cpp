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

#include "rmvl/opcua/event.hpp"
#include "rmvl/opcua/object.hpp"

namespace rm
{

VariableType &VariableType::operator=(const VariableType &val)
{
    browse_name = val.browse_name;
    display_name = val.display_name;
    description = val.description;
    _value = val._value;
    _data_type = val._data_type;
    _dims = val._dims;
    return *this;
}

VariableType &VariableType::operator=(VariableType &&val)
{
    browse_name = std::move(val.browse_name);
    display_name = std::move(val.display_name);
    description = std::move(val.description);
    _value = std::move(val._value);
    _data_type = std::exchange(val._data_type, 0);
    _dims = std::exchange(val._dims, 0);
    return *this;
}

Variable &Variable::operator=(const Variable &val)
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

Variable &Variable::operator=(Variable &&val)
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

Argument &Argument::operator=(const Argument &val)
{
    name = val.name;
    data_type = val.data_type;
    dims = val.dims;
    return *this;
}

Argument &Argument::operator=(Argument &&val)
{
    name = std::move(val.name);
    data_type = std::exchange(val.data_type, 0);
    dims = std::exchange(val.dims, 0);
    return *this;
}

Method &Method::operator=(const Method &val)
{
    browse_name = val.browse_name;
    display_name = val.display_name;
    description = val.description;
    func = val.func;
    iargs = val.iargs;
    oargs = val.oargs;
    return *this;
}

Method &Method::operator=(Method &&val)
{
    browse_name = std::move(val.browse_name);
    display_name = std::move(val.display_name);
    description = std::move(val.description);
    func = std::exchange(val.func, nullptr);
    iargs = std::move(val.iargs);
    oargs = std::move(val.oargs);
    return *this;
}

ObjectType &ObjectType::operator=(const ObjectType &val)
{
    browse_name = val.browse_name;
    display_name = val.display_name;
    description = val.description;
    _base = val._base;
    _variables = val._variables;
    _methods = val._methods;
    return *this;
}

ObjectType &ObjectType::operator=(ObjectType &&val)
{
    browse_name = std::move(val.browse_name);
    display_name = std::move(val.display_name);
    description = std::move(val.description);
    _base = std::exchange(val._base, nullptr);
    _variables = std::move(val._variables);
    _methods = std::move(val._methods);
    return *this;
}

EventType &EventType::operator=(const EventType &val)
{
    browse_name = val.browse_name;
    display_name = val.display_name;
    description = val.description;
    _properties = val._properties;
    return *this;
}

EventType &EventType::operator=(EventType &&val)
{
    browse_name = std::move(val.browse_name);
    display_name = std::move(val.display_name);
    description = std::move(val.description);
    _properties = std::move(val._properties);
    return *this;
}

Event &Event::operator=(const Event &val)
{
    _type = val._type;
    _properties = val._properties;
    return *this;
}

Event &Event::operator=(Event &&val)
{
    _type = std::exchange(val._type, nullptr);
    _properties = std::move(val._properties);
    return *this;
}

} // namespace rm
