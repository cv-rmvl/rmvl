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
#   find the imported modules for RMVLConfig.cmake, including some 3rd-
#   party libraries
#
#  Note:
#   These targets are typically found using the file 'FindXxx.cmake' or
#   created by the 'CMakeLists.txt' in '3rdparty' directory.
#
#  Example:
#   __find_imported_modules(
#     mvsdk   # target name
#     MvSDK   # package name
#   )
# --------------------------------------------------------------------
function(__find_imported_modules target pkg_name)
  set(PKG_MODULE_CONFIGCMAKE ${target})
  # get file name excluding path
  if(${pkg_name}_LIB)
    get_filename_component(pkg_lib_name ${${pkg_name}_LIB} NAME)
  else()
    return()
  endif()
  if(${pkg_name}_DLL)
    get_filename_component(pkg_dll_name ${${pkg_name}_DLL} NAME)
  endif()
  # for Windows
  if(pkg_dll_name)
    set(PKG_WINIMPLIB_CONFIGCMAKE "\$\{RMVL_INSTALL_PATH\}/${RMVL_3P_LIB_INSTALL_PATH}/${pkg_lib_name}")
    set(PKG_WINLOCATION_CONFIGCMAKE "\$\{RMVL_INSTALL_PATH\}/${RMVL_BIN_INSTALL_PATH}/${pkg_dll_name}")
  else()
    set(PKG_WINLOCATION_CONFIGCMAKE "\$\{RMVL_INSTALL_PATH\}/${RMVL_3P_LIB_INSTALL_PATH}/${pkg_lib_name}")
  endif()
  # for UNIX
  set(PKG_LOCATION_CONFIGCMAKE "\$\{RMVL_INSTALL_PATH\}/${RMVL_3P_LIB_INSTALL_PATH}/${pkg_lib_name}")
  
  set(PKG_INCLUDE_CONFIGCMAKE "\$\{RMVL_INSTALL_PATH\}/${RMVL_INCLUDE_INSTALL_PATH}")
  rmvl_cmake_configure("${template_dir}/RMVLConfig-IMPORTED.cmake.in" MODULE_CONFIGCMAKE @ONLY)
  set(RMVL_MODULES_IMPORTED_CONFIGCMAKE "${RMVL_MODULES_IMPORTED_CONFIGCMAKE}${MODULE_CONFIGCMAKE}\n" PARENT_SCOPE)
endfunction()

# SDK for hardware, they are *.so for Linux and *.dll (including import library *.lib) for Windows
__find_imported_modules(mvsdk MvSDK)
__find_imported_modules(hiksdk HikSDK)
__find_imported_modules(optcamsdk OPTCameraSDK)
__find_imported_modules(galaxysdk GalaxySDK)
__find_imported_modules(optlc OPTLightCtrl)

# 3rdparty (local source code)
# all targets have been configured in the corresponding CMakeLists.txt, no need to configure again

# 3rdparty (download)
if(WITH_OPEN62541)
  list(APPEND RMVL_MODULES_3RD_DOWNLOAD_CONFIGCMAKE "if(TARGET open62541::open62541)")
  if(open62541_IN_3RD)
    list(APPEND RMVL_MODULES_3RD_DOWNLOAD_CONFIGCMAKE "  include(\$\{RMVL_INSTALL_PATH\}/lib/cmake/open62541/open62541Config.cmake)")
  else()
    list(APPEND RMVL_MODULES_3RD_DOWNLOAD_CONFIGCMAKE "  find_package(open62541 REQUIRED)")
  endif()
  list(APPEND RMVL_MODULES_3RD_DOWNLOAD_CONFIGCMAKE "endif()")
endif()

string(REPLACE ";" "\n" RMVL_MODULES_3RD_DOWNLOAD_CONFIGCMAKE "${RMVL_MODULES_3RD_DOWNLOAD_CONFIGCMAKE}")

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
