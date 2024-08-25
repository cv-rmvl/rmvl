find_package(OPTLightCtrl QUIET)
rmvl_update_build(opt_light_control OPTLightCtrl_FOUND)

rmvl_add_module(
  opt_light_control
  DEPENDS core
)

rmvl_link_libraries(
  opt_light_control
  PRIVATE optlc
)

# ----------------------------------------------------------------------------
#  Build Python bindings
# ----------------------------------------------------------------------------
if(BUILD_PYTHON)
  foreach(m opt)
    if(BUILD_rmvl_${m}_light_control)
      list(APPEND light_inc rmvl/light/${m}_light_control.h)
      list(APPEND light_dep ${m}_light_control)
    endif()
  endforeach()
  rmvl_generate_python(light
    FILES ${light_inc}
    DEPENDS ${light_dep}
  )
endif()