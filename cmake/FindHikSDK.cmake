if(WIN32)
  set(HikSDK_Path "$ENV{MVCAM_COMMON_RUNENV}")
else()
  set(HikSDK_Path "$ENV{MVCAM_SDK_PATH}")
endif()

if(HikSDK_Path STREQUAL "")
  set(HikSDK_FOUND FALSE)
  return()
endif()

if(UNIX)
  # add the include directories path
  find_path(
    HikSDK_INCLUDE_DIR
    NAMES CameraParams.h MvCameraControl.h MvErrorDefine.h MvISPErrorDefine.h PixelType.h
    PATHS "${HikSDK_Path}/include"
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
    PATHS "${HikSDK_Path}/lib/${ARCH_HIKLIB}"
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
elseif(WIN32)
  # add the include directories path
  find_path(
    HikSDK_INCLUDE_DIR
    NAMES CameraParams.h MvCameraControl.h MvErrorDefine.h MvISPErrorDefine.h PixelType.h
    PATHS "${HikSDK_Path}/Includes"
    NO_DEFAULT_PATH
  )
  # add import libraries
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_HIKLIB "64")
    set(ARCH_HIKRT "Win64_x64")
  else()
    set(ARCH_HIKLIB "32")
    set(ARCH_HIKRT "Win32_i86")
  endif()
  find_library(
    HikSDK_LIB
    NAMES MvCameraControl.lib
    PATHS "${HikSDK_Path}/Libraries/win${ARCH_HIKLIB}"
    NO_DEFAULT_PATH
  )
  # add dynamic libraries
  find_file(
    HikSDK_DLL
    NAMES MvCameraControl.dll
    PATHS "C:/Program Files (x86)/Common Files/MVS/Runtime/${ARCH_HIKRT}"
    NO_DEFAULT_PATH
  )

  if(NOT TARGET hiksdk)
    add_library(hiksdk SHARED IMPORTED)
    set_target_properties(hiksdk PROPERTIES
      IMPORTED_IMPLIB "${HikSDK_LIB}"
      IMPORTED_LOCATION "${HikSDK_DLL}"
      INTERFACE_INCLUDE_DIRECTORIES "${HikSDK_INCLUDE_DIR}"
    )
  endif()

  mark_as_advanced(HikSDK_INCLUDE_DIR HikSDK_LIB HikSDK_DLL)
endif()

set(HikSDK_LIBS "hiksdk")
set(HikSDK_INCLUDE_DIRS "${HikSDK_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  HikSDK
  REQUIRED_VARS HikSDK_LIB HikSDK_INCLUDE_DIR
)