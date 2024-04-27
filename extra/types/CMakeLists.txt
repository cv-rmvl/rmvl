# ----------------------------------------------------------------------------
#  Generate para and add module for types
# ----------------------------------------------------------------------------
rmvl_add_module(
  types
  DEPENDS core
)

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    types Unit
    DEPENDS types
    DEPEND_TESTS GTest::gtest_main
  )
endif()
