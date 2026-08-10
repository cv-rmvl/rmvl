rmvl_add_module(
  logger
  DEPENDS io
  EXTERNAL fmt
)

# --------------------------------------------------------------------------
#  Build the test program
# --------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    logger Unit
    DEPENDS logger
    EXTERNAL GTest::gtest_main
  )
endif(BUILD_TESTS)

if(BUILD_PERF_TESTS)
  rmvl_add_test(
    logger Performance
    DEPENDS logger
    EXTERNAL benchmark::benchmark_main
  )
endif(BUILD_PERF_TESTS)
