# ----------------------------------------------------------------------------
#  Generate para and add module for each group
# ----------------------------------------------------------------------------
rmvl_add_module(
  group INTERFACE
  DEPENDS tracker
)

# gyro_group
rmvl_generate_para(
  gyro_group
  MODULE group
)
rmvl_add_module(
  gyro_group
  DEPENDS group gyro_tracker
)

# rune_group
rmvl_generate_para(
  rune_group
  MODULE group
)
rmvl_add_module(
  rune_group
  DEPENDS group rune_tracker
)

rmvl_generate_module_para(group)

# ----------------------------------------------------------------------------
#  Build Python bindings
# ----------------------------------------------------------------------------
if(BUILD_PYTHON)
  foreach(m group gyro_group rune_group)
    if(BUILD_rmvl_${m})
      list(APPEND group_inc rmvl/group/${m}.h) 
      list(APPEND group_dep ${m})
    endif()
  endforeach() 
  foreach(m gyro_group rune_group)
    if(BUILD_rmvl_${m})
      list(APPEND group_inc rmvlpara/group/${m}.h) 
    endif()
  endforeach()

  rmvl_generate_python(group
    FILES ${group_inc}
    DEPENDS ${group_dep}
    PREBIND cv
  )
endif()

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    group Unit
    DEPENDS gyro_group gyro_tracker
    EXTERNAL GTest::gtest_main
  )
endif(BUILD_TESTS)

# ----------------------------------------------------------------------------
#  Export the group modules
# ----------------------------------------------------------------------------
set(RMVL_GROUP_MODULES_BUILD ${modules_build} CACHE INTERNAL "modules to be built")
