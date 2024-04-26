find_package(Threads REQUIRED)
find_package(OpenCV COMPONENTS core REQUIRED)
find_package(Eigen3 REQUIRED)

# 生成预定义宏
rmvl_check_include_file(
  FILES termios.h dirent.h fcntl.h unistd.h
  DETAILS CORE_DEFINITIONS
)

# 为 core 模块生成参数
rmvl_generate_para(core)

# 添加 core 模块
rmvl_add_module(
  core
  EXTRA_HEADER ${EIGEN3_INCLUDE_DIR}
  EXTERNAL ${CMAKE_THREAD_LIBS_INIT} opencv_core
)

# 添加预定义宏到 core 模块
rmvl_compile_definitions(
  core
  PUBLIC ${CORE_DEFINITIONS}
)

# 添加测试程序
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
