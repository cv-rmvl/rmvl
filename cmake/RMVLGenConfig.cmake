# --------------------------------------------------------------------------------------------
#  Installation for CMake Module:  RMVLConfig.cmake
#  Part 1/3: Prepare module list
#  Part 2/3: Generate RMVLConfig.cmake
#  Part 3/3: Make install
# --------------------------------------------------------------------------------------------

set(cmake_dir "${CMAKE_SOURCE_DIR}/cmake")
set(config_dir "${CMAKE_BINARY_DIR}/config-install")
set(template_dir "${cmake_dir}/templates")

# --------------------------------------------------------------------------------------------
#  Part 1/3: Prepare module list
# --------------------------------------------------------------------------------------------
set(RMVL_MODULES_CONFIGCMAKE ${RMVL_MODULES_BUILD})

set(RMVL_MODULES_IMPORTED_CONFIGCMAKE "")
# --------------------------------------------------------------------
#  Usage:
#   find the imported modules for RMVLConfig.cmake.
#
#  Note:
#   These targets are typically found using the file 'FindXxx.cmake'
#
#  Example:
#   __find_imported_modules(
#     mvsdk   # target name
#     MvSDK   # package name
#   )
# --------------------------------------------------------------------
function(__find_imported_modules target pkg_name)
  set(PKG_MODULE_CONFIGCMAKE ${target})
  set(PKG_LOCATION_CONFIGCMAKE ${${pkg_name}_LIB})
  set(PKG_INCLUDE_DIRS_CONFIGCMAKE ${${pkg_name}_INCLUDE_DIR})
  if(NOT PKG_LOCATION_CONFIGCMAKE OR NOT PKG_INCLUDE_DIRS_CONFIGCMAKE)
    return()
  endif()
  rmvl_cmake_configure("${template_dir}/RMVLConfig-IMPORTED.cmake.in" MODULE_CONFIGCMAKE @ONLY)
  set(RMVL_MODULES_IMPORTED_CONFIGCMAKE "${RMVL_MODULES_IMPORTED_CONFIGCMAKE}${MODULE_CONFIGCMAKE}\n" PARENT_SCOPE)
endfunction()

__find_imported_modules(mvsdk MvSDK)
__find_imported_modules(hiksdk HikSDK)
__find_imported_modules(optcamsdk OPTCameraSDK)
__find_imported_modules(galaxysdk GalaxySDK)
__find_imported_modules(optlc OPTLightCtrl)

set(RMVL_3RD_PKGS_CONFIGCMAKE "")
# --------------------------------------------------------------------
#  Usage:
#   Check whether the target or package required by the RMVL module
#   exists. otherwise 'find_package' will be added to RMVL_3RD_PKGS_CONFIGCMAKE
#   
#  Example:
#   __find_3rd_package_if(
#     flag            # whether the package is required, if not, return
#     TARGET apriltag # the target to be checked, 'FOUND' means
#                       whether the package has been found
#     apriltag        # the package name to be found in TARGET mode
#   )
# --------------------------------------------------------------------
function(__find_3rd_package_if flag)
  if(NOT ${flag})
    return()
  endif()
  if("${ARGV1}" STREQUAL "TARGET")
    set(RMVL_3RD_PKGS_CONFIGCMAKE "${RMVL_3RD_PKGS_CONFIGCMAKE}if(NOT TARGET ${ARGV2})\n")
    set(RMVL_3RD_PKGS_CONFIGCMAKE "${RMVL_3RD_PKGS_CONFIGCMAKE}  find_package(${ARGV3} REQUIRED)\n")
  elseif("${ARGV1}" STREQUAL "FOUND")
    set(RMVL_3RD_PKGS_CONFIGCMAKE "${RMVL_3RD_PKGS_CONFIGCMAKE}if(NOT ${ARGV2}_FOUND)\n")
    set(RMVL_3RD_PKGS_CONFIGCMAKE "${RMVL_3RD_PKGS_CONFIGCMAKE}  find_package(${ARGV2} REQUIRED)\n")
  else()
    message(FATAL_ERROR "Unknown type of 3rd party package: ${ARGV1}")
  endif()
  set(RMVL_3RD_PKGS_CONFIGCMAKE "${RMVL_3RD_PKGS_CONFIGCMAKE}endif()\n\n" PARENT_SCOPE)
