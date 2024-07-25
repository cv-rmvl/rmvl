# ----------------------------------------------------------------------------
#  Generate para and add module for algorithm
# ----------------------------------------------------------------------------
rmvl_generate_para(algorithm)

rmvl_add_module(
  algorithm
  DEPENDS core
  EXTERNAL $<$<BOOL:${WITH_OPENCV}>:opencv_imgproc>
)

rmvl_compile_definitions(
  algorithm
  PUBLIC $<$<BOOL:${WITH_OPENCV}>:HAVE_OPENCV>
)

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    algorithm Unit
    DEPENDS algorithm
    EXTERNAL GTest::gtest_main
  )
endif(BUILD_TESTS)

if(BUILD_PERF_TESTS)
  rmvl_add_test(
    algorithm Performance
    DEPENDS algorithm
    EXTERNAL benchmark::benchmark_main $<$<BOOL:${WITH_OPENCV}>:opencv_videoio>
  )
endif(BUILD_PERF_TESTS)
