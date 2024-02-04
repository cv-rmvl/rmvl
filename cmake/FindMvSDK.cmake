# add the include directories path
find_path(
  MvSDK_INCLUDE_DIR
  NAMES "CameraApi.h"
  PATHS "/usr/include"
  NO_DEFAULT_PATH
)

# add libraries
find_library(
  MvSDK_LIB
  PATHS "/lib"
  NAMES "libMVSDK.so"
  NO_DEFAULT_PATH
)

mark_as_advanced(MvSDK_INCLUDE_DIR MvSDK_LIB)

set(MvSDK_INCLUDE_DIRS "${MvSDK_INCLUDE_DIR}")
set(MvSDK_LIBS "${MvSDK_LIB}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  MvSDK
  REQUIRED_VARS MvSDK_LIB MvSDK_INCLUDE_DIR
)
