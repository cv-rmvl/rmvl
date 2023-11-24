/**
 * @file utilities.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2023-10-22
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <string>
#include <typeindex>
#include <unordered_map>

#include <open62541/types_generated.h>

struct UA_Server;
struct UA_Client;

namespace rm
{

#define OPCUA_VERSION UA_OPEN62541_VER_MAJOR * 10000 +   \
                          UA_OPEN62541_VER_MINOR * 100 + \
                          UA_OPEN62541_VER_PATCH

//! @addtogroup opcua
//! @{

/**
 * @brief 用户信息
 * @brief
 * - 包含用户名与密码
 * @brief
 * - 用于给 OPC UA 服务器、客户端注册和配置用户信息
 */
struct UserConfig final
{
    ::std::string id;     //!< 用户名
    ::std::string passwd; //!< 密码
};

//! 类型标志位，可通过 `typeflag[xxx]` 进行获取
using UA_TypeFlag = UA_UInt32;

//! 获取形如 `UA_TYPES_<xxx>` 的类型标志位
inline ::std::unordered_map<::std::type_index, UA_TypeFlag> typeflag =
    {{::std::type_index(typeid(bool)), UA_TYPES_BOOLEAN},
     {::std::type_index(typeid(int8_t)), UA_TYPES_SBYTE},
     {::std::type_index(typeid(uint8_t)), UA_TYPES_BYTE},
     {::std::type_index(typeid(UA_Int16)), UA_TYPES_INT16},
     {::std::type_index(typeid(UA_UInt16)), UA_TYPES_UINT16},
     {::std::type_index(typeid(UA_Int32)), UA_TYPES_INT32},
     {::std::type_index(typeid(UA_UInt32)), UA_TYPES_UINT32},
     {::std::type_index(typeid(UA_Int64)), UA_TYPES_INT64},
     {::std::type_index(typeid(UA_UInt64)), UA_TYPES_UINT64},
     {::std::type_index(typeid(UA_Float)), UA_TYPES_FLOAT},
     {::std::type_index(typeid(UA_Double)), UA_TYPES_DOUBLE},
     {::std::type_index(typeid(const char *)), UA_TYPES_STRING}};

///////////////// 常用的 `0` 命名空间的节点 ID /////////////////

//////// Object NodeId ////////
constexpr UA_NodeId nodeObjectsFolder{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_OBJECTSFOLDER};             //!< 对象节点：`ObjectsFolder` 节点 ID
constexpr UA_NodeId nodeTypesFolder{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_TYPESFOLDER};                 //!< 对象节点：`TypesFolder` 节点 ID
constexpr UA_NodeId nodeViewsFolder{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_VIEWSFOLDER};                 //!< 对象节点：`ViewsFolder` 节点 ID
constexpr UA_NodeId nodeObjectTypesFolder{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_OBJECTTYPESFOLDER};     //!< 对象节点：`ObjectTypesFolder` 节点 ID
constexpr UA_NodeId nodeVariableTypesFolder{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_VARIABLETYPESFOLDER}; //!< 对象节点：`VariableTypesFolder` 节点 ID
constexpr UA_NodeId nodeServer{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_SERVER};                           //!< 对象节点：`Server` 节点 ID

//////// ObjectType NodeId ////////
constexpr UA_NodeId nodeFolderType{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_FOLDERTYPE};         //!< 对象类型节点：`FolderType` 节点 ID
constexpr UA_NodeId nodeBaseObjectType{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_BASEOBJECTTYPE}; //!< 对象类型节点：`BaseObjectType` 节点 ID
constexpr UA_NodeId nodeBaseEventType{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_BASEEVENTTYPE};   //!< 对象类型节点：`BaseEventType` 节点 ID

//////// VariableType NodeId ////////
constexpr UA_NodeId nodeBaseDataVariableType{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_BASEDATAVARIABLETYPE}; //!< 变量类型节点：`BaseDataVariableType` 节点 ID
constexpr UA_NodeId nodePropertyType{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_PROPERTYTYPE};                 //!< 变量类型节点：`PropertyType` 节点 ID

//////// ReferenceType NodeId ////////
constexpr UA_NodeId nodeOrganizes{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_ORGANIZES};                 //!< 引用类型节点：`Organizes` 节点 ID
constexpr UA_NodeId nodeHasTypeDefinition{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_HASTYPEDEFINITION}; //!< 引用类型节点：`HasTypeDefinition` 节点 ID
constexpr UA_NodeId nodeHasComponent{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_HASCOMPONENT};           //!< 引用类型节点：`HasComponent` 节点 ID
constexpr UA_NodeId nodeHasProperty{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_HASPROPERTY};             //!< 引用类型节点：`HasProperty` 节点 ID
constexpr UA_NodeId nodeHasSubtype{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_HASSUBTYPE};               //!< 引用类型节点：`HasSubtype` 节点 ID
constexpr UA_NodeId nodeHasModellingRule{0, UA_NODEIDTYPE_NUMERIC, UA_NS0ID_HASMODELLINGRULE};   //!< 引用类型节点：`HasModellingRule` 节点 ID

//! 目标节点信息（服务端指针、浏览名）
using FindNodeInServer = ::std::tuple<UA_Server *, ::std::string>;
//! 目标节点信息（客户端指针、浏览名）
using FindNodeInClient = ::std::tuple<UA_Client *, ::std::string>;

//! @} opcua

namespace helper
{

//! 获取编译期常量 `zh-CN`
inline constexpr char *zh_CN() { return const_cast<char *>("zh-CN"); }
//! 获取编译期常量 `en-US`
inline constexpr char *en_US() { return const_cast<char *>("en-US"); }
//! `std::string` 转为 `char *`
inline char *to_char(const std::string &str) { return const_cast<char *>(str.c_str()); }

} // namespace helper

} // namespace rm

//! @addtogroup opcua
//! @{

/**
 * @brief 服务端路径搜索
 *
 * @param[in] origin 起始 NodeId
 * @param[in] fnis 目标节点信息（服务端指针、命名空间、浏览名）
 * @return 目标 NodeId
 */
UA_NodeId operator|(UA_NodeId origin, rm::FindNodeInServer &&fnis);

/**
 * @brief 客户端路径搜索
 *
 * @param[in] origin 起始 NodeId
 * @param[in] fnic 目标节点信息（客户端指针、命名空间、浏览名）
 * @return 目标 NodeId
 */
UA_NodeId operator|(UA_NodeId origin, rm::FindNodeInClient &&fnic);

//! @} opcua
