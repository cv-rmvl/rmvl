# ----------------------------------------------------------------------------
#  Root CMake file for RMVL
#
#    From the off-tree build directory, invoke:
#      $ cmake <PATH_TO_RMVL_ROOT>
#
# ----------------------------------------------------------------------------

# Disable in-source builds to prevent source tree corruption.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(
    FATAL_ERROR 
    "FATAL: In-source builds are not allowed.
    You should create a separate directory for build files."
  )
endif()

# ----------------------------------------------------------------------------
#  Configure CMake policies
# ----------------------------------------------------------------------------
if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW) # option() honors normal variables.
endif()
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW) # CMake 3.24: URL download, extract file and timestamp
endif()

cmake_minimum_required(VERSION 3.16)

project(
  RMVL
  LANGUAGES CXX C
)

# CMake module named xxx.cmake PATH
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# ----------------------------------------------------------------------------
#  RMVL utilities macro and function
# ----------------------------------------------------------------------------
include(RMVLUtils)

# Build Type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# ----------------------------------------------------------------------------
#  Build
# ----------------------------------------------------------------------------
# RMVL Install layout 
include(RMVLInstall)
include(RMVLModule)
include(RMVLExtraTarget)

include_directories(${CMAKE_BINARY_DIR})

# ----------------------------------------------------------------------------
#  RMVL compile configure and options
# ----------------------------------------------------------------------------
include(RMVLCompilerOptions)

# ----------------------------------------------------------------------------
#  Get actual RMVL version number from sources
# ----------------------------------------------------------------------------
include(RMVLVersion)

# ----------------------------------------------------------------------------
#  RMVL parameters and miscellaneous generation
# ----------------------------------------------------------------------------
set(para_template_path "${CMAKE_SOURCE_DIR}/cmake/templates" CACHE STRING "GenPara template path" FORCE)
include(RMVLGenPara)

# ----------------------------------------------------------------------------
#  Process subdirectories
# ----------------------------------------------------------------------------
# rmvl.hpp and legacy headers
add_subdirectory(include)
# Main modules
add_subdirectory(modules)
# RMVL extra modules containing 4 data components and 4 function modules
if(BUILD_EXTRA)
  add_subdirectory(extra)
endif()

# ----------------------------------------------------------------------------
#  RMVL Examples
# ----------------------------------------------------------------------------
if(BUILD_EXAMPLES)
  add_subdirectory(${PROJECT_SOURCE_DIR}/samples)
endif(BUILD_EXAMPLES)

# ----------------------------------------------------------------------------
#  Documents generation
# ----------------------------------------------------------------------------
if(BUILD_DOCS)
  find_package(Doxygen)
    if(DOXYGEN_FOUND)
    add_custom_target(
      doxygen
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
      COMMENT "Generate Doxygen documentation"
    )
  endif()
  add_subdirectory(${PROJECT_SOURCE_DIR}/doc)# Doxygen documents and other file
endif()

# ----------------------------------------------------------------------------
#  Finalization: generate configuration-based files
# ----------------------------------------------------------------------------
include(RMVLGenHeaders)

include(RMVLGenConfig)

# ----------------------------------------------------------------------------
#  Summary:
# ----------------------------------------------------------------------------
status("")
status("------------- General configuration for RMVL (${RMVL_VERSION}) -------------")
status("  Platform:")
status("     Host:"              ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION} ${CMAKE_HOST_SYSTEM_PROCESSOR})
status("     CMake:"             ${CMAKE_VERSION})
status("     CMake generator:"   ${CMAKE_GENERATOR})
status("     CMake build tool:"  ${CMAKE_BUILD_TOOL})
if(MSVC)
  status("     MSVC:" ${MSVC_VERSION})
endif()
if(CMAKE_GENERATOR MATCHES Xcode)
  status("     Xcode:" ${XCODE_VERSION})
