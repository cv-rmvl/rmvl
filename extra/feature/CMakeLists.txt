# ----------------------------------------------------------------------------
#  Generate para and add module for each feature
# ----------------------------------------------------------------------------
rmvl_add_module(
  feature INTERFACE
  DEPENDS types
  EXTERNAL opencv_imgproc opencv_calib3d
)

# light_blob
rmvl_generate_para(
  light_blob
  MODULE feature
)
rmvl_add_module(
  light_blob
  DEPENDS feature
)

# pilot
rmvl_generate_para(
  pilot
  MODULE feature
)
rmvl_add_module(
  pilot
  DEPENDS feature
)

# rune_center
rmvl_generate_para(
  rune_center
  MODULE feature
)
rmvl_add_module(
  rune_center
  DEPENDS feature
)

# rune_target
rmvl_generate_para(
  rune_target
  MODULE feature
)
rmvl_add_module(
  rune_target
  DEPENDS feature
)

# tag
rmvl_add_module(
  tag
  DEPENDS feature
)

rmvl_generate_module_para(feature)

# ----------------------------------------------------------------------------
#  build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    feature Unit
    DEPENDS light_blob pilot rune_center rune_target
    DEPEND_TESTS GTest::gtest_main
  )
endif(BUILD_TESTS)

# ----------------------------------------------------------------------------
#  Export the feature modules
# ----------------------------------------------------------------------------
set(RMVL_FEATURE_MODULES_BUILD ${modules_build} CACHE INTERNAL "modules to be built")
