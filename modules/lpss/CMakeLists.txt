rmvl_generate_para(lpss)

rmvl_add_module(
  lpss
  DEPENDS io
)

# --------------------------------------------------------------------------
#  Build the test program
# --------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    lpss Unit
    DEPENDS lpss
    EXTERNAL GTest::gtest_main
  )
endif(BUILD_TESTS)

if(BUILD_PERF_TESTS)
  rmvl_add_test(
    lpss Performance
    DEPENDS lpss
    EXTERNAL benchmark::benchmark_main
  )
endif(BUILD_PERF_TESTS)
