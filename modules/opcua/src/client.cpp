/**
 * @file client.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 客户端
 * @version 1.0
 * @date 2023-10-29
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/plugin/log_stdout.h>

#include "rmvl/opcua/client.hpp"

rm::Client::Client(std::string_view address, rm::UserConfig usr)
{
    _client = UA_Client_new();
    UA_ClientConfig *config = UA_Client_getConfig(_client);
    auto status = UA_ClientConfig_setDefault(config);
    if (status == UA_STATUSCODE_GOOD)
    {
        if (usr.id.empty() || usr.passwd.empty())
            status = UA_Client_connect(_client, address.data());
        else
            status = UA_Client_connectUsername(_client, address.data(), usr.id.c_str(), usr.passwd.c_str());
    }
    if (status != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "Failed to create client");
        UA_Client_delete(_client);
        _client = nullptr;
    }
}

rm::Client::~Client()
{
    auto status = UA_Client_disconnect(_client);
    if (status != UA_STATUSCODE_GOOD)
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "Failed to disconnect the client");
    UA_Client_delete(_client);
    _client = nullptr;
}
