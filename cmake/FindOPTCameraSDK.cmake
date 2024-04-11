set(OPTCameraSDK_root_path "/opt/OPT/OPTCameraDemo")

# add the include directories path
find_path(
  OPTCameraSDK_INCLUDE_DIR
  NAMES OPTApi.h OPTDefines.h
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

if(NOT TARGET optcamsdk)
  add_library(optcamsdk SHARED IMPORTED)
  set_target_properties(optcamsdk PROPERTIES
    IMPORTED_LOCATION "${OPTCameraSDK_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPTCameraSDK_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(OPTCameraSDK_INCLUDE_DIR OPTCameraSDK_LIB)

set(OPTCameraSDK_LIBS "optcamsdk")
set(OPTCameraSDK_INCLUDE_DIRS "${OPTCameraSDK_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  OPTCameraSDK
  REQUIRED_VARS OPTCameraSDK_LIB OPTCameraSDK_INCLUDE_DIR
)