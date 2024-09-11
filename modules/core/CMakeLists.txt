# ----------------------------------------------------------------------------
#  3rdparty and predefined macros
# ----------------------------------------------------------------------------
find_package(Threads REQUIRED)

rmvl_check_include_file(
  FILES termios.h dirent.h fcntl.h unistd.h
  DETAILS CORE_DEFINITIONS
)

rmvl_add_module(
  core
  EXTERNAL ${CMAKE_THREAD_LIBS_INIT} $<$<BOOL:${WITH_OPENCV}>:opencv_core>
)

rmvl_compile_definitions(
  core
  PUBLIC ${CORE_DEFINITIONS}
)

# ----------------------------------------------------------------------------
#  Build Python bindings
# ----------------------------------------------------------------------------
if(BUILD_PYTHON)
  foreach(m dataio timer version)
    list(APPEND core_inc rmvl/core/${m}.hpp)
  endforeach()
  rmvl_generate_python(core
    FILES ${core_inc}
    DEPENDS core
  )
endif()

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    core Unit
    DEPENDS core
    EXTERNAL GTest::gtest_main
  )
endif(BUILD_TESTS)
