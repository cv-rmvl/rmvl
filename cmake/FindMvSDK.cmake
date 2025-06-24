# --------------------------------------------------------------------------------------------
#  This file is used to find the MindVision camera SDK, which provides the following variables:
#
#  Advanced Variables:
#  - MvSDK_INCLUDE_DIR: Directory where SDK header files are located
#  - MvSDK_LIB:         Path to the SDK library file (import library on Windows, shared
#                       library on Linux)
#  - MvSDK_DLL:         Path to the SDK dynamic library file (only on Windows)
#
#  Local Variables:
#  - MvSDK_LIBS:         CMake target name for the SDK, which is "mvsdk"
#  - MvSDK_INCLUDE_DIRS: Directory where SDK header files are located
# --------------------------------------------------------------------------------------------

if(UNIX)
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
    NAMES libMVSDK.so
    PATHS "/lib"
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
elseif(WIN32)
  if(NOT DEFINED MvSDK_Path)
    set(MvSDK_Path "C:/Program Files (x86)/MindVision")
  endif()

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_MVLIB "X64")
  else()
    set(ARCH_MVLIB "X86")
  endif()

  # add the include directories path
  find_path(
    MvSDK_INCLUDE_DIR
    NAMES CameraApi.h CameraDefine.h CameraStatus.h
    PATHS "${MvSDK_Path}/Demo/VC++/Include"
    NO_DEFAULT_PATH
  )
  # add import libraries
  find_library(
    MvSDK_LIB
    NAMES MVCAMSDK_${ARCH_MVLIB}.lib
    PATHS "${MvSDK_Path}/SDK/${ARCH_MVLIB}"
    NO_DEFAULT_PATH
  )
  # add dynamic libraries
  find_file(
    MvSDK_DLL
    NAMES MVCAMSDK_${ARCH_MVLIB}.dll
    PATHS "${MvSDK_Path}/SDK/${ARCH_MVLIB}"
    NO_DEFAULT_PATH
  )

  if(NOT TARGET mvsdk)
    add_library(mvsdk SHARED IMPORTED GLOBAL)
    set_target_properties(mvsdk PROPERTIES
      IMPORTED_IMPLIB "${MvSDK_LIB}"
      IMPORTED_LOCATION "${MvSDK_DLL}"
      INTERFACE_INCLUDE_DIRECTORIES "${MvSDK_INCLUDE_DIR}"
    )
  endif()

  mark_as_advanced(MvSDK_LIB MvSDK_DLL MvSDK_INCLUDE_DIR)
endif()

set(MvSDK_LIBS "mvsdk")
set(MvSDK_INCLUDE_DIRS "${MvSDK_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  MvSDK
  REQUIRED_VARS MvSDK_LIB MvSDK_INCLUDE_DIR
)
