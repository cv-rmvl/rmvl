set(hik_sdk_path "$ENV{MVCAM_SDK_PATH}")
if(hik_sdk_path STREQUAL "")
  set(HikSDK_FOUND FALSE)
  return()
endif()

# add the include directories path
find_path(
  HikSDK_INCLUDE_DIR
  NAMES CameraParams.h MvCameraControl.h MvErrorDefine.h MvISPErrorDefine.h PixelType.h
  PATHS "${hik_sdk_path}/include"
  NO_DEFAULT_PATH
)

# add libraries
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "amd64")
  set(ARCH_HIKLIB "64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86" OR arch STREQUAL "i386")
  set(ARCH_HIKLIB "32")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
  set(ARCH_HIKLIB "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  set(ARCH_HIKLIB "arm")
else()
  message(STATUS "Unsupported architecture: ${ARCH_HIKLIB}")
  set(HikSDK_FOUND FALSE)
  return()
endif()

find_library(
  HikSDK_LIB
  NAMES "libMvCameraControl.so"
  PATHS "${hik_sdk_path}/lib/${ARCH_HIKLIB}"
  NO_DEFAULT_PATH
)

if(NOT TARGET hiksdk)
  add_library(hiksdk SHARED IMPORTED)
  set_target_properties(hiksdk PROPERTIES
    IMPORTED_LOCATION "${HikSDK_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${HikSDK_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(HikSDK_LIB HikSDK_INCLUDE_DIR)

set(HikSDK_LIBS "hiksdk")
set(HikSDK_INCLUDE_DIRS "${HikSDK_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  HikSDK
  REQUIRED_VARS HikSDK_LIB HikSDK_INCLUDE_DIR
)