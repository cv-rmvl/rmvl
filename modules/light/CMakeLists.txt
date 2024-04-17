find_package(OPTLightCtrl QUIET)
set(BUILD_rmvl_opt_light_control_INIT ${OPTLightCtrl_FOUND})

rmvl_add_module(opt_light_control)

rmvl_link_libraries(
  opt_light_control
  PRIVATE optlc
)