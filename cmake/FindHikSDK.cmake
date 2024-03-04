set(mvcam_sdk_path "$ENV{MVCAM_SDK_PATH}")
if(mvcam_sdk_path STREQUAL "")
  set(HikSDK_FOUND FALSE)
  return()
endif()

# add the include directories path
find_path(
  HikSDK_INCLUDE_DIRS
  NAMES "MvCameraControl.h"
  PATHS "${mvcam_sdk_path}/include"
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
  HikSDK_LIBS
  NAMES "libMvCameraControl.so"
  PATHS "${mvcam_sdk_path}/lib/${ARCH_HIKLIB}"
  NO_DEFAULT_PATH
)

mark_as_advanced(ARCH_HIKLIB HikSDK_INCLUDE_DIRS HikSDK_LIBS)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  HikSDK
  REQUIRED_VARS HikSDK_LIBS HikSDK_INCLUDE_DIRS
)