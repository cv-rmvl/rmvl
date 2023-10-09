rmvl_add_module(
  combo
  DEPENDS feature camera
)

# armor
rmvl_generate_para(
  armor
  MODULE combo
)
rmvl_add_module(
  armor
  DEPENDS combo light_blob 
)

# rune
rmvl_generate_para(
  rune
  MODULE combo
)
rmvl_add_module(
  rune
  DEPENDS combo rune_center rune_target
)

rmvl_generate_module_para(combo)

# build the test program
if(BUILD_TESTS)
  rmvl_add_test(
    combo Unit
    DEPENDS armor rune
    DEPEND_TESTS GTest::gtest_main
  )
endif(BUILD_TESTS)
