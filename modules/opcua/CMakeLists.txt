# ----------------------------------------------------------------------------
#  Generate para and add module for opcua
# ----------------------------------------------------------------------------
rmvl_update_build(opcua WITH_OPEN62541)
# generate parameters module
rmvl_generate_para(opcua)
# configure library module
rmvl_add_module(
  opcua
  DEPENDS core
  EXTERNAL open62541::open62541
)

# ----------------------------------------------------------------------------
#  Build Python bindings
# ----------------------------------------------------------------------------
if(BUILD_PYTHON)
  foreach(m utilities variable method event object view client server)
    list(APPEND opcua_inc rmvl/opcua/${m}.hpp)
  endforeach()
  rmvl_generate_python(opcua
    FILES ${opcua_inc} rmvlpara/opcua.hpp
    DEPENDS opcua
  )
endif()

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS AND WITH_OPEN62541)
  rmvl_add_test(
    opcua Unit
    DEPENDS opcua
    EXTERNAL GTest::gtest_main
  )
endif()

# doxygen update
rmvl_update_doxygen_predefined("UA_ENABLE_PUBSUB")
