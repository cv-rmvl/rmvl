set(MVCAM_SDK_PATH "$ENV{MVCAM_SDK_PATH}")
if(MVCAM_SDK_PATH STREQUAL "")
  set(HikSDK_FOUND FALSE)
  return()
endif()

# add the include directories path
find_path(
  HikSDK_INCLUDE_DIR
  NAMES "MvCameraControl.h"
  PATHS "${MVCAM_SDK_PATH}/include"
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
  PATHS "${MVCAM_SDK_PATH}/lib/${ARCH_HIKLIB}"
  NO_DEFAULT_PATH
)

mark_as_advanced(ARCH_HIKLIB HikSDK_INCLUDE_DIR HikSDK_LIB)

set(HikSDK_INCLUDE_DIRS "${HikSDK_INCLUDE_DIR}")
set(HikSDK_LIBS "${HikSDK_LIB}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  HikSDK
  REQUIRED_VARS HikSDK_LIB HikSDK_INCLUDE_DIR
)