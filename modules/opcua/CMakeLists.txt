if(WITH_OPEN62541)
  if(BUILD_OPEN62541)
    find_package(open62541)
  endif()
  rmvl_add_module(
    opcua
    DEPENDS core
    EXTERNAL open62541
  )
endif()