endfunction()

__find_3rd_package_if(WITH_OPENCV FOUND OpenCV)
__find_3rd_package_if(WITH_APRILTAG TARGET apriltag apriltag)
__find_3rd_package_if(WITH_OPEN62541 TARGET open62541::open62541 open62541)

# --------------------------------------------------------------------------------------------
#  Part 2/3: Generate RMVLConfig.cmake
# --------------------------------------------------------------------------------------------
set(RMVL_INCLUDE_DIRS "")
foreach(m ${RMVL_MODULES_BUILD})
  if(EXISTS "${RMVL_MODULE_${m}_LOCATION}/include")
    list(APPEND RMVL_INCLUDE_DIRS "${RMVL_MODULE_${m}_LOCATION}/include")
  endif()
endforeach(m ${RMVL_MODULES_BUILD})
list(REMOVE_DUPLICATES RMVL_INCLUDE_DIRS)
# install include directories
foreach(m ${RMVL_INCLUDE_DIRS})
  rmvl_install_directories(
    ${m} DESTINATION
    ${RMVL_INCLUDE_INSTALL_PATH}
  )
endforeach(m ${RMVL_INCLUDE_DIRS})

# --------------------------------------------------------------------------------------------
#  Part 3/3: Generate install target
# --------------------------------------------------------------------------------------------
file(RELATIVE_PATH RMVL_INSTALL_PATH_RELATIVE_CONFIGCMAKE 
  "${CMAKE_INSTALL_PREFIX}/${RMVL_CONFIG_INSTALL_PATH}/" ${CMAKE_INSTALL_PREFIX})
if (IS_ABSOLUTE ${RMVL_INCLUDE_INSTALL_PATH})
  set(RMVL_INCLUDE_DIRS_CONFIGCMAKE "\"${RMVL_INCLUDE_INSTALL_PATH}\"")
else()
  set(RMVL_INCLUDE_DIRS_CONFIGCMAKE "\"\${RMVL_INSTALL_PATH}/${RMVL_INCLUDE_INSTALL_PATH}\"")
endif()

configure_file("${template_dir}/RMVLConfig.cmake.in" "${config_dir}/RMVLConfig.cmake" @ONLY)
configure_file("${template_dir}/RMVLConfig-version.cmake.in" "${config_dir}/RMVLConfig-version.cmake" @ONLY)

install(
  EXPORT RMVLModules
  FILE RMVLModules.cmake
  DESTINATION "${RMVL_CONFIG_INSTALL_PATH}"
)

install(
  FILES "${config_dir}/RMVLConfig.cmake"
        "${config_dir}/RMVLConfig-version.cmake"
        "${cmake_dir}/RMVLModule.cmake"
        "${cmake_dir}/RMVLUtils.cmake"
        "${cmake_dir}/RMVLGenPara.cmake"
  DESTINATION "${RMVL_CONFIG_INSTALL_PATH}"
)

# gen para
install(
  FILES "${cmake_dir}/templates/para_generator_header.in"
        "${cmake_dir}/templates/para_generator_module.in"
        "${cmake_dir}/templates/para_generator_source.in"
        "${cmake_dir}/templates/para_generator_header_without_cv.in"
  DESTINATION "${RMVL_CONFIG_INSTALL_PATH}/templates"
)

# install file: __init__.py and rmvl_typing.pyi of 'rm' module
if(BUILD_PYTHON)
  install(
    FILES "${RMVL_PYTHON_OUTPUT_DIR}/__init__.py"
          "${RMVL_PYTHON_OUTPUT_DIR}/rmvl_typing.pyi"
    DESTINATION "${RMVL_PYTHON_INSTALL_PATH}"
  )
endif()
