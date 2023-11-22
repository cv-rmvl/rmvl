find_package(Eigen3 REQUIRED)

rmvl_add_module(
  rmath
  DEPENDS core
  EXTRA_HEADER ${EIGEN3_INCLUDE_DIR}
  EXTERNAL
)

# build the test program
if(BUILD_TESTS)
  rmvl_add_test(
    rmath Unit
    DEPENDS rmath
    DEPEND_TESTS GTest::gtest_main
  )
endif(BUILD_TESTS)
