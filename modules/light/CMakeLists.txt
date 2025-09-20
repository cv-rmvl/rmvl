# ----------------------------------------------------------------------------
#  Light control module
# ----------------------------------------------------------------------------
# light
rmvl_add_module(light INTERFACE)

# opt_light_control
find_package(OPTLightCtrl QUIET)
rmvl_update_build(opt_light_control OPTLightCtrl_FOUND)

rmvl_add_module(
  opt_light_control
  DEPENDS core light
  EXTERNAL optlc
)

# hik_light_control
rmvl_generate_para(
  hik_light_control
  MODULE light
)

rmvl_add_module(
  hik_light_control
  DEPENDS light core io
) 

# ----------------------------------------------------------------------------
#  Parameters module for 'light'
# ----------------------------------------------------------------------------
rmvl_generate_module_para(light)

# --------------------------------------------------------------------------
#  Install the necessary library for OS
# --------------------------------------------------------------------------
foreach(sdk_name OPTLightCtrl)
  if(${sdk_name}_FOUND AND DEFINED ${sdk_name}_LIB)
    rmvl_install_sdk(${sdk_name})
  endif()
endforeach()

# ----------------------------------------------------------------------------
#  Build Python bindings
# ----------------------------------------------------------------------------
if(BUILD_PYTHON)
  set(light_inc rmvl/light/lightutils.hpp)
  foreach(m opt hik)
    if(BUILD_rmvl_${m}_light_control)
      list(APPEND light_inc rmvl/light/${m}_light_control.h)
      list(APPEND light_dep ${m}_light_control)
    endif()
  endforeach()
  foreach(m hik)
    if(BUILD_rmvl_${m}_light_control)
      list(APPEND light_inc rmvlpara/light/${m}_light_control.h)
    endif()
  endforeach()
  rmvl_generate_python(light
    FILES ${light_inc}
    DEPENDS ${light_dep}
  )
endif()

# ----------------------------------------------------------------------------
#  Export the light modules
# ----------------------------------------------------------------------------
set(RMVL_LIGHT_MODULES_BUILD ${modules_build} CACHE INTERNAL "modules to be built")
