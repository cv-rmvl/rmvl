# ----------------------------------------------------------------------------
#  rmvl_modules.hpp based on actual modules list
# ----------------------------------------------------------------------------
if(WITH_OPENCV AND WITH_EIGEN3)
  set(RMVL_MODULE_DEFINITIONS_CONFIGMAKE "#ifndef HAVE_OPENCV\n#define HAVE_OPENCV\n#endif\n\n")
else()
  set(RMVL_MODULE_DEFINITIONS_CONFIGMAKE "")
endif()

set(RMVL_MOD_LIST ${RMVL_MODULES_BUILD})
foreach(m ${RMVL_MOD_LIST})
  string(TOUPPER "${m}" m)
  set(RMVL_MODULE_DEFINITIONS_CONFIGMAKE "${RMVL_MODULE_DEFINITIONS_CONFIGMAKE}#define HAVE_${m}\n")
endforeach()

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/templates/rmvl_modules.hpp.in
  ${CMAKE_BINARY_DIR}/rmvl/rmvl_modules.hpp
  @ONLY
)
install(
  FILES ${CMAKE_BINARY_DIR}/rmvl/rmvl_modules.hpp
  DESTINATION ${RMVL_INCLUDE_INSTALL_PATH}/rmvl
)
