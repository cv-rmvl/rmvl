# ----------------------------------------------------------------------------
#  3rdparty and predefined macros
# ----------------------------------------------------------------------------
find_package(Threads REQUIRED)

rmvl_check_include_file(
  FILES termios.h dirent.h fcntl.h unistd.h
  DETAILS CORE_DEFINITIONS
)

if(WITH_OPENCV)
  set(cv_dep opencv_core)
endif()

rmvl_add_module(
  core
  EXTERNAL ${CMAKE_THREAD_LIBS_INIT} ${cv_dep}
)

rmvl_compile_definitions(
  core
  PUBLIC ${CORE_DEFINITIONS}
)

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    core Unit
    DEPENDS core
    DEPEND_TESTS GTest::gtest_main
  )
endif(BUILD_TESTS)

if(BUILD_PERF_TESTS)
  rmvl_add_test(
    core Performance
    DEPENDS core
    DEPEND_TESTS benchmark::benchmark_main
  )
endif(BUILD_PERF_TESTS)
