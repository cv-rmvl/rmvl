find_package(open62541)

if(WITH_OPEN62541)
  rmvl_add_module(
    opcua
    DEPENDS core
    EXTERNAL open62541
  )
endif()
