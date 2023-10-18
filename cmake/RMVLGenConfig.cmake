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
  DESTINATION "${RMVL_CONFIG_INSTALL_PATH}"
)
