# ----------------------------------------------------------------------------
#  Generate para and add module for each detector
# ----------------------------------------------------------------------------
rmvl_add_module(
  detector INTERFACE
  DEPENDS group
)

# armor_detector
rmvl_update_build(armor_detector WITH_ONNXRUNTIME)
rmvl_generate_para(
  armor_detector
  MODULE detector
)
rmvl_add_module(
  armor_detector
  DEPENDS detector ort planar_tracker
)

# gyro_detector
rmvl_update_build(gyro_detector WITH_ONNXRUNTIME)
rmvl_generate_para(
  gyro_detector
  MODULE detector
)
rmvl_add_module(
  gyro_detector
  DEPENDS detector ort gyro_tracker gyro_group
)

# rune_detector
rmvl_generate_para(
  rune_detector
  MODULE detector
)
rmvl_add_module(
  rune_detector
  DEPENDS detector rune_group
)

# tag_detector
rmvl_generate_para(
  tag_detector
  MODULE detector
)
if(WITH_APRILTAG)
  rmvl_add_module(
    tag_detector
    DEPENDS detector
    EXTERNAL apriltag 
  )
endif()

rmvl_generate_module_para(detector)

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    detector Unit
    DEPENDS armor_detector rune_detector gyro_detector
    DEPEND_TESTS GTest::gtest_main
  )
endif(BUILD_TESTS)

# ----------------------------------------------------------------------------
#  Export the detector modules
# ----------------------------------------------------------------------------
set(RMVL_DETECTOR_MODULES_BUILD ${modules_build} CACHE INTERNAL "modules to be built")

