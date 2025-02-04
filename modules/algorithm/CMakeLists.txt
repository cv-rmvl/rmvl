# ----------------------------------------------------------------------------
#  Generate para and add module for algorithm
# ----------------------------------------------------------------------------
rmvl_generate_para(algorithm)

rmvl_add_module(algorithm DEPENDS core)
if(WITH_OPENCV AND WITH_EIGEN3)
  rmvl_compile_definitions(algorithm PUBLIC HAVE_OPENCV)
endif()

# ----------------------------------------------------------------------------
#  Build Python bindings
# ----------------------------------------------------------------------------
if(BUILD_PYTHON)
  if(WITH_OPENCV)
    foreach(m numcal dsp pretreat transform)
      list(APPEND alg_inc rmvl/algorithm/${m}.hpp)
    endforeach()
    rmvl_generate_python(algorithm
      FILES ${alg_inc} rmvlpara/algorithm.hpp
      DEPENDS algorithm
      USE_PREBIND
    )
  else()
    foreach(m numcal)
      list(APPEND alg_inc rmvl/algorithm/${m}.hpp)
    endforeach()
    rmvl_generate_python(algorithm
      FILES ${alg_inc} rmvlpara/algorithm.hpp
      DEPENDS algorithm
    )
  endif()
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
    EXTERNAL benchmark::benchmark_main
  )
endif(BUILD_PERF_TESTS)

# ----------------------------------------------------------------------------
#  Doxygen update
# ----------------------------------------------------------------------------
rmvl_update_doxygen_predefined("HAVE_OPENCV")
