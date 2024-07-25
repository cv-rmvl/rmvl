set(BUILD_rmvl_opcua_INIT ${WITH_OPEN62541})
# generate parameters module
rmvl_generate_para(opcua)
# configure library module
rmvl_add_module(
  opcua
  DEPENDS core
  EXTERNAL open62541::open62541
)

# add test
if(BUILD_TESTS AND WITH_OPEN62541)
  rmvl_add_test(
    opcua Unit
    DEPENDS opcua
    EXTERNAL GTest::gtest_main
  )
endif()

# doxygen update
rmvl_update_doxygen_predefined("UA_ENABLE_PUBSUB")
