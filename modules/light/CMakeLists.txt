find_package(OPTLightCtrl)
set(BUILD_rmvl_opt_light_control_INIT ${OPTLightCtrl_FOUND})

rmvl_add_module(
    opt_light_control
    EXTRA_HEADER ${OPTLightCtrl_INCLUDE_DIRS}
    EXTERNAL ${OPTLightCtrl_LIBS}
)
