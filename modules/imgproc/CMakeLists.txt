rmvl_add_module(
  imgproc
  DEPENDS core
  EXTERNAL opencv_imgproc
)

# Build the test program
if(BUILD_TESTS)
  rmvl_add_test(
    imgproc Unit
    DEPENDS imgproc
    DEPEND_TESTS GTest::gtest_main
  )
endif(BUILD_TESTS)

# Build the perf test program
if(BUILD_PERF_TESTS)
  rmvl_add_test(
    imgproc Performance
    DEPENDS imgproc
    DEPEND_TESTS benchmark::benchmark_main
  )
endif(BUILD_PERF_TESTS)
