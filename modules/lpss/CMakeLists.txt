# --------------------------------------------------------------------------
#  Generate code from IDL
# --------------------------------------------------------------------------
# para
rmvl_generate_para(lpss)

# msg
foreach(dir std geometry sensor)
  file(GLOB msg_files "${CMAKE_CURRENT_LIST_DIR}/msg/${dir}/*.msg")
  foreach(msg_file ${msg_files})
    get_filename_component(msg_name ${msg_file} NAME_WE)
    rmvl_generate_msg(${dir}/${msg_name})
  endforeach()
endforeach()

# --------------------------------------------------------------------------
#  Create module
# --------------------------------------------------------------------------
rmvl_add_module(
  lpss
  DEPENDS io
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

# ----------------------------------------------------------------------------
#  Doxygen update
# ----------------------------------------------------------------------------
rmvl_update_doxygen_predefined("LPSS_CV_VERSION=99999") # Doxygen 设置为支持所有版本的 OpenCV，实际值在 cv.hpp 中定义
