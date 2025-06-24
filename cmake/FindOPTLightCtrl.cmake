# --------------------------------------------------------------------------------------------
#  This file is used to find the OPT Machine Vision light controller SDK, which provides the
#  following variables:
#
#  Advanced Variables:
#  - OPTLightCtrl_INCLUDE_DIR: Directory where SDK header files are located
#  - OPTLightCtrl_LIB:         Path to the SDK library file (import library on Windows,
#                              shared library on Linux)
#  - OPTLightCtrl_DLL:         Path to the SDK dynamic library file (only on Windows)
#
#  Local Variables:
#  - OPTLightCtrl_LIBS:         CMake target name for the SDK, which is "optlc"
#  - OPTLightCtrl_INCLUDE_DIRS: Directory where SDK header files are located
# --------------------------------------------------------------------------------------------

set(OPTLightCtrl_root_path "/opt/OPT/OPTController")

# add the include directories path
find_path(
  OPTLightCtrl_INCLUDE_DIR
  NAMES OPTController.h OPTErrorCode.h
  PATHS "${OPTLightCtrl_root_path}/include"
  NO_DEFAULT_PATH
)

# add libraries
find_library(
  OPTLightCtrl_LIB
  NAMES "libOPTController.so"
  PATHS "${OPTLightCtrl_root_path}/lib"
  NO_DEFAULT_PATH
)

if(NOT TARGET optlc)
  add_library(optlc SHARED IMPORTED)
  set_target_properties(optlc PROPERTIES
    IMPORTED_LOCATION "${OPTLightCtrl_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPTLightCtrl_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(OPTLightCtrl_INCLUDE_DIR OPTLightCtrl_LIB)

set(OPTLightCtrl_LIBS "optlc")
set(OPTLightCtrl_INCLUDE_DIRS "${OPTLightCtrl_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  OPTLightCtrl
  REQUIRED_VARS OPTLightCtrl_LIB OPTLightCtrl_INCLUDE_DIR
)