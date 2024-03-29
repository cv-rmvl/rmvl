/**
 * @file data.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 数据，包括变量、方法、对象、事件、视图
 * @version 1.1
 * @date 2024-03-29
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include "rmvl/opcua/event.hpp"
#include "rmvl/opcua/object.hpp"
#include "rmvl/opcua/view.hpp"

namespace rm
{

VariableType &VariableType::operator=(const VariableType &val)
{
    browse_name = val.browse_name;
    display_name = val.display_name;
    description = val.description;
    _value = val._value;
    _data_type = val._data_type;
    _size = val._size;
    return *this;
}

VariableType &VariableType::operator=(VariableType &&val)
{
    browse_name = std::move(val.browse_name);
    display_name = std::move(val.display_name);
    description = std::move(val.description);
    _value = std::move(val._value);
    _data_type = std::exchange(val._data_type, 0);
    _size = std::exchange(val._size, 0);
    return *this;
}

Variable &Variable::operator=(const Variable &val)
{
    browse_name = val.browse_name;
    display_name = val.display_name;
    description = val.description;
    access_level = val.access_level;
    _type = val._type;
    _value = val._value;
    _data_type = val._data_type;
    _size = val._size;
    return *this;
}

Variable &Variable::operator=(Variable &&val)
{
    browse_name = std::move(val.browse_name);
    display_name = std::move(val.display_name);
    description = std::move(val.description);
    access_level = std::exchange(val.access_level, 0);
    _type = std::exchange(val._type, nullptr);
    _value = std::move(val._value);
    _data_type = std::exchange(val._data_type, 0);
    _size = std::exchange(val._size, 0);
    return *this;
}

bool Variable::operator==(const Variable &val) const
{
    if (_data_type != val._data_type)
        return false;
    if (_size != val._size)
        return false;
    if (_size == 1)
    {
        switch (_data_type)
        {
        case UA_TYPES_BOOLEAN:
            return std::any_cast<bool>(_value) == std::any_cast<bool>(val._value);
        case UA_TYPES_SBYTE:
            return std::any_cast<int8_t>(_value) == std::any_cast<int8_t>(val._value);
        case UA_TYPES_BYTE:
            return std::any_cast<uint8_t>(_value) == std::any_cast<uint8_t>(val._value);
        case UA_TYPES_INT16:
            return std::any_cast<int16_t>(_value) == std::any_cast<int16_t>(val._value);
        case UA_TYPES_UINT16:
            return std::any_cast<uint16_t>(_value) == std::any_cast<uint16_t>(val._value);
        case UA_TYPES_INT32:
            return std::any_cast<int32_t>(_value) == std::any_cast<int32_t>(val._value);
        case UA_TYPES_UINT32:
            return std::any_cast<uint32_t>(_value) == std::any_cast<uint32_t>(val._value);
        case UA_TYPES_INT64:
            return std::any_cast<int64_t>(_value) == std::any_cast<int64_t>(val._value);
        case UA_TYPES_UINT64:
            return std::any_cast<uint64_t>(_value) == std::any_cast<uint64_t>(val._value);
        case UA_TYPES_FLOAT:
            return std::any_cast<float>(_value) == std::any_cast<float>(val._value);
        case UA_TYPES_DOUBLE:
            return std::any_cast<double>(_value) == std::any_cast<double>(val._value);
        case UA_TYPES_STRING:
            return std::any_cast<const char *>(_value) == std::any_cast<const char *>(val._value);
        default:
            return false;
        }
    }
    else
    {
        switch (_data_type)
        {
        case UA_TYPES_SBYTE:
            return std::any_cast<std::vector<int8_t>>(_value) == std::any_cast<std::vector<int8_t>>(val._value);
        case UA_TYPES_BYTE:
            return std::any_cast<std::vector<uint8_t>>(_value) == std::any_cast<std::vector<uint8_t>>(val._value);
        case UA_TYPES_INT16:
            return std::any_cast<std::vector<int16_t>>(_value) == std::any_cast<std::vector<int16_t>>(val._value);
        case UA_TYPES_UINT16:
            return std::any_cast<std::vector<uint16_t>>(_value) == std::any_cast<std::vector<uint16_t>>(val._value);
        case UA_TYPES_INT32:
            return std::any_cast<std::vector<int32_t>>(_value) == std::any_cast<std::vector<int32_t>>(val._value);
        case UA_TYPES_UINT32:
            return std::any_cast<std::vector<uint32_t>>(_value) == std::any_cast<std::vector<uint32_t>>(val._value);
        case UA_TYPES_INT64:
            return std::any_cast<std::vector<int64_t>>(_value) == std::any_cast<std::vector<int64_t>>(val._value);
        case UA_TYPES_UINT64:
            return std::any_cast<std::vector<uint64_t>>(_value) == std::any_cast<std::vector<uint64_t>>(val._value);
        case UA_TYPES_FLOAT:
            return std::any_cast<std::vector<float>>(_value) == std::any_cast<std::vector<float>>(val._value);
        case UA_TYPES_DOUBLE:
            return std::any_cast<std::vector<double>>(_value) == std::any_cast<std::vector<double>>(val._value);
        default:
            return false;
        }
    }
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

View &View::operator=(const View &val)
{
    browse_name = val.browse_name;
    display_name = val.display_name;
    description = val.description;
    _nodes = val._nodes;
    return *this;
}

View &View::operator=(View &&val)
{
    browse_name = std::move(val.browse_name);
    display_name = std::move(val.display_name);
    description = std::move(val.description);
    _nodes = std::move(val._nodes);
    return *this;
}

} // namespace rm
