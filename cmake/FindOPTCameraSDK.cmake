set(OPTCameraSDK_root_path "/opt/OPT/OPTCameraDemo")

# add the include directories path
list(APPEND OPTCameraSDK_INCLUDE_DIR "${OPTCameraSDK_root_path}/include")
find_path(
    OPTCameraSDK_INCLUDE_DIR
    PATH "${OPTCameraSDK_INCLUDE_DIR}"
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
    REQUIRED_VARS OPTCameraSDK_LIB
)