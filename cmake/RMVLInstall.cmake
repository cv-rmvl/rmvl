# --------------------------------------------------------------------------------------------
#  Default install prefix
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
#  Obtain architecture and runtime information
# --------------------------------------------------------------------------------------------
if(DEFINED RMVL_ARCH_forWin AND DEFINED RMVL_RUNTIME_forWin)
  # custom overridden values
elseif(MSVC)
  # see Modules/CMakeGenericSystem.cmake
  if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
    set(RMVL_ARCH_forWin "x64")
  elseif("${CMAKE_GENERATOR_PLATFORM}" MATCHES "ARM64")
    set(RMVL_ARCH_forWin "ARM64")
  elseif("${CMAKE_GENERATOR}" MATCHES "ARM")
    set(RMVL_ARCH_forWin "ARM")
  elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(RMVL_ARCH_forWin "x64")
  else()
    set(RMVL_ARCH_forWin x86)
  endif()

  if(MSVC_VERSION EQUAL 1400)
    set(RMVL_RUNTIME_forWin vc8)
  elseif(MSVC_VERSION EQUAL 1500)
    set(RMVL_RUNTIME_forWin vc9)
  elseif(MSVC_VERSION EQUAL 1600)
    set(RMVL_RUNTIME_forWin vc10)
  elseif(MSVC_VERSION EQUAL 1700)
    set(RMVL_RUNTIME_forWin vc11)
  elseif(MSVC_VERSION EQUAL 1800)
    set(RMVL_RUNTIME_forWin vc12)
  elseif(MSVC_VERSION EQUAL 1900)
    set(RMVL_RUNTIME_forWin vc14)
  elseif(MSVC_VERSION MATCHES "^191[0-9]$")
    set(RMVL_RUNTIME_forWin vc15)
  elseif(MSVC_VERSION MATCHES "^192[0-9]$")
    set(RMVL_RUNTIME_forWin vc16)
  elseif(MSVC_VERSION MATCHES "^19[34][0-9]$")
    set(RMVL_RUNTIME_forWin vc17)
  else()
    message(WARNING "RMVL does not recognize MSVC_VERSION \"${MSVC_VERSION}\". Cannot set RMVL_RUNTIME_forWin")
  endif()
elseif(MINGW)
  set(RMVL_RUNTIME_forWin mingw)

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*")
    set(RMVL_ARCH_forWin x64)
  else()
    set(RMVL_ARCH_forWin x86)
  endif()
endif()

# --------------------------------------------------------------------------------------------
#  Initial install layout
# --------------------------------------------------------------------------------------------
if(WIN32 AND CMAKE_HOST_SYSTEM_NAME MATCHES Windows)
  if(DEFINED RMVL_RUNTIME_forWin AND DEFINED RMVL_ARCH_forWin)
    set(RMVL_INSTALL_BINARIES_PREFIX "${RMVL_ARCH_forWin}/${RMVL_RUNTIME_forWin}")
  else()
    message(STATUS "Can't detect runtime and/or arch")
    set(RMVL_INSTALL_BINARIES_PREFIX "")
  endif()

  if(NOT BUILD_SHARED_LIBS)
    set(RMVL_INSTALL_BINARIES_SUFFIX "staticlib")
  else()
    set(RMVL_INSTALL_BINARIES_SUFFIX "lib")
  endif()

  set(RMVL_BIN_INSTALL_PATH         "${RMVL_INSTALL_BINARIES_PREFIX}/bin")
  set(RMVL_TEST_INSTALL_PATH        "${RMVL_BIN_INSTALL_PATH}")
  set(RMVL_SAMPLES_BIN_INSTALL_PATH "${RMVL_INSTALL_BINARIES_PREFIX}/samples")
  set(RMVL_LIB_INSTALL_PATH         "${RMVL_INSTALL_BINARIES_PREFIX}/${RMVL_INSTALL_BINARIES_SUFFIX}")
  set(RMVL_3P_LIB_INSTALL_PATH      "${RMVL_INSTALL_BINARIES_PREFIX}/staticlib")
  set(RMVL_CONFIG_INSTALL_PATH      "${RMVL_LIB_INSTALL_PATH}")
  set(RMVL_INCLUDE_INSTALL_PATH     "include")
  set(RMVL_DOC_INSTALL_PATH         "doc")
else() # UNIX
  include(GNUInstallDirs)
  set(RMVL_BIN_INSTALL_PATH     "bin")
  set(RMVL_TEST_INSTALL_PATH    "${RMVL_BIN_INSTALL_PATH}")
  set(RMVL_LIB_INSTALL_PATH     "${CMAKE_INSTALL_LIBDIR}")
  set(RMVL_3P_LIB_INSTALL_PATH  "${RMVL_LIB_INSTALL_PATH}/${PROJECT_NAME}/3rdparty")
  set(RMVL_CONFIG_INSTALL_PATH  "${RMVL_LIB_INSTALL_PATH}/cmake/${PROJECT_NAME}")
  set(RMVL_INCLUDE_INSTALL_PATH "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
  set(RMVL_DOC_INSTALL_PATH     "${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME}")
endif()

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${RMVL_LIB_INSTALL_PATH}")
