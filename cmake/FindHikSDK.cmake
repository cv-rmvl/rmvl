set(MVCAM_SDK_PATH "$ENV{MVCAM_SDK_PATH}")
if(MVCAM_SDK_PATH STREQUAL "")
    set(MVCAM_SDK_PATH "/opt/MVS")
endif()

# add the include directories path
list(APPEND HikSDK_INCLUDE_DIR "${MVCAM_SDK_PATH}/include")
find_path(
    HikSDK_INCLUDE_DIR
    PATH "${HikSDK_INCLUDE_DIR}"
    NO_DEFAULT_PATH
)

# add libraries
find_library(
    HikSDK_LIB
    NAMES "libMvCameraControl.so"
    PATHS "${MVCAM_SDK_PATH}/lib/64"
    NO_DEFAULT_PATH
)

mark_as_advanced(HikSDK_INCLUDE_DIR HikSDK_LIB)

set(HikSDK_INCLUDE_DIRS "${HikSDK_INCLUDE_DIR}")
set(HikSDK_LIBS "${HikSDK_LIB}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    HikSDK
    REQUIRED_VARS HikSDK_LIB HikSDK_INCLUDE_DIR
)