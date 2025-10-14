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
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <open62541/nodeids.h>
#include <open62541/types_generated_handling.h>

#include "rmvl/core/rmvldef.hpp"

struct UA_Server;
struct UA_Client;

namespace rm {

#define OPCUA_VERSION UA_OPEN62541_VER_MAJOR * 10000 +   \
                          UA_OPEN62541_VER_MINOR * 100 + \
                          UA_OPEN62541_VER_PATCH

static_assert(OPCUA_VERSION >= 10400, "The version of open62541 must be greater than or equal to 1.4.0");

//! @addtogroup opcua
//! @{

//! OPC UA 节点 ID
class RMVL_EXPORTS_W NodeId final {
public:
    //! 默认构造节点 ID
    RMVL_W NodeId() = default;

    /**
     * @brief 构造 OPC UA 节点 ID
     *
     * @param[in] ns_ 命名空间
     * @param[in] id_ 节点号
     */
    RMVL_W constexpr NodeId(uint16_t ns_, uint32_t id_) : ns{ns_}, id{id_} {}

    /**
     * @brief 构造 OPC UA 节点 ID
     *
     * @param[in] nd 使用 `UA_NodeId` 表示的节点 ID
     */
    NodeId(const UA_NodeId &nd) : ns{nd.namespaceIndex}, id{nd.identifier.numeric} {}

    NodeId(const NodeId &nd) : id(nd.id) {}
    NodeId(NodeId &&nd) noexcept : id(nd.id) {}

    inline void operator=(const NodeId &nd) { ns = nd.ns, id = nd.id; }
    inline void operator=(NodeId &&nd) noexcept { ns = nd.ns, id = nd.id; }

    RMVL_W inline bool operator==(const NodeId &nd) const { return ns == nd.ns && id == nd.id; }
    RMVL_W inline bool operator!=(const NodeId &nd) const { return !(*this == nd); }

    //! 到 `UA_NodeId` 的转换
    constexpr operator UA_NodeId() const { return UA_NodeId{ns, UA_NODEIDTYPE_NUMERIC, {id}}; }

    //! 获取节点 ID
    inline UA_NodeId data() const { return UA_NodeId{ns, UA_NODEIDTYPE_NUMERIC, {id}}; }

    /**
     * @brief 判断节点 ID 是否为空
     *
     * @return 是否为空
     */
    RMVL_W inline bool empty() const { return ns == 0 && id == 0; }

    //! 清空节点 ID
    RMVL_W inline void clear() { ns = 0, id = 0; }

    RMVL_W_RW uint16_t ns{}; //!< 命名空间
    RMVL_W_RW uint32_t id{}; //!< 节点号
};

//! OPC UA 数据类型
class RMVL_EXPORTS_W DataType {
public:
    RMVL_W DataType() = default;

    /**
     * @brief 从 `UA_TYPES_<xxx>` 枚举值构造数据类型
     *
     * @param[in] id `UA_TYPES_<xxx>` 枚举值
     */
    RMVL_W constexpr DataType(uint32_t id) : _id(id) {}

    /**
     * @brief 从 `std::type_info` 构造数据类型
     *
     * @param[in] tp `std::type_info` 类型，可用 `typeid()` 获取
     */
    DataType(const std::type_info &tp) : _id(_map.at(std::type_index(tp))) {}

    operator uint32_t() const { return _id; }

private:
    //! 形如 `UA_TYPES_<xxx>` 的类型标志位
    static const std::unordered_map<std::type_index, uint32_t> _map;
    //! 数据类型 ID
    uint32_t _id{};
};

/**
 * @brief 用户信息
 * @brief
 * - 包含用户名与密码
 * @brief
 * - 用于给 OPC UA 服务器、客户端注册和配置用户信息
 */
struct RMVL_EXPORTS_W_AG UserConfig final {
    RMVL_W_RW std::string id;     //!< 用户名
    RMVL_W_RW std::string passwd; //!< 密码
};

//! 传输协议
enum class TransportID : uint8_t {
    UDP_UADP = 1U,  //!< 使用 `UDP` 传输协议映射和 `UADP` 消息映射的组合，此协议用于 **无代理** 的消息传递
    MQTT_UADP = 2U, //!< 使用 `MQTT` 传输协议映射和 `UADP` 消息映射的组合，此协议用于 **基于代理** 的消息传递
    MQTT_JSON = 3U, //!< 使用 `MQTT` 传输协议映射和 `JSON` 消息映射的组合，此协议用于 **基于代理** 的消息传递
};

/////////////////////////// 数据类型 ///////////////////////////
RMVL_W_SUBST("DT_RW")
constexpr DataType tpBoolean{UA_TYPES_BOOLEAN}; //!< 数据类型：`Boolean`
constexpr DataType tpSbyte{UA_TYPES_SBYTE};     //!< 数据类型：`Sbyte`
constexpr DataType tpByte{UA_TYPES_BYTE};       //!< 数据类型：`Byte`
constexpr DataType tpInt16{UA_TYPES_INT16};     //!< 数据类型：`Int16`
constexpr DataType tpUInt16{UA_TYPES_UINT16};   //!< 数据类型：`UInt16`
constexpr DataType tpInt32{UA_TYPES_INT32};     //!< 数据类型：`Int32`
constexpr DataType tpUInt32{UA_TYPES_UINT32};   //!< 数据类型：`UInt32`
constexpr DataType tpInt64{UA_TYPES_INT64};     //!< 数据类型：`Int64`
constexpr DataType tpUInt64{UA_TYPES_UINT64};   //!< 数据类型：`UInt64`
constexpr DataType tpFloat{UA_TYPES_FLOAT};     //!< 数据类型：`Float`
constexpr DataType tpDouble{UA_TYPES_DOUBLE};   //!< 数据类型：`Double`
constexpr DataType tpString{UA_TYPES_STRING};   //!< 数据类型：`String`

///////////////// 常用的 `0` 命名空间的节点 ID /////////////////

//////// DataType NodeId ////////
RMVL_W_RW constexpr NodeId nodeBoolean{0, UA_NS0ID_BOOLEAN};   //!< 数据类型节点：`Boolean` 节点 ID
RMVL_W_RW constexpr NodeId nodeSbyte{0, UA_NS0ID_SBYTE};       //!< 数据类型节点：`Sbyte` 节点 ID
RMVL_W_RW constexpr NodeId nodeByte{0, UA_NS0ID_BYTE};         //!< 数据类型节点：`Byte` 节点 ID
RMVL_W_RW constexpr NodeId nodeInt16{0, UA_NS0ID_INT16};       //!< 数据类型节点：`Int16` 节点 ID
RMVL_W_RW constexpr NodeId nodeUInt16{0, UA_NS0ID_UINT16};     //!< 数据类型节点：`UInt16` 节点 ID
RMVL_W_RW constexpr NodeId nodeInt32{0, UA_NS0ID_INT32};       //!< 数据类型节点：`Int32` 节点 ID
RMVL_W_RW constexpr NodeId nodeUInt32{0, UA_NS0ID_UINT32};     //!< 数据类型节点：`UInt32` 节点 ID
RMVL_W_RW constexpr NodeId nodeInt64{0, UA_NS0ID_INT64};       //!< 数据类型节点：`Int64` 节点 ID
RMVL_W_RW constexpr NodeId nodeUInt64{0, UA_NS0ID_UINT64};     //!< 数据类型节点：`UInt64` 节点 ID
RMVL_W_RW constexpr NodeId nodeFloat{0, UA_NS0ID_FLOAT};       //!< 数据类型节点：`Float` 节点 ID
RMVL_W_RW constexpr NodeId nodeDouble{0, UA_NS0ID_DOUBLE};     //!< 数据类型节点：`Double` 节点 ID
RMVL_W_RW constexpr NodeId nodeString{0, UA_NS0ID_STRING};     //!< 数据类型节点：`String` 节点 ID
RMVL_W_RW constexpr NodeId nodeDatetime{0, UA_NS0ID_DATETIME}; //!< 数据类型节点：`Datetime` 节点 ID

///////// Object NodeId /////////
RMVL_W_RW constexpr NodeId nodeObjectsFolder{0, UA_NS0ID_OBJECTSFOLDER};             //!< 对象节点：`ObjectsFolder` 节点 ID
RMVL_W_RW constexpr NodeId nodeTypesFolder{0, UA_NS0ID_TYPESFOLDER};                 //!< 对象节点：`TypesFolder` 节点 ID
RMVL_W_RW constexpr NodeId nodeViewsFolder{0, UA_NS0ID_VIEWSFOLDER};                 //!< 对象节点：`ViewsFolder` 节点 ID
RMVL_W_RW constexpr NodeId nodeObjectTypesFolder{0, UA_NS0ID_OBJECTTYPESFOLDER};     //!< 对象节点：`ObjectTypesFolder` 节点 ID
RMVL_W_RW constexpr NodeId nodeVariableTypesFolder{0, UA_NS0ID_VARIABLETYPESFOLDER}; //!< 对象节点：`VariableTypesFolder` 节点 ID
RMVL_W_RW constexpr NodeId nodeServer{0, UA_NS0ID_SERVER};                           //!< 对象节点：`OpcuaServer` 节点 ID

/////// ObjectType NodeId ///////
RMVL_W_RW constexpr NodeId nodeFolderType{0, UA_NS0ID_FOLDERTYPE};         //!< 对象类型节点：`FolderType` 节点 ID
RMVL_W_RW constexpr NodeId nodeBaseObjectType{0, UA_NS0ID_BASEOBJECTTYPE}; //!< 对象类型节点：`BaseObjectType` 节点 ID
RMVL_W_RW constexpr NodeId nodeBaseEventType{0, UA_NS0ID_BASEEVENTTYPE};   //!< 对象类型节点：`BaseEventType` 节点 ID

////// VariableType NodeId //////
RMVL_W_RW constexpr NodeId nodeBaseDataVariableType{0, UA_NS0ID_BASEDATAVARIABLETYPE}; //!< 变量类型节点：`BaseDataVariableType` 节点 ID
RMVL_W_RW constexpr NodeId nodePropertyType{0, UA_NS0ID_PROPERTYTYPE};                 //!< 变量类型节点：`PropertyType` 节点 ID

///// ReferenceType NodeId //////
RMVL_W_RW constexpr NodeId nodeOrganizes{0, UA_NS0ID_ORGANIZES};                 //!< 引用类型节点：`Organizes` 节点 ID
RMVL_W_RW constexpr NodeId nodeHasTypeDefinition{0, UA_NS0ID_HASTYPEDEFINITION}; //!< 引用类型节点：`HasTypeDefinition` 节点 ID
RMVL_W_RW constexpr NodeId nodeHasComponent{0, UA_NS0ID_HASCOMPONENT};           //!< 引用类型节点：`HasComponent` 节点 ID
RMVL_W_RW constexpr NodeId nodeHasProperty{0, UA_NS0ID_HASPROPERTY};             //!< 引用类型节点：`HasProperty` 节点 ID
RMVL_W_RW constexpr NodeId nodeHasSubtype{0, UA_NS0ID_HASSUBTYPE};               //!< 引用类型节点：`HasSubtype` 节点 ID
RMVL_W_RW constexpr NodeId nodeHasModellingRule{0, UA_NS0ID_HASMODELLINGRULE};   //!< 引用类型节点：`HasModellingRule` 节点 ID

//! 目标节点信息（服务端指针、浏览名、命名空间索引）
using FindNodeInServer = std::tuple<UA_Server *, std::string_view, uint16_t>;
//! 目标节点信息（客户端指针、浏览名、命名空间索引）
using FindNodeInClient = std::tuple<UA_Client *, std::string_view, uint16_t>;

//! @} opcua

namespace helper {

//! 获取编译期常量 `zh-CN`
inline constexpr char *zh_CN() { return const_cast<char *>("zh-CN"); }
//! 获取编译期常量 `en-US`
inline constexpr char *en_US() { return const_cast<char *>("en-US"); }
//! 转为 `char *`
inline char *to_char(std::string_view str) { return const_cast<char *>(str.data()); }

} // namespace helper

//! @addtogroup opcua
//! @{

/**
 * @brief 服务端路径搜索
 *
 * @param[in] origin 起始 NodeId
 * @param[in] fnis 目标节点信息（服务端指针、命名空间、浏览名）
 * @return 目标 NodeId
 */
NodeId operator|(NodeId origin, rm::FindNodeInServer &&fnis);

/**
 * @brief 客户端路径搜索
 *
 * @param[in] origin 起始 NodeId
 * @param[in] fnic 目标节点信息（客户端指针、命名空间、浏览名）
 * @return 目标 NodeId
 */
NodeId operator|(NodeId origin, rm::FindNodeInClient &&fnic);

//! @} opcua

} // namespace rm
