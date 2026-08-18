file(GLOB msg_files "${CMAKE_CURRENT_LIST_DIR}/msg/nav/*.msg")
foreach(msg_file ${msg_files})
  get_filename_component(msg_name ${msg_file} NAME_WE)
  rmvl_generate_msg(nav/${msg_name})
endforeach()

rmvl_add_module(
  nav
  DEPENDS lpss
)

if(BUILD_TESTS)
  rmvl_add_test(
    nav Unit
    DEPENDS nav
    EXTERNAL GTest::gtest_main
  )
endif()

if(BUILD_PERF_TESTS)
  rmvl_add_test(
    nav Performance
    DEPENDS nav
    EXTERNAL benchmark::benchmark_main
  )
endif()
