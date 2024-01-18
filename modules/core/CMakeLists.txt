find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

rmvl_generate_para(core)

rmvl_add_module(
  core
  EXTERNAL ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT}
)

# build the test program
if(BUILD_TESTS)
  rmvl_add_test(
    core Unit
    DEPENDS core
    DEPEND_TESTS GTest::gtest_main
  )
endif(BUILD_TESTS)
