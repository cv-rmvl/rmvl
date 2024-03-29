# add the include directories path
find_path(
  MvSDK_INCLUDE_DIRS
  NAMES "CameraApi.h"
  PATHS "/usr/include"
  NO_DEFAULT_PATH
)

# add libraries
find_library(
  MvSDK_LIBS
  PATHS "/lib"
  NAMES "libMVSDK.so"
  NO_DEFAULT_PATH
)

mark_as_advanced(MvSDK_INCLUDE_DIRS MvSDK_LIBS)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  MvSDK
  REQUIRED_VARS MvSDK_LIBS MvSDK_INCLUDE_DIRS
)
