# --------------------------------------------------------------------------------------------
#  Installation for CMake Module:  RMVLConfig.cmake
#  Part 1/3: Prepare module list
#  Part 2/3: Generate RMVLConfig.cmake
#  Part 3/3: Generate install target
# --------------------------------------------------------------------------------------------

if(BUILD_WORLD)
  rmvl_add_world()
endif()

set(cmake_dir "${CMAKE_SOURCE_DIR}/cmake")
set(config_dir "${CMAKE_BINARY_DIR}/config-install")
set(template_dir "${cmake_dir}/templates")

# --------------------------------------------------------------------------------------------
#  Part 1/3: Prepare module list
# --------------------------------------------------------------------------------------------
if(BUILD_WORLD)
  set(RMVL_MODULES_CONFIGCMAKE world)
else()
  set(RMVL_MODULES_CONFIGCMAKE ${RMVL_MODULES_BUILD})
endif()

set(RMVL_MODULES_IMPORTED_CONFIGCMAKE "")

# SDK for hardware and software, they are *.so for Linux and *.dll (including import library *.lib) for Windows
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

__find_imported_modules(mvsdk MvSDK)
__find_imported_modules(hiksdk HikSDK)
__find_imported_modules(optcamsdk OPTCameraSDK)
__find_imported_modules(galaxysdk GalaxySDK)
__find_imported_modules(optlc OPTLightCtrl)
__find_imported_modules(onnxruntime Ort)

# 3rdparty
function(__find_3rd_modules pkg_name)
  cmake_parse_arguments(3RD "" "CFG_SUFFIX" "TARGETS" ${ARGN})
  # preprocess
  set(configcmake_str "# ${pkg_name}")
  string(TOUPPER "${pkg_name}" pkg_name_upper)
  string(REPLACE ";" " AND NOT TARGET " judge_statement "NOT TARGET ${3RD_TARGETS}")
  # update info
  if(WITH_${pkg_name_upper})
    list(APPEND configcmake_str "if(${judge_statement})")
    if(${pkg_name}_IN_3RD)
      list(APPEND configcmake_str "  include(\$\{RMVL_INSTALL_PATH\}/${3RD_CFG_SUFFIX})")
    else()
      list(APPEND configcmake_str "  find_package(${pkg_name} REQUIRED)")
    endif()
    list(APPEND configcmake_str "endif()")
  endif()
  # update 'RMVL_MODULES_3RD_CONFIGCMAKE'
  string(REPLACE ";" "\n" configcmake_str "${configcmake_str}")
  set(RMVL_MODULES_3RD_CONFIGCMAKE "${RMVL_MODULES_3RD_CONFIGCMAKE}${configcmake_str}\n\n" PARENT_SCOPE)
endfunction()

# ------ local 3rdparty ------
set(RMVL_MODULES_3RD_CONFIGCMAKE "# ------ 3rdparty local -------\n")
# apriltag
__find_3rd_modules(apriltag
  CFG_SUFFIX "${RMVL_CONFIG_INSTALL_PATH}/apriltagTargets.cmake"
  TARGETS apriltag
)

# nlohmann_json
__find_3rd_modules(nlohmann_json
  CFG_SUFFIX "${RMVL_CONFIG_INSTALL_PATH}/nlohmann_jsonTargets.cmake"
  TARGETS nlohmann_json::nlohmann_json
)

# --------- download ---------
set(RMVL_MODULES_3RD_CONFIGCMAKE "${RMVL_MODULES_3RD_CONFIGCMAKE}# ----- 3rdparty download -----\n")
# open62541
__find_3rd_modules(open62541
  CFG_SUFFIX "lib/cmake/open62541/open62541Config.cmake"
  TARGETS open62541::open62541
)

# Eigen3
__find_3rd_modules(Eigen3
  CFG_SUFFIX "share/eigen3/cmake/Eigen3Config.cmake"
  TARGETS Eigen3::Eigen
)

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
        "${cmake_dir}/RMVLCodeGenerate.cmake"
  DESTINATION "${RMVL_CONFIG_INSTALL_PATH}"
)

# code generator templates
install(FILES
  # *.para
  "${cmake_dir}/templates/para_generator_header.in"
  "${cmake_dir}/templates/para_generator_module.in"
  "${cmake_dir}/templates/para_generator_source.in"
  "${cmake_dir}/templates/para_generator_header_without_cv.in"
  # *.msg
  "${cmake_dir}/templates/msg_generator.cpp.in"
  "${cmake_dir}/templates/msg_generator.hpp.in"
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