endif()
if(CMAKE_GENERATOR MATCHES "Xcode|Visual Studio|Multi-Config")
  status("     Configuration:" ${CMAKE_CONFIGURATION_TYPES})
else()
  status("     Configuration:" ${CMAKE_BUILD_TYPE})
endif()
# ========================== C/C++ options ==========================
if(CMAKE_CXX_COMPILER_VERSION)
  set(RMVL_COMPILER_STR "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} (ver ${CMAKE_CXX_COMPILER_VERSION})")
else()
  set(RMVL_COMPILER_STR "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}")
endif()
string(STRIP "${RMVL_COMPILER_STR}" RMVL_COMPILER_STR)

status("")
status("  C/C++:")
status("     Built as dynamic libs?:" BUILD_SHARED_LIBS THEN YES ELSE NO)
if(DEFINED CMAKE_CXX_STANDARD AND CMAKE_CXX_STANDARD)
  status("     C++ standard:"           "${CMAKE_CXX_STANDARD}")
endif()
status("     C++ Compiler:"        ${RMVL_COMPILER_STR})
status("     C++ flags (Release):" ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE})
status("     C++ flags (Debug):"   ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG})
status("     C Compiler:"          ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1})
status("     C flags (Release):"   ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE})
status("     C flags (Debug):"     ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG})
if(WIN32)
  status("     Linker flags (Release):" ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_RELEASE})
  status("     Linker flags (Debug):"   ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
else()
  status("     Linker flags (Release):" ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
  status("     Linker flags (Debug):"   ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
endif()
status("     ccache:"              ENABLE_CCACHE AND CCACHE_FOUND THEN YES ELSE NO)

# =================== Other third-party libraries ===================
find_package(OpenCV QUIET)
find_package(Eigen3 QUIET)
status("")
status("  Other third-party libraries:")
status("     OpenCV: (*)"           OpenCV_FOUND THEN "YES (ver ${OpenCV_VERSION})" ELSE NO)
status("     Eigen library: (*)"    Eigen3_FOUND THEN "YES (ver ${EIGEN3_VERSION_STRING})" ELSE NO)
status("     onnxruntime:"          WITH_ONNXRUNTIME THEN YES ELSE NO)
status("     apriltag:"             WITH_APRILTAG THEN YES ELSE NO)
status("     open62541:"            WITH_OPEN62541 THEN YES ELSE NO)

# =============================== SDK ===============================
status("")
status("  Hardware SDK:")
status("     Camera:")
status("        MindVision:"        BUILD_rmvl_mv_video_capture THEN YES ELSE NO)
status("        HikRobot:"          BUILD_rmvl_hik_video_capture THEN YES ELSE NO)
status("        OPT:"               BUILD_rmvl_opt_video_capture THEN YES ELSE NO)
status("     Light controller:")
status("        OPT:"               BUILD_rmvl_opt_light_control THEN YES ELSE NO)

# ===================== Inference engine in RMVL ====================
status("")
status("  Inference engine in RMVL:")
status("     Ort:"                  BUILD_rmvl_ort THEN YES ELSE NO)

# ========================== RMVL modules ===========================
string(REGEX REPLACE "rmvl_" " " modules_public_dir ${RMVL_MODULES_PUBLIC})
string(REGEX REPLACE "rmvl_" " " modules_interface_dir ${RMVL_MODULES_INTERFACE})
status("")
status("  RMVL modules:")
status("     public modules:"       ${modules_public_dir})
status("     interface modules:"    ${modules_interface_dir})

# ======================== Test and examples ========================
status("")
status("  Test and examples:")
status("     Unit tests:"           BUILD_TESTS THEN YES ELSE NO)
status("     Performance tests:"    BUILD_PERF_TESTS THEN YES ELSE NO)
status("     RMVL examples:"        BUILD_EXAMPLES THEN YES ELSE NO)

status("")
status("  Install to:"              "${CMAKE_INSTALL_PREFIX}")
status("----------------------------------------------------------------------")
status("")

rmvl_finalize_status()
