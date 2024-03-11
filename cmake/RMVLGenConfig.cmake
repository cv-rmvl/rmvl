# --------------------------------------------------------------------------------------------
#  Installation for CMake Module:  RMVLConfig.cmake
#  Part 1/2: Generate RMVLConfig.cmake
#  Part 2/2: Make install
# --------------------------------------------------------------------------------------------

set(RMVL_MODULES_CONFIGCMAKE ${RMVL_MODULES_BUILD})

# --------------------------------------------------------------------------------------------
#  Part 1/2: Generate RMVLConfig.cmake
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
    ${CMAKE_INSTALL_PREFIX}/${RMVL_INCLUDE_INSTALL_PATH}
  )
endforeach(m ${RMVL_INCLUDE_DIRS})

# --------------------------------------------------------------------------------------------
#  Part 2/2: Make install
# --------------------------------------------------------------------------------------------
file(RELATIVE_PATH RMVL_INSTALL_PATH_RELATIVE_CONFIGCMAKE 
  "${CMAKE_INSTALL_PREFIX}/${RMVL_CONFIG_INSTALL_PATH}/" ${CMAKE_INSTALL_PREFIX})
if (IS_ABSOLUTE ${RMVL_INCLUDE_INSTALL_PATH})
  set(RMVL_INCLUDE_DIRS_CONFIGCMAKE "\"${RMVL_INCLUDE_INSTALL_PATH}\"")
else()
  set(RMVL_INCLUDE_DIRS_CONFIGCMAKE "\"\${RMVL_INSTALL_PATH}/${RMVL_INCLUDE_INSTALL_PATH}\"")
endif()

set(cmake_dir "${CMAKE_SOURCE_DIR}/cmake")
set(config_dir "${CMAKE_BINARY_DIR}/config-install")
set(template_dir "${cmake_dir}/templates")

set(3RD_PKGS_CMAKEDEF)

# ----------------------------------------------------------------------------
#   说明:
#   检查 RMVL 模块需要的目标或者包是否存在，不存在则 find_package，并添加至
#   3RD_PKGS_CMAKEDEF 中
#
#   示例:
#   _find_3rd_package(
#       tag_detector    # RMVL 模块名
#       TARGET apriltag # 需要检查的目标（若指定 FOUND 则表示是否已经找到该包）
#       apriltag        # TARGET 模式下，需要 find_package 的包名
#   )
# ----------------------------------------------------------------------------
macro(_find_3rd_package)
  set(3RD_PKGS_CMAKEDEF "${3RD_PKGS_CMAKEDEF}# Find ${ARGV2} for \"rmvl_${ARGV0}\"\n")
  set(3RD_PKGS_CMAKEDEF "${3RD_PKGS_CMAKEDEF}if(TARGET rmvl_${ARGV0})\n")
  if("${ARGV1}" STREQUAL "TARGET")
    set(3RD_PKGS_CMAKEDEF "${3RD_PKGS_CMAKEDEF}  if(NOT TARGET ${ARGV2})\n")
    set(3RD_PKGS_CMAKEDEF "${3RD_PKGS_CMAKEDEF}    find_package(${ARGV3} REQUIRED)\n")
  elseif("${ARGV1}" STREQUAL "FOUND")
    set(3RD_PKGS_CMAKEDEF "${3RD_PKGS_CMAKEDEF}  if(NOT ${ARGV2}_FOUND)\n")
    set(3RD_PKGS_CMAKEDEF "${3RD_PKGS_CMAKEDEF}    find_package(${ARGV2} REQUIRED)\n")
  else()
    message(FATAL_ERROR "Unknown type of 3rd party package: ${ARGV1}")
  endif()
  set(3RD_PKGS_CMAKEDEF "${3RD_PKGS_CMAKEDEF}  endif()\n")
  set(3RD_PKGS_CMAKEDEF "${3RD_PKGS_CMAKEDEF}endif()\n\n")
endmacro()

_find_3rd_package(core FOUND OpenCV)
_find_3rd_package(tag_detector TARGET apriltag apriltag)
_find_3rd_package(opcua TARGET open62541::open62541 open62541)

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

# genpara
install(
  FILES "${cmake_dir}/templates/para_generator_header.in"
        "${cmake_dir}/templates/para_generator_module.in"
        "${cmake_dir}/templates/para_generator_source.in"
  DESTINATION "${RMVL_CONFIG_INSTALL_PATH}/templates"
)
