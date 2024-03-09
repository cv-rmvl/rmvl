if(NOT BUILD_rmvl_opcua)
  return()
endif()

rmvl_add_exe(
  rmvl_opcua_server
  SOURCES opcua_server.cpp
  DEPENDS opcua
)

rmvl_add_exe(
  rmvl_opcua_client
  SOURCES opcua_client.cpp
  DEPENDS opcua
)
