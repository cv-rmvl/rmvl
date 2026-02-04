# ----------------------------------------------------------------------------
#  Generate para and add module for each predictor
# ----------------------------------------------------------------------------

# armor_predictor
rmvl_generate_para(
  armor_predictor
  MODULE predictor
)
rmvl_add_module(
  armor_predictor
  DEPENDS tracker camera
)

# rune_predictor
rmvl_generate_para(
  rune_predictor
  MODULE predictor
)
rmvl_add_module(
  rune_predictor
  DEPENDS rune_tracker camera rune_group
)

# spi_rune_predictor
rmvl_generate_para(
  spi_rune_predictor
  MODULE predictor
)
rmvl_add_module(
  spi_rune_predictor
  DEPENDS rune_group rune_group
)

# gyro_predictor
rmvl_generate_para(
  gyro_predictor
  MODULE predictor
)
rmvl_add_module(
  gyro_predictor
  DEPENDS gyro_group camera
)

rmvl_generate_module_para(predictor)

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    predictor Unit
    DEPENDS spi_rune_predictor
    EXTERNAL GTest::gtest_main
  )
endif(BUILD_TESTS)

# ----------------------------------------------------------------------------
#  Export the predictor modules
# ----------------------------------------------------------------------------
set(RMVL_PREDICTOR_MODULES_BUILD ${modules_build} CACHE INTERNAL "modules to be built")
