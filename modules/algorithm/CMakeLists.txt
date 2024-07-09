# ----------------------------------------------------------------------------
#  Generate para and add module for algorithm
# ----------------------------------------------------------------------------
set(BUILD_rmvl_algorithm_INIT ${WITH_OPENCV})
rmvl_generate_para(algorithm)

rmvl_add_module(
  algorithm
  DEPENDS core
  EXTERNAL opencv_imgproc
)

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    algorithm Unit
    DEPENDS algorithm
    DEPEND_TESTS GTest::gtest_main
  )
endif(BUILD_TESTS)

if(BUILD_PERF_TESTS)
  rmvl_add_test(
    algorithm Performance
    DEPENDS algorithm
    DEPEND_TESTS benchmark::benchmark_main opencv_video
  )
endif(BUILD_PERF_TESTS)
