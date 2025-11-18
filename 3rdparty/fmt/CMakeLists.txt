# ----------------------------------------------------------------------------
#   fmt
# ----------------------------------------------------------------------------

project(
  fmt
  LANGUAGES CXX
)

rmvl_add_module(fmt)

if(BUILD_TESTS)
  rmvl_add_exe(
    rmvl_fmt_test
    SOURCES test/test_fmt.cpp
    DEPENDS fmt
  )
  add_test(FMT.str ${CMAKE_BINARY_DIR}/bin/rmvl_fmt_test)
endif()
  