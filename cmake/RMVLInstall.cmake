# --------------------------------------------------------------------------------------------
#  默认安装路径前缀
#  https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT.html
#  https://juejin.cn/post/6942734287351316487
# --------------------------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(NOT CMAKE_TOOLCHAIN_FILE)
    if(WIN32)
      set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation Directory" FORCE)
    else()
      set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation Directory" FORCE)
    endif()
  else()
    # any cross-compiling
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation Directory" FORCE)
  endif()
endif()

# --------------------------------------------------------------------------------------------
#  Initial install layout
# --------------------------------------------------------------------------------------------
# Unix
include(GNUInstallDirs)
set(RMVL_BIN_INSTALL_PATH     "bin")
set(RMVL_TEST_INSTALL_PATH    "${RMVL_BIN_INSTALL_PATH}")
set(RMVL_LIB_INSTALL_PATH     "${CMAKE_INSTALL_LIBDIR}")
set(RMVL_3P_LIB_INSTALL_PATH  "${RMVL_LIB_INSTALL_PATH}/${PROJECT_NAME}/3rdparty")
set(RMVL_CONFIG_INSTALL_PATH  "${RMVL_LIB_INSTALL_PATH}/cmake/${PROJECT_NAME}")
set(RMVL_INCLUDE_INSTALL_PATH "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${RMVL_LIB_INSTALL_PATH}")
