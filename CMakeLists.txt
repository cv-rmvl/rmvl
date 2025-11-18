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

# Build Type
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
if(NOT MSVC)
  if(NOT DEFINED CMAKE_BUILD_TYPE)
    message(STATUS "'Release' build type is used by default. Use CMAKE_BUILD_TYPE to specify build type (Release or Debug)")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build")
  endif()
  if(DEFINED CMAKE_BUILD_TYPE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${CMAKE_CONFIGURATION_TYPES}")
  endif()
endif()

project(
  RMVL
  DESCRIPTION "Robotic Manipulation and Vision Library"
  HOMEPAGE_URL "https://cv-rmvl.github.io"
  LANGUAGES CXX C
)

# ----------------------------------------------------------------------------
#  RMVL docker compiler images
# ----------------------------------------------------------------------------
option(ENABLE_RUNIN "Enable RMVL run_in_<xxx> functions" OFF)
if(ENABLE_RUNIN)
  message(STATUS "RMVL run_in_<xxx> functions enabled")
  include(cmake/RMVLCompilerImages.cmake)
  return()
endif()

# CMake module named xxx.cmake PATH
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# ----------------------------------------------------------------------------
#  RMVL utilities macro and function
# ----------------------------------------------------------------------------
include(cmake/RMVLUtils.cmake)

# ----------------------------------------------------------------------------
#  Include necessary CMake modules
# ----------------------------------------------------------------------------
include_directories(${PROJECT_BINARY_DIR})

# RMVL version number from sources
include(cmake/RMVLVersion.cmake)
# RMVL Install layout 
include(cmake/RMVLInstall.cmake)
# RMVL build module
include(cmake/RMVLModule.cmake)
# RMVL compile configure and options
include(cmake/RMVLCompilerOptions.cmake)
# RMVL extra target
include(cmake/RMVLExtraTarget.cmake)

# ----------------------------------------------------------------------------
#  RMVL parameters and miscellaneous generation
# ----------------------------------------------------------------------------
set(para_template_path "${CMAKE_SOURCE_DIR}/cmake/templates" CACHE INTERNAL "GenPara template path" FORCE)
include(cmake/RMVLGenPara.cmake)

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
  add_subdirectory(${PROJECT_SOURCE_DIR}/doc)# Doxygen documents and other file
endif()

# ----------------------------------------------------------------------------
#  Finalization: generate configuration-based files
# ----------------------------------------------------------------------------
include(cmake/RMVLGenHeaders.cmake)

include(cmake/RMVLGenConfig.cmake)

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
  status("     MSVC:"            ${MSVC_VERSION})
endif()
if(CMAKE_GENERATOR MATCHES Xcode)
  status("     Xcode:"           ${XCODE_VERSION})
endif()
if(CMAKE_GENERATOR MATCHES "Xcode|Visual Studio|Multi-Config")
  status("     Configuration:"   ${CMAKE_CONFIGURATION_TYPES})
else()
  status("     Configuration:"   ${CMAKE_BUILD_TYPE})
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
  status("     C++ standard:"      "${CMAKE_CXX_STANDARD}")
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
status("")
status("  Other third-party libraries:")
status("     OpenCV:"               WITH_OPENCV THEN "YES (ver ${OpenCV_VERSION})" ELSE NO)
status("     Eigen3:"               WITH_EIGEN3 THEN "YES (ver ${EIGEN3_VERSION_STRING})" ELSE NO)
status("     onnxruntime:"          WITH_ONNXRUNTIME THEN "YES (ver ${Ort_VERSION})" ELSE NO)
status("     apriltag:"             WITH_APRILTAG THEN YES ELSE NO)
status("     open62541:"            WITH_OPEN62541 THEN "YES (ver ${open62541_VERSION})" ELSE NO)

# =============================== SDK ===============================
status("")
status("  Hardware SDK:")
status("     Camera:")
status("        MindVision:"        BUILD_rmvl_mv_camera THEN YES ELSE NO)
status("        HikRobot:"          BUILD_rmvl_hik_camera THEN YES ELSE NO)
status("        OPT:"               BUILD_rmvl_opt_camera THEN YES ELSE NO)
status("        Galaxy:"            BUILD_rmvl_galaxy_camera THEN YES ELSE NO)
status("     Light controller:")
status("        OPT:"               BUILD_rmvl_opt_light_control THEN YES ELSE NO)
status("        HikRobot:"          BUILD_rmvl_hik_light_control THEN YES ELSE NO)

# ========================== RMVL modules ===========================
set(RMVL_MAIN_MODULES_BUILD ${RMVL_MODULES_BUILD})
foreach(m ${RMVL_EXTRA_MODULE})
  string(TOUPPER "${m}" mUP)
  list(REMOVE_ITEM RMVL_MAIN_MODULES_BUILD ${RMVL_${mUP}_MODULES_BUILD} rmvl_types)
  if("${m}" STREQUAL "types")
    continue()
  endif()
  if("${RMVL_${mUP}_MODULES_BUILD}" STREQUAL "")
    set(${m}_modules "")
  else()
    string(REGEX REPLACE "rmvl_" " " ${m}_modules ${RMVL_${mUP}_MODULES_BUILD})
  endif()
endforeach()

foreach(m ${RMVL_MAIN_MODULE})
  string(TOUPPER "${m}" mUP)
  if(NOT "${RMVL_${mUP}_MODULES_BUILD}" STREQUAL "")
    list(REMOVE_ITEM RMVL_MAIN_MODULES_BUILD ${RMVL_${mUP}_MODULES_BUILD})
  endif()
  if("${RMVL_${mUP}_MODULES_BUILD}" STREQUAL "")
    set(${m}_modules "")
  else()
    string(REGEX REPLACE "rmvl_" " " ${m}_modules ${RMVL_${mUP}_MODULES_BUILD})
  endif()
endforeach()

string(REGEX REPLACE "rmvl_" " " main_modules ${RMVL_MAIN_MODULES_BUILD})

status("")
status("  RMVL main modules:")
status("     To be built:"          ${main_modules})
foreach(m ${RMVL_MAIN_MODULE})
  if("${${m}_modules}" STREQUAL "")
    continue()
  endif()
  status("        ${m}:"          ${${m}_modules})
endforeach()
status("")
status("  RMVL extra modules:"      BUILD_EXTRA THEN YES ELSE NO)
if(BUILD_EXTRA)
  status("     To be built:")
  foreach(m ${RMVL_EXTRA_MODULE})
    if("${m}" STREQUAL "types")
      continue()
    endif()
    status("        ${m}:"          ${${m}_modules})
  endforeach()
endif()
status("")
status("  Documents (Doxygen):"     BUILD_DOCS THEN "YES (ver ${DOXYGEN_VERSION})" ELSE NO)

# ============================= Python ==============================
status("")
status("  Python 3:"                BUILD_PYTHON THEN YES ELSE NO)
if(BUILD_PYTHON)
  status("     Python generate:")
  status("        pybind11:"        "ver ${pybind11_VERSION}")
  status("     Interpreter:"        "${RMVL_PYTHON_EXECUTABLE} (ver ${RMVL_PYTHON_VERSION})")
  status("     Libraries:"          "${RMVL_PYTHON_LIBRARIES}")
  status("     install path:"       "${RMVL_PYTHON_INSTALL_PATH}")
endif()

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

# ----------------------------------------------------------------------------
# CPack stuff
# ----------------------------------------------------------------------------
include(cmake/RMVLPackaging.cmake)
