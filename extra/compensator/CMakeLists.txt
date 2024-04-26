rmvl_add_module(
  compensator INTERFACE
  DEPENDS group
)

rmvl_generate_para(
  gravity_compensator
  MODULE compensator
)

rmvl_generate_para(
  gyro_compensator
  MODULE compensator
)

# gravity_compensator
rmvl_add_module(
  gravity_compensator
  DEPENDS compensator
)

# gyro_compensator
rmvl_add_module(
  gyro_compensator
  DEPENDS compensator gyro_group
)

rmvl_generate_module_para(compensator)

# Tests
if(BUILD_TESTS)
  rmvl_add_test(
    compensator Unit
    DEPENDS gravity_compensator
    DEPEND_TESTS GTest::gtest_main
  )
endif()
