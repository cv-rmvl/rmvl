rmvl_add_module(
  group INTERFACE
  DEPENDS tracker
)

# gyro_group
rmvl_generate_para(
  gyro_group
  MODULE group
)
rmvl_add_module(
  gyro_group
  DEPENDS group gyro_tracker 
)

# rune_group
rmvl_generate_para(
  rune_group
  MODULE group
)
rmvl_add_module(
  rune_group
  DEPENDS group rune_tracker 
)

rmvl_generate_module_para(group)

# build the test program
if(BUILD_TESTS)
  rmvl_add_test(
    group Unit
    DEPENDS gyro_group gyro_tracker
    DEPEND_TESTS GTest::gtest_main
  )
endif(BUILD_TESTS)
