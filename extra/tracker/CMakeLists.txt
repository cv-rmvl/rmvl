rmvl_add_module(
  tracker
  DEPENDS combo
)

# planar_tracker
rmvl_generate_para(
  planar_tracker
  MODULE tracker
)
rmvl_add_module(
  planar_tracker
  DEPENDS tracker armor 
)

# gyro_tracker
rmvl_generate_para(
  gyro_tracker
  MODULE tracker
)
rmvl_add_module(
  gyro_tracker
  DEPENDS tracker armor 
)

# rune_tracker
rmvl_generate_para(
  rune_tracker
  MODULE tracker
)
rmvl_add_module(
  rune_tracker
  DEPENDS tracker rune camera
)

rmvl_generate_module_para(tracker)

# build the test program
if(BUILD_TESTS)
  rmvl_add_test(
    tracker Unit
    DEPENDS planar_tracker rune_tracker gyro_tracker
    DEPEND_TESTS GTest::gtest_main
  )
endif(BUILD_TESTS)
