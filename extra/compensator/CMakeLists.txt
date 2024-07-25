# ----------------------------------------------------------------------------
#  Generate para and add module for each compensator
# ----------------------------------------------------------------------------
rmvl_add_module(
  compensator INTERFACE
  DEPENDS group
)

# gravity_compensator
rmvl_generate_para(
  gravity_compensator
  MODULE compensator
)
rmvl_add_module(
  gravity_compensator
  DEPENDS compensator
)

# gyro_compensator
rmvl_generate_para(
  gyro_compensator
  MODULE compensator
)
rmvl_add_module(
  gyro_compensator
  DEPENDS compensator gyro_group
)

rmvl_generate_module_para(compensator)

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    compensator Unit
    DEPENDS gravity_compensator
    EXTERNAL GTest::gtest_main
  )
endif()

# ----------------------------------------------------------------------------
#  Export the compensator modules
# ----------------------------------------------------------------------------
set(RMVL_COMPENSATOR_MODULES_BUILD ${modules_build} CACHE INTERNAL "modules to be built")
