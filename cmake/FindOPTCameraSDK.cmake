set(OPTCameraSDK_root_path "/opt/OPT/OPTCameraDemo")

# add the include directories path
find_path(
  OPTCameraSDK_INCLUDE_DIRS
  NAMES "OPTApi.h"
  PATHS "${OPTCameraSDK_root_path}/include"
  NO_DEFAULT_PATH
)

# add libraries
find_library(
  OPTCameraSDK_LIBS
  NAMES "libOPTSDK.so"
  PATHS "${OPTCameraSDK_root_path}/lib"
  NO_DEFAULT_PATH
)

mark_as_advanced(OPTCameraSDK_INCLUDE_DIRS OPTCameraSDK_LIBS)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  OPTCameraSDK
  REQUIRED_VARS OPTCameraSDK_LIBS OPTCameraSDK_INCLUDE_DIRS
)