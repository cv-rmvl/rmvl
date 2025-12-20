rmvl_generate_para(lpss)

# # file(xxx)
# foreach(m string unit8 unit16 unit32 unit64 float32 float64)
#   rmvl_generate_msg(
#     ${m}
#     MODULE std
#   )
#   set(std_src ${std_src} src/${m})
# endforeach()

# file(xxx)
file(GLOB MSG_FILES "${CMAKE_CURRENT_LIST_DIR}/msg/*.msg")
foreach(msg_file ${MSG_FILES})
  get_filename_component(m ${msg_file} NAME_WE)
  rmvl_generate_msg(
    ${m}
    MODULE std
  )
  set(std_src ${std_src} src/${m})
endforeach()

rmvl_generate_module_msg(std)

rmvl_generate_module_msg(std)

rmvl_add_module(
  lpss
  DEPENDS io
  EXTRA_SOURCE ${std_src}
)

# --------------------------------------------------------------------------
#  Build the test program
# --------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    lpss Unit
    DEPENDS lpss
    EXTERNAL GTest::gtest_main
  )
endif(BUILD_TESTS)

if(BUILD_PERF_TESTS)
  rmvl_add_test(
    lpss Performance
    DEPENDS lpss
    EXTERNAL benchmark::benchmark_main
  )
endif(BUILD_PERF_TESTS)
