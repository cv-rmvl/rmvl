# ----------------------------------------------------------------------------
#  Generate para and add module for each decider
# ----------------------------------------------------------------------------
rmvl_add_module(
  decider INTERFACE
  DEPENDS detector compensator predictor
)

# GyroDecider
rmvl_generate_para(
  gyro_decider
  MODULE decider
)
rmvl_add_module(
  gyro_decider
  DEPENDS decider camera
)

# TranslationDecider
rmvl_generate_para(
  translation_decider
  MODULE decider
)
rmvl_add_module(
  translation_decider
  DEPENDS decider camera
)

# RuneDecider
rmvl_generate_para(
  rune_decider
  MODULE decider
)
rmvl_add_module(
  rune_decider
  DEPENDS decider camera
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

