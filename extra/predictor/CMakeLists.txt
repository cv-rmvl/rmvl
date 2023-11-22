rmvl_add_module(
  predictor INTERFACE
  DEPENDS group
)

# armor_predictor
rmvl_generate_para(
  armor_predictor
  MODULE predictor
)
rmvl_add_module(
  armor_predictor
  DEPENDS predictor rmath camera
)

# rune_predictor
rmvl_generate_para(
  rune_predictor
  MODULE predictor
)
rmvl_add_module(
  rune_predictor
  DEPENDS predictor rmath camera rune_group
)

# spi_rune_predictor
rmvl_generate_para(
  spi_rune_predictor
  MODULE predictor
)
rmvl_add_module(
  spi_rune_predictor
  DEPENDS predictor rmath rune_group
)

# gyro_predictor
rmvl_generate_para(
  gyro_predictor
  MODULE predictor
)
rmvl_add_module(
  gyro_predictor
  DEPENDS predictor rmath camera
)

rmvl_generate_module_para(predictor)

# build the test program
if(BUILD_TESTS)
  rmvl_add_test(
    predictor Unit
    DEPENDS spi_rune_predictor
    DEPEND_TESTS GTest::gtest_main
  )
endif(BUILD_TESTS)
