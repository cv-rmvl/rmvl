# ----------------------------------------------------------------------------
#  Add module for io
# ----------------------------------------------------------------------------
rmvl_update_build(io WITH_NLOHMANN_JSON)

rmvl_generate_para(io)

rmvl_add_module(io
  DEPENDS core
  EXTERNAL nlohmann_json::nlohmann_json
)

# for glibc < 2.34
if(UNIX)
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
endif(BUILD_TESTS)

if(BUILD_PERF_TESTS)
  rmvl_add_test(
    io Performance
    DEPENDS io
    EXTERNAL benchmark::benchmark_main
  )
endif(BUILD_PERF_TESTS)
