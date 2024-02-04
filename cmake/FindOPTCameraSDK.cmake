set(OPTCameraSDK_root_path "/opt/OPT/OPTCameraDemo")

# add the include directories path
find_path(
  OPTCameraSDK_INCLUDE_DIR
  NAMES "OPTApi.h"
  PATHS "${OPTCameraSDK_root_path}/include"
  NO_DEFAULT_PATH
)

# add libraries
find_library(
  OPTCameraSDK_LIB
  NAMES "libOPTSDK.so"
  PATHS "${OPTCameraSDK_root_path}/lib"
  NO_DEFAULT_PATH
)

mark_as_advanced(OPTCameraSDK_INCLUDE_DIR OPTCameraSDK_LIB)

set(OPTCameraSDK_INCLUDE_DIRS "${OPTCameraSDK_INCLUDE_DIR}")
set(OPTCameraSDK_LIBS "${OPTCameraSDK_LIB}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  OPTCameraSDK
  REQUIRED_VARS OPTCameraSDK_LIB OPTCameraSDK_INCLUDE_DIR
)