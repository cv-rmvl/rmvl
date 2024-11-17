# ----------------------------------------------------------------------------
#  Generate para and add module for types
# ----------------------------------------------------------------------------
rmvl_add_module(
  types
  DEPENDS core
)

# ----------------------------------------------------------------------------
#  Build Python bindings
# ----------------------------------------------------------------------------
if(BUILD_PYTHON)
  rmvl_generate_python(types
    FILES rmvl/types.hpp
    DEPENDS types
  )
endif()

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    types Unit
    DEPENDS types
    EXTERNAL GTest::gtest_main
  )
endif()
