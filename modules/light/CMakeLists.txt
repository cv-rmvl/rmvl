find_package(OPTLightCtrl QUIET)
rmvl_update_build(opt_light_control OPTLightCtrl_FOUND)

rmvl_add_module(opt_light_control)

rmvl_link_libraries(
  opt_light_control
  PRIVATE optlc
)