if(WITH_OPEN62541)
  if(NOT BUILD_OPEN62541)
    find_package(open62541)
  endif()
  # generate parameters module
  rmvl_generate_para(opcua)
  # configure library module
  rmvl_add_module(
    opcua
    DEPENDS para
    EXTERNAL open62541
  )
endif()
