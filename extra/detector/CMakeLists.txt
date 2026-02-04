# ----------------------------------------------------------------------------
#  Generate para and add module for each detector
# ----------------------------------------------------------------------------
# armor_detector
rmvl_update_build(armor_detector WITH_ONNXRUNTIME)
rmvl_generate_para(
  armor_detector
  MODULE detector
)
rmvl_add_module(
  armor_detector
  DEPENDS ort planar_tracker
)

# gyro_detector
rmvl_update_build(gyro_detector WITH_ONNXRUNTIME)
rmvl_generate_para(
  gyro_detector
  MODULE detector
)
rmvl_add_module(
  gyro_detector
  DEPENDS ort gyro_tracker gyro_group
)

# rune_detector
rmvl_generate_para(
  rune_detector
  MODULE detector
)
rmvl_add_module(
  rune_detector
  DEPENDS rune_group
)

# tag_detector
rmvl_update_build(tag_detector WITH_APRILTAG)
rmvl_add_module(
  tag_detector
  DEPENDS planar_tracker tag
  EXTERNAL apriltag
)

rmvl_generate_module_para(detector)

# ----------------------------------------------------------------------------
#  Build Python bindings
# ----------------------------------------------------------------------------
if(BUILD_PYTHON)
  foreach(m detector armor_detector gyro_detector rune_detector tag_detector)
    if(BUILD_rmvl_${m})
      list(APPEND detector_inc rmvl/detector/${m}.h) 
      list(APPEND detector_dep ${m})
    endif()
  endforeach() 
  foreach(m armor_detector gyro_detector rune_detector tag_detector)
    if(BUILD_rmvl_${m})
      list(APPEND detector_inc rmvlpara/detector/${m}.h) 
    endif()
  endforeach()

  rmvl_generate_python(detector
    FILES ${detector_inc}
    DEPENDS ${detector_dep}
    PREBIND cv
  )
endif()

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    detector Unit
    DEPENDS armor_detector rune_detector gyro_detector
    EXTERNAL GTest::gtest_main
  )
endif(BUILD_TESTS)

# ----------------------------------------------------------------------------
#  Export the detector modules
# ----------------------------------------------------------------------------
set(RMVL_DETECTOR_MODULES_BUILD ${modules_build} CACHE INTERNAL "modules to be built")

