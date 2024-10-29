if(NOT DEFINED galaxy_sdk_path)
  set(galaxy_sdk_path "/opt/Galaxy_camera")
endif()

# add the include directories path
find_path(
  GalaxySDK_INCLUDE_DIR
  NAMES GxIAPI.h DxImageProc.h
  PATHS "${galaxy_sdk_path}/inc"
  NO_DEFAULT_PATH
)

# add libraries
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "amd64")
  set(arch_galaxylib "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86" OR arch STREQUAL "i386")
  set(arch_galaxylib "x86")
else()
  message(STATUS "Unsupported architecture")
  set(GalaxySDK_FOUND FALSE)
  return()
endif()

find_library(
  GalaxySDK_LIB
  NAMES "libgxiapi.so"
  PATHS "${galaxy_sdk_path}/lib/${arch_galaxylib}"
  NO_DEFAULT_PATH
)

if(NOT TARGET galaxysdk)
  add_library(galaxysdk SHARED IMPORTED)
  set_target_properties(galaxysdk PROPERTIES
    IMPORTED_LOCATION "${GalaxySDK_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${GalaxySDK_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(GalaxySDK_INCLUDE_DIR GalaxySDK_LIB)

set(GalaxySDK_LIBS "galaxysdk")
set(GalaxySDK_INCLUDE_DIRS "${GalaxySDK_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  GalaxySDK
  REQUIRED_VARS GalaxySDK_LIB GalaxySDK_INCLUDE_DIR
)