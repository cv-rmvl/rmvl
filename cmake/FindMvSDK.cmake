# add the include directories path
find_path(
  MvSDK_INCLUDE_DIR
  NAMES CameraApi.h CameraDefine.h CameraStatus.h
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

if(NOT TARGET mvsdk)
  add_library(mvsdk SHARED IMPORTED)
  set_target_properties(mvsdk PROPERTIES
    IMPORTED_LOCATION "${MvSDK_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${MvSDK_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(MvSDK_LIB MvSDK_INCLUDE_DIR)

set(MvSDK_LIBS "mvsdk")
set(MvSDK_INCLUDE_DIRS "${MvSDK_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  MvSDK
  REQUIRED_VARS MvSDK_LIB MvSDK_INCLUDE_DIR
)
