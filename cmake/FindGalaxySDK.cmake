if(NOT DEFINED GalaxySDK_Path)
  if(UNIX)
    set(GalaxySDK_Path "/opt/Galaxy_camera")
  elseif(WIN32)
    set(GalaxySDK_Path "C:/Program Files/Daheng Imaging/GalaxySDK")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(sys_type "64")
    else()
      set(sys_type "32")
    endif()
  else()
    message(STATUS "Unsupported platform")
    set(GalaxySDK_FOUND FALSE)
    return()
  endif()
endif()

# add libraries
if(UNIX)
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "amd64")
    set(arch_galaxylib "x86_64")
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86" OR arch STREQUAL "i386")
    set(arch_galaxylib "x86")
  else()
    message(STATUS "Unsupported architecture")
    set(GalaxySDK_FOUND FALSE)
    return()
  endif()

  find_path(
    GalaxySDK_INCLUDE_DIR
    NAMES GxIAPI.h DxImageProc.h
    PATHS "${GalaxySDK_Path}/inc"
    NO_DEFAULT_PATH
  )
  find_library(
    GalaxySDK_LIB
    NAMES "libgxiapi.so"
    PATHS "${GalaxySDK_Path}/lib/${arch_galaxylib}"
    NO_DEFAULT_PATH
  )

  if(NOT TARGET galaxysdk)
    add_library(galaxysdk SHARED IMPORTED)
    set_target_properties(galaxysdk PROPERTIES
      IMPORTED_LOCATION "${GalaxySDK_LIB}"
      INTERFACE_INCLUDE_DIRECTORIES "${GalaxySDK_INCLUDE_DIR}"
    )
  endif()
elseif(WIN32)
  find_path(
    GalaxySDK_INCLUDE_DIR
    NAMES GxIAPI.h DxImageProc.h
    PATHS "${GalaxySDK_Path}/Development/VC SDK/inc"
    NO_DEFAULT_PATH
  )
  find_library(
    GalaxySDK_LIB
    NAMES GxIAPI.lib DxImageProc.lib
    PATHS "${GalaxySDK_Path}/Development/VC SDK/lib/x${sys_type}"
    NO_DEFAULT_PATH
  )
  find_file(
    GalaxySDK_DLL
    NAMES GxIAPI.dll DxImageProc.dll
    PATHS "${GalaxySDK_Path}/APIDll/Win${sys_type}"
    NO_DEFAULT_PATH
  )

  if(NOT TARGET galaxysdk)
    add_library(galaxysdk SHARED IMPORTED GLOBAL)
    set_target_properties(galaxysdk PROPERTIES
      IMPORTED_IMPLIB "${GalaxySDK_LIB}"
      IMPORTED_LOCATION "${GalaxySDK_DLL}"
      INTERFACE_INCLUDE_DIRECTORIES "${GalaxySDK_INCLUDE_DIR}"
    )
  endif()
endif()

mark_as_advanced(GalaxySDK_INCLUDE_DIR GalaxySDK_LIB GalaxySDK_DLL)

set(GalaxySDK_LIBS "galaxysdk")
set(GalaxySDK_INCLUDE_DIRS "${GalaxySDK_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  GalaxySDK
  REQUIRED_VARS GalaxySDK_LIB GalaxySDK_INCLUDE_DIR
)