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
  rmvl_generate_python(
    light
    DEPENDS opt_light_control
  )
endif()