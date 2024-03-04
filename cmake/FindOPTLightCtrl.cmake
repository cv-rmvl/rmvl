set(OPTLightCtrl_root_path "/opt/OPT/OPTController")

# add the include directories path
find_path(
  OPTLightCtrl_INCLUDE_DIRS
  NAMES "OPTController.h"
  PATHS "${OPTLightCtrl_root_path}/include"
  NO_DEFAULT_PATH
)

# add libraries
find_library(
  OPTLightCtrl_LIBS
  NAMES "libOPTController.so"
  PATHS "${OPTLightCtrl_root_path}/lib"
  NO_DEFAULT_PATH
)

mark_as_advanced(OPTLightCtrl_INCLUDE_DIRS OPTLightCtrl_LIBS)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  OPTLightCtrl
  REQUIRED_VARS OPTLightCtrl_LIBS OPTLightCtrl_INCLUDE_DIRS
)