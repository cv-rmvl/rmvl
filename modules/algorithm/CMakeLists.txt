# ----------------------------------------------------------------------------
#  Generate para and add module for algorithm
# ----------------------------------------------------------------------------
rmvl_generate_para(algorithm)

if(WITH_OPENCV)
  rmvl_add_module(algorithm DEPENDS core EXTERNAL opencv_imgproc)
  rmvl_compile_definitions(algorithm PUBLIC HAVE_OPENCV)
else()
  rmvl_add_module(algorithm DEPENDS core)
endif()

# ----------------------------------------------------------------------------
#  Build Python bindings
# ----------------------------------------------------------------------------
if(BUILD_PYTHON)
  rmvl_generate_python(algorithm
    FILES rmvl/algorithm/numcal.hpp
    DEPENDS algorithm
  )
endif()

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

# ----------------------------------------------------------------------------
#  Doxygen update
# ----------------------------------------------------------------------------
rmvl_update_doxygen_predefined("HAVE_OPENCV")
