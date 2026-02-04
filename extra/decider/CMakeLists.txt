# ----------------------------------------------------------------------------
#  Generate para and add module for each decider
# ----------------------------------------------------------------------------

# GyroDecider
rmvl_generate_para(
  gyro_decider
  MODULE decider
)
rmvl_add_module(
  gyro_decider
  DEPENDS gyro_compensator gyro_predictor
)

# TranslationDecider
rmvl_generate_para(
  translation_decider
  MODULE decider
)
rmvl_add_module(
  translation_decider
  DEPENDS gravity_compensator armor_predictor
)

# RuneDecider
rmvl_generate_para(
  rune_decider
  MODULE decider
)
rmvl_add_module(
  rune_decider
  DEPENDS rune_predictor spi_rune_predictor gravity_compensator
)

rmvl_generate_module_para(decider)

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)

endif(BUILD_TESTS)

# ----------------------------------------------------------------------------
#  Export the decider modules
# ----------------------------------------------------------------------------
set(RMVL_DECIDER_MODULES_BUILD ${modules_build} CACHE INTERNAL "modules to be built")

