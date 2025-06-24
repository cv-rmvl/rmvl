# --------------------------------------------------------------------------------------------
#  This file is used to find the OPT Machine Vision camera SDK, which provides the following
#  variables:
#
#  Advanced Variables:
#  - OPTCameraSDK_INCLUDE_DIR: Directory where SDK header files are located
#  - OPTCameraSDK_LIB:         Path to the SDK library file (import library on Windows,
#                              shared library on Linux)
#  - OPTCameraSDK_DLL:         Path to the SDK dynamic library file (only on Windows)
#
#  Local Variables:
#  - OPTCameraSDK_LIBS:         CMake target name for the SDK, which is "optcamsdk"
#  - OPTCameraSDK_INCLUDE_DIRS: Directory where SDK header files are located
# --------------------------------------------------------------------------------------------

set(OPTCameraSDK_Path "/opt/OPT/OPTCameraDemo")

# add the include directories path
find_path(
  OPTCameraSDK_INCLUDE_DIR
  NAMES OPTApi.h OPTDefines.h
  PATHS "${OPTCameraSDK_Path}/include"
  NO_DEFAULT_PATH
)

# add libraries
find_library(
  OPTCameraSDK_LIB
  NAMES "libOPTSDK.so"
  PATHS "${OPTCameraSDK_Path}/lib"
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