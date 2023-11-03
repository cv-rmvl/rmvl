/**
 * @file helper.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2023-10-23
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <open62541/client.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>

#include "rmvl/opcua/method.hpp"
#include "rmvl/opcua/variable.hpp"

UA_NodeId operator|(UA_NodeId origin, rm::FindNodeInServer &&fnis)
{
    if (!UA_NodeId_equal(&origin, &UA_NODEID_NULL))
        return origin;
    auto &&[p_server, browse_name] = fnis;
    auto qualified_name = UA_QUALIFIEDNAME(1, rm::helper::to_char(browse_name.c_str()));
    auto bpr = UA_Server_browseSimplifiedBrowsePath(p_server, origin, 1, &qualified_name);
    UA_NodeId retval = UA_NODEID_NULL;
    if (bpr.statusCode != UA_STATUSCODE_GOOD || bpr.targetsSize < 1)
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to find NodeId (name = %s): %s", browse_name.c_str(), UA_StatusCode_name(bpr.statusCode));
    else
        UA_NodeId_copy(&bpr.targets[0].targetId.nodeId, &retval);
    return retval;
}

UA_NodeId operator|(UA_NodeId origin, rm::findNodeInClient &&fnic)
{
    UA_BrowsePath browse_path;
    UA_BrowsePath_init(&browse_path);
    browse_path.startingNode = origin;
    auto elem = std::make_unique<UA_RelativePathElement>();
    browse_path.relativePath.elements = elem.get();
    browse_path.relativePath.elementsSize = 1;

    auto &&[p_client, browse_name] = fnic;
    elem->targetName = UA_QUALIFIEDNAME(1, rm::helper::to_char(browse_name));

    UA_TranslateBrowsePathsToNodeIdsRequest request;
    UA_TranslateBrowsePathsToNodeIdsRequest_init(&request);
    request.browsePaths = &browse_path;
    request.browsePathsSize = 1;

    auto response = UA_Client_Service_translateBrowsePathsToNodeIds(p_client, request);
    if (response.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
        if (response.resultsSize == 1 && response.results[0].targetsSize == 1)
            return response.results[0].targets[0].targetId.nodeId;

    UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "Failed to find NodeId (name = %s): %s",
                 browse_name.c_str(), UA_StatusCode_name(response.responseHeader.serviceResult));
    return UA_NODEID_NULL;
}

UA_Variant *rm::helper::cvtVariable(const rm::Variable &val)
{
    const std::any &data = val.getValue();

    UA_Variant *p_val = UA_Variant_new();
    if (val.getArrayDimensions() == 1)
    {
        switch (val.getDataType())
        {
        case UA_TYPES_STRING: {
            UA_String str = UA_STRING_ALLOC(std::any_cast<const char *>(data));
            UA_Variant_setScalarCopy(p_val, &str, &UA_TYPES[UA_TYPES_STRING]);
        }
        break;
        case UA_TYPES_BOOLEAN: {
            auto rawval = std::any_cast<UA_Boolean>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_BOOLEAN]);
        }
        break;
        case UA_TYPES_SBYTE: {
            auto rawval = std::any_cast<UA_SByte>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_SBYTE]);
        }
        break;
        case UA_TYPES_BYTE: {
            auto rawval = std::any_cast<UA_Byte>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_BYTE]);
        }
        break;
        case UA_TYPES_INT16: {
            auto rawval = std::any_cast<UA_Int16>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_INT16]);
        }
        break;
        case UA_TYPES_UINT16: {
            auto rawval = std::any_cast<UA_UInt16>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_UINT16]);
        }
        break;
        case UA_TYPES_INT32: {
            auto rawval = std::any_cast<UA_Int32>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_INT32]);
        }
        break;
        case UA_TYPES_UINT32: {
            auto rawval = std::any_cast<UA_UInt32>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_UINT32]);
        }
        break;
        case UA_TYPES_INT64: {
            auto rawval = std::any_cast<UA_Int64>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_INT64]);
        }
        break;
        case UA_TYPES_UINT64: {
            auto rawval = std::any_cast<UA_UInt64>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_UINT64]);
        }
        break;
        case UA_TYPES_FLOAT: {
            auto rawval = std::any_cast<UA_Float>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_FLOAT]);
        }
        break;
        case UA_TYPES_DOUBLE: {
            auto rawval = std::any_cast<UA_Double>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_DOUBLE]);
        }
        break;
        default:
            RMVL_Error(RMVL_StsBadArg, "Unknown UA_TypeFlag");
            break;
        }
        p_val->arrayDimensionsSize = 0;
        p_val->arrayDimensions = nullptr;
    }
    else
    {
        switch (val.getDataType())
        {
        case UA_TYPES_SBYTE: {
            auto rawval = std::any_cast<std::vector<UA_SByte>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_SBYTE]);
        }
        break;
        case UA_TYPES_BYTE: {
            auto rawval = std::any_cast<std::vector<UA_Byte>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_BYTE]);
        }
        break;
        case UA_TYPES_INT16: {
            auto rawval = std::any_cast<std::vector<UA_Int16>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_INT16]);
        }
        break;
        case UA_TYPES_UINT16: {
            auto rawval = std::any_cast<std::vector<UA_UInt16>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_UINT16]);
        }
        break;
        case UA_TYPES_INT32: {
            auto rawval = std::any_cast<std::vector<UA_Int32>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_INT32]);
        }
        break;
        case UA_TYPES_UINT32: {
            auto rawval = std::any_cast<std::vector<UA_UInt32>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_UINT32]);
        }
        break;
        case UA_TYPES_INT64: {
            auto rawval = std::any_cast<std::vector<UA_Int64>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_INT64]);
        }
        break;
        case UA_TYPES_UINT64: {
            auto rawval = std::any_cast<std::vector<UA_UInt64>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_UINT64]);
        }
        break;
        case UA_TYPES_FLOAT: {
            auto rawval = std::any_cast<std::vector<UA_Float>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_FLOAT]);
        }
        break;
        case UA_TYPES_DOUBLE: {
            auto rawval = std::any_cast<std::vector<UA_Double>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_DOUBLE]);
        }
        break;
        default:
            RMVL_Error_(RMVL_StsBadArg, "Unknown UA_TypeFlag: %zu", val.getDataType());
            break;
        }

        p_val->arrayDimensionsSize = 1;
        p_val->arrayDimensions = &const_cast<UA_UInt32 &>(val.getArrayDimensions());
    }
    return p_val;
}

rm::Variable rm::helper::cvtVariable(const UA_Variant *p_val)
{
    UA_UInt32 dims = p_val->arrayLength;
    UA_TypeFlag type_flag = p_val->type->typeKind;
    void *data = p_val->data;
    if (dims == 1)
    {
        switch (type_flag)
        {
        case UA_TYPES_STRING:
            return reinterpret_cast<const char *>(data);
        case UA_TYPES_BOOLEAN:
            return *reinterpret_cast<UA_Boolean *>(data);
        case UA_TYPES_SBYTE:
            return *reinterpret_cast<UA_SByte *>(data);
        case UA_TYPES_BYTE:
            return *reinterpret_cast<UA_Byte *>(data);
        case UA_TYPES_INT16:
            return *reinterpret_cast<UA_Int16 *>(data);
        case UA_TYPES_UINT16:
            return *reinterpret_cast<UA_UInt16 *>(data);
        case UA_TYPES_INT32:
            return *reinterpret_cast<UA_Int32 *>(data);
        case UA_TYPES_UINT32:
            return *reinterpret_cast<UA_UInt32 *>(data);
        case UA_TYPES_INT64:
            return *reinterpret_cast<UA_Int64 *>(data);
        case UA_TYPES_UINT64:
            return *reinterpret_cast<UA_UInt64 *>(data);
        case UA_TYPES_FLOAT:
            return *reinterpret_cast<UA_Float *>(data);
        case UA_TYPES_DOUBLE:
            return *reinterpret_cast<UA_Double *>(data);
        default:
            RMVL_Error_(RMVL_StsBadArg, "Unknown UA_TypeFlag: %zu", type_flag);
        }
    }
    else
    {
        switch (type_flag)
        {
        case UA_TYPES_SBYTE:
            return std::vector(reinterpret_cast<UA_SByte *>(data), reinterpret_cast<UA_SByte *>(data) + dims);
        case UA_TYPES_BYTE:
            return std::vector(reinterpret_cast<UA_Byte *>(data), reinterpret_cast<UA_Byte *>(data) + dims);
        case UA_TYPES_INT16:
            return std::vector(reinterpret_cast<UA_Int16 *>(data), reinterpret_cast<UA_Int16 *>(data) + dims);
        case UA_TYPES_UINT16:
            return std::vector(reinterpret_cast<UA_UInt16 *>(data), reinterpret_cast<UA_UInt16 *>(data) + dims);
        case UA_TYPES_INT32:
            return std::vector(reinterpret_cast<UA_Int32 *>(data), reinterpret_cast<UA_Int32 *>(data) + dims);
        case UA_TYPES_UINT32:
            return std::vector(reinterpret_cast<UA_UInt32 *>(data), reinterpret_cast<UA_UInt32 *>(data) + dims);
        case UA_TYPES_INT64:
            return std::vector(reinterpret_cast<UA_Int64 *>(data), reinterpret_cast<UA_Int64 *>(data) + dims);
        case UA_TYPES_UINT64:
            return std::vector(reinterpret_cast<UA_UInt64 *>(data), reinterpret_cast<UA_UInt64 *>(data) + dims);
        case UA_TYPES_FLOAT:
            return std::vector(reinterpret_cast<UA_Float *>(data), reinterpret_cast<UA_Float *>(data) + dims);
        case UA_TYPES_DOUBLE:
            return std::vector(reinterpret_cast<UA_Double *>(data), reinterpret_cast<UA_Double *>(data) + dims);
        default:
            RMVL_Error_(RMVL_StsBadArg, "Unknown UA_TypeFlag: %zu", type_flag);
        }
    }
    return {};
}

UA_Variant *rm::helper::cvtVariable(const rm::VariableType &vtype)
{
    const std::any &data = vtype.getValue();

    UA_Variant *p_val = UA_Variant_new();
    if (vtype.getArrayDimensions() == 1)
    {
        switch (vtype.getDataType())
        {
        case UA_TYPES_STRING: {
            UA_String str = UA_STRING(to_char(std::any_cast<const char *>(data)));
            UA_Variant_setScalarCopy(p_val, &str, &UA_TYPES[UA_TYPES_STRING]);
        }
        break;
        case UA_TYPES_BOOLEAN: {
            auto rawval = std::any_cast<UA_Boolean>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_BOOLEAN]);
        }
        break;
        case UA_TYPES_SBYTE: {
            auto rawval = std::any_cast<UA_SByte>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_SBYTE]);
        }
        break;
        case UA_TYPES_BYTE: {
            auto rawval = std::any_cast<UA_Byte>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_BYTE]);
        }
        break;
        case UA_TYPES_INT16: {
            auto rawval = std::any_cast<UA_Int16>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_INT16]);
        }
        break;
        case UA_TYPES_UINT16: {
            auto rawval = std::any_cast<UA_UInt16>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_UINT16]);
        }
        break;
        case UA_TYPES_INT32: {
            auto rawval = std::any_cast<UA_Int32>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_INT32]);
        }
        break;
        case UA_TYPES_UINT32: {
            auto rawval = std::any_cast<UA_UInt32>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_UINT32]);
        }
        break;
        case UA_TYPES_INT64: {
            auto rawval = std::any_cast<UA_Int64>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_INT64]);
        }
        break;
        case UA_TYPES_UINT64: {
            auto rawval = std::any_cast<UA_UInt64>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_UINT64]);
        }
        break;
        case UA_TYPES_FLOAT: {
            auto rawval = std::any_cast<UA_Float>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_FLOAT]);
        }
        break;
        case UA_TYPES_DOUBLE: {
            auto rawval = std::any_cast<UA_Double>(data);
            UA_Variant_setScalarCopy(p_val, &rawval, &UA_TYPES[UA_TYPES_DOUBLE]);
        }
        break;
        default:
            RMVL_Error(RMVL_StsBadArg, "Unknown UA_TypeFlag");
            break;
        }
        p_val->arrayDimensionsSize = 0;
        p_val->arrayDimensions = nullptr;
    }
    else
    {
        switch (vtype.getDataType())
        {
        case UA_TYPES_SBYTE: {
            auto rawval = std::any_cast<std::vector<UA_SByte>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_SBYTE]);
        }
        break;
        case UA_TYPES_BYTE: {
            auto rawval = std::any_cast<std::vector<UA_Byte>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_BYTE]);
        }
        break;
        case UA_TYPES_INT16: {
            auto rawval = std::any_cast<std::vector<UA_Int16>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_INT16]);
        }
        break;
        case UA_TYPES_UINT16: {
            auto rawval = std::any_cast<std::vector<UA_UInt16>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_UINT16]);
        }
        break;
        case UA_TYPES_INT32: {
            auto rawval = std::any_cast<std::vector<UA_Int32>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_INT32]);
        }
        break;
        case UA_TYPES_UINT32: {
            auto rawval = std::any_cast<std::vector<UA_UInt32>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_UINT32]);
        }
        break;
        case UA_TYPES_INT64: {
            auto rawval = std::any_cast<std::vector<UA_Int64>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_INT64]);
        }
        break;
        case UA_TYPES_UINT64: {
            auto rawval = std::any_cast<std::vector<UA_UInt64>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_UINT64]);
        }
        break;
        case UA_TYPES_FLOAT: {
            auto rawval = std::any_cast<std::vector<UA_Float>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_FLOAT]);
        }
        break;
        case UA_TYPES_DOUBLE: {
            auto rawval = std::any_cast<std::vector<UA_Double>>(data);
            UA_Variant_setArrayCopy(p_val, rawval.data(), rawval.size(), &UA_TYPES[UA_TYPES_DOUBLE]);
        }
        break;
        default:
            RMVL_Error_(RMVL_StsBadArg, "Unknown UA_TypeFlag: %zu", vtype.getDataType());
            break;
        }

        p_val->arrayDimensionsSize = 1;
        p_val->arrayDimensions = &const_cast<UA_UInt32 &>(vtype.getArrayDimensions());
    }
    return p_val;
}

UA_Argument *rm::helper::cvtArgument(const rm::Argument &arg)
{
    UA_Argument *argument = UA_Argument_new();
    argument->name = UA_STRING_ALLOC(arg.name.c_str());
    argument->description = UA_LOCALIZEDTEXT_ALLOC(helper::zh_CN(), arg.name.c_str());
    argument->dataType = UA_TYPES[arg.data_type].typeId;
    RMVL_Assert(arg.dims);
    if (arg.dims == 1)
    {
        argument->valueRank = UA_VALUERANK_SCALAR;
        argument->arrayDimensionsSize = 0;
        argument->arrayDimensions = nullptr;
    }
    else
    {
        argument->valueRank = 1;
        argument->arrayDimensionsSize = 1;
        argument->arrayDimensions = &const_cast<rm::Argument &>(arg).dims;
    }
    return argument;
}
