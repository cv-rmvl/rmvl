# ----------------------------------------------------------------------------
#  Add module for io
# ----------------------------------------------------------------------------
rmvl_generate_para(io)

rmvl_add_module(io
  DEPENDS core
  EXTERNAL json fmt
)

if(WITH_OPENSSL)
  rmvl_link_libraries(io PUBLIC OpenSSL::SSL OpenSSL::Crypto)
  rmvl_compile_definitions(io PUBLIC HAVE_OPENSSL)
endif()

# for glibc < 2.34
if(UNIX)
  find_package(Threads REQUIRED)
  rmvl_link_libraries(io PRIVATE Threads::Threads)
  rmvl_link_libraries(io PRIVATE rt)
endif()

# ----------------------------------------------------------------------------
#  Build Python bindings
# ----------------------------------------------------------------------------
if(BUILD_PYTHON)
  foreach(m util ipc serial netapp)
    list(APPEND io_inc rmvl/io/${m}.hpp)
  endforeach()
  rmvl_generate_python(io
    FILES ${io_inc}
    DEPENDS io
    PREBIND json
  )
endif()

# --------------------------------------------------------------------------
#  Build the test program
# --------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    io Unit
    DEPENDS io
    EXTERNAL GTest::gtest_main
  )
  target_compile_definitions(
    rmvl_io_test PRIVATE
    RMVL_IO_TEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test/data"
  )
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Compile the peripheral sources here so wrapping also works with shared builds.
    add_executable(rmvl_io_peripheral_test
      test/peripheral/test_peripheral.cpp src/gpio.cpp src/i2c.cpp src/can.cpp)
    if(BUILD_WORLD)
      target_link_libraries(rmvl_io_peripheral_test PRIVATE rmvl_world)
    else()
      target_link_libraries(rmvl_io_peripheral_test PRIVATE rmvl_io)
    endif()
    target_link_libraries(rmvl_io_peripheral_test PRIVATE GTest::gtest_main Threads::Threads)
    target_link_options(rmvl_io_peripheral_test PRIVATE
      -Wl,--wrap=ioctl -Wl,--wrap=if_nametoindex -Wl,--wrap=socket
      -Wl,--wrap=bind -Wl,--wrap=setsockopt -Wl,--wrap=send -Wl,--wrap=recv)
    gtest_discover_tests(rmvl_io_peripheral_test TEST_PREFIX "io_peripheral." PROPERTIES TIMEOUT 10)
  endif()
endif(BUILD_TESTS)

if(BUILD_PERF_TESTS)
  rmvl_add_test(
    io Performance
    DEPENDS io
    EXTERNAL benchmark::benchmark_main
  )
endif(BUILD_PERF_TESTS)
