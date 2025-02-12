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
  DEPENDS core light
) 

# ----------------------------------------------------------------------------
#  Parameters module for 'light'
# ----------------------------------------------------------------------------
rmvl_generate_module_para(light)

# --------------------------------------------------------------------------
#   Install the necessary library for OS
# --------------------------------------------------------------------------
foreach(SDK OPTLightCtrl)
  if(${SDK}_FOUND AND DEFINED ${SDK}_LIB)
    if(IS_SYMLINK "${${SDK}_LIB}")
      get_filename_component(lib_realname ${${SDK}_LIB} REALPATH)
      set(lib_install ${lib_realname} ${${SDK}_LIB})
    else()
      set(lib_install ${${SDK}_LIB})
    endif()
    install(
      FILES ${lib_install}
      DESTINATION ${RMVL_3P_LIB_INSTALL_PATH}
    )
    if(DEFINED ${SDK}_DLL)
      if(IS_SYMLINK "${${SDK}_DLL}")
        get_filename_component(dll_realname ${${SDK}_DLL} REALPATH)
        set(dll_install ${dll_realname} ${${SDK}_DLL})
      else()
        set(dll_install ${${SDK}_DLL})
      endif()
      install(
        FILES ${dll_install}
        DESTINATION ${RMVL_BIN_INSTALL_PATH}
      )
    endif()
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