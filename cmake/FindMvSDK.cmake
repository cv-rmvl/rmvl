# add the include directories path
list(APPEND MvSDK_INCLUDE_DIR "/usr/include")
find_path(
  MvSDK_INCLUDE_DIR
  PATH "${MvSDK_INCLUDE_DIR}"
  NO_DEFAULT_PATH
)

if(EXISTS "${MvSDK_INCLUDE_DIR}/CameraApi.h")
  set(inc_file_exists ON)
else()
  set(inc_file_exists OFF)
endif()


# add libraries
find_library(
  MvSDK_LIB
  NAMES "libMVSDK.so"
  PATHS "/lib"
  NO_DEFAULT_PATH
)

mark_as_advanced(MvSDK_INCLUDE_DIR MvSDK_LIB)

set(MvSDK_INCLUDE_DIRS "${MvSDK_INCLUDE_DIR}")
set(MvSDK_LIBS "${MvSDK_LIB}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  MvSDK
  REQUIRED_VARS MvSDK_LIB inc_file_exists
)
