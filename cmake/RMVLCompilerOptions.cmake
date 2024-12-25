# ----------------------------------------------------------------------------
#   Obtain compiler ID
# ----------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(RMVL_GNU TRUE)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(RMVL_CLANG TRUE)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  set(RMVL_APPLECLANG TRUE)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(RMVL_MSVC TRUE)
endif()

# ----------------------------------------------------------------------------
#   Set encoding format
# ----------------------------------------------------------------------------
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# ----------------------------------------------------------------------------
#   RMVL compile configure
# ----------------------------------------------------------------------------

# Compile Standard
set(max_version 17)
set(_cxx_init_flags "${CMAKE_CXX_FLAGS}")
if(RMVL_MSVC)
  set(_cxx_flags "/std:c++")
elseif(RMVL_GNU OR RMVL_CLANG OR RMVL_APPLECLANG)
  set(_cxx_flags "-std=c++")
endif()
foreach(ver 17 20 23)
  rmvl_check_cxx(HAVE_CXX${ver} "cxx${ver}.cpp" "")
  if(HAVE_CXX${ver})
    set(max_version ${ver})
  else()
    rmvl_check_cxx(HAVE_STD_CXX${ver} "cxx${ver}.cpp" "${_cxx_flags}${ver}")
    if(HAVE_STD_CXX${ver})
      set(CMAKE_CXX_FLAGS "${_cxx_init_flags} ${_cxx_flags}${ver}")
      set(HAVE_CXX${ver} ON)
      set(max_version ${ver})
    else()
      break()
    endif()
  endif()
endforeach()
unset(_cxx_flags)

if(NOT HAVE_CXX17)
  message(FATAL_ERROR "RMVL requires C++17")
endif()

set(CMAKE_CXX_STANDARD ${max_version})

# ----------------------------------------------------------------------------
#   Detect target platform architecture
# ----------------------------------------------------------------------------
# Add these standard paths to the search paths for FIND_LIBRARY
# to find libraries from these locations first
if(UNIX AND NOT ANDROID)
  if(X86_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8)
    if(EXISTS /lib64)
      list(APPEND CMAKE_LIBRARY_PATH /lib64)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /lib)
    endif()
    if(EXISTS /usr/lib64)
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib64)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib)
    endif()
  elseif(X86 OR CMAKE_SIZEOF_VOID_P EQUAL 4)
    if(EXISTS /lib32)
      list(APPEND CMAKE_LIBRARY_PATH /lib32)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /lib)
    endif()
    if(EXISTS /usr/lib32)
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib32)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib)
    endif()
  endif()
endif()

# Add these standard paths to the search paths for FIND_PATH
# to find include files from these locations first
if(MINGW)
  if(EXISTS /mingw)
    list(APPEND CMAKE_INCLUDE_PATH /mingw)
  endif()
  if(EXISTS /mingw32)
    list(APPEND CMAKE_INCLUDE_PATH /mingw32)
  endif()
  if(EXISTS /mingw64)
    list(APPEND CMAKE_INCLUDE_PATH /mingw64)
  endif()
endif()

# ----------------------------------------------------------------------------
#   Compile generate path
# ----------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(3P_LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/3rdparty/lib)

# ----------------------------------------------------------------------------
#   Build options and configurations
# ----------------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_PIC "Generate position independent code (necessary for shared libraries)" ON)
if(ENABLE_PIC)
  set(CMAKE_POSITION_INDEPENDENT_CODE ${ENABLE_PIC})
endif()

include(CheckCXXCompilerFlag)

function(check_and_add_cxx_flag cxx_flag)
  string(FIND "${CMAKE_CXX_FLAGS}" "${cxx_flag}" flag_already_set)
  if(flag_already_set EQUAL -1)
    string(REGEX REPLACE "[-=/]" "_" flag_var_name "HAVE${cxx_flag}")
    string(TOUPPER ${flag_var_name} flag_var_name)
    check_cxx_compiler_flag("${cxx_flag}" ${flag_var_name})
    if(${flag_var_name})
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cxx_flag}" CACHE INTERNAL "CXX Compiler Flags")
    endif()
  endif()
endfunction()

if(RMVL_MSVC)
  option(ENABLE_LTO "Enable Link Time Optimization" OFF)
else()
  option(ENABLE_LTO "Enable Link Time Optimization" ON)
endif()
if(ENABLE_LTO)
  if(RMVL_GNU OR RMVL_CLANG OR RMVL_APPLECLANG)
    check_and_add_cxx_flag("-flto=auto")
    if(NOT HAVE_FLTO_AUTO)
      check_and_add_cxx_flag("-flto")
    endif()
  elseif(RMVL_MSVC)
    check_and_add_cxx_flag("/GL")
    if(HAVE_GL)
      set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /LTCG")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LTCG")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
    endif()
  endif()
endif()

if(WIN32)
  # postfix of DLLs
  set(RMVL_LIBVERSION_SUFFIX "${RMVL_VERSION_MAJOR}${RMVL_VERSION_MINOR}${RMVL_VERSION_PATCH}" CACHE INTERNAL "RMVL library version suffix")
  set(RMVL_DEBUG_POSTFIX "d" CACHE INTERNAL "RMVL debug postfix")
else()
  # postfix of *.so
  set(RMVL_LIBVERSION_SUFFIX "")
  set(RMVL_DEBUG_POSTFIX "")
endif()

if(DEFINED CMAKE_DEBUG_POSTFIX)
  set(RMVL_DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}" CACHE INTERNAL "RMVL debug postfix")
endif()

# ----------------------------------------------------------------------------
#   Develop options
# ----------------------------------------------------------------------------
# Cache compilation acceleration
option(ENABLE_CCACHE "Use ccache to faster compile when develop" ON)
if(ENABLE_CCACHE)
  message(STATUS "Looking for ccache")
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
    set(CMAKE_C_COMPILER_LAUNCHER ccache)
    # set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache) # ccache for link is useless
  endif()
  if(CCACHE_FOUND)
    message(STATUS "Looking for ccache - found (${CCACHE_FOUND})")
  else()
    message(STATUS "Looking for ccache - not found")
  endif()
endif()

# Code warning
option(ENABLE_WARNING "Enable warning for all project " ON)
if(ENABLE_WARNING)
  if(RMVL_MSVC)
    check_and_add_cxx_flag("/W3")
  else()
    check_and_add_cxx_flag("-Wall")
    check_and_add_cxx_flag("-Wextra")
    check_and_add_cxx_flag("-Wpedantic")
  endif()
endif()

# Coverage test
option(ENABLE_COVERAGE "Build with unit test coverage" OFF)
if(ENABLE_COVERAGE)
  if(RMVL_GNU OR RMVL_CLANG OR RMVL_APPLECLANG)
    add_compile_options("-fprofile-arcs -ftest-coverage --coverage")
  else()
    add_compile_options("-fprofile-instr-generate -fcoverage-mapping")
  endif()
endif()

# Code analysis: Address / Undefined / Memory / Thread
option(ENABLE_ADDRESS_SANITIZER "Build with Address Sanitizers (Debug + GCC / Clang / AppleClang) " OFF)
option(ENABLE_UNDEFINED_SANITIZER "Build Undefined Sanitizers (Debug + GCC / Clang / AppleClang) " OFF)
option(ENABLE_MEMORY_SANITIZER "Build Memory Sanitizers (Debug + GCC / Clang / AppleClang) " OFF)
option(ENABLE_THREAD_SANITIZER "Build Thread Sanitizers (Debug + GCC / Clang / AppleClang) " OFF)
if(NOT MSVC)
  if(ENABLE_ADDRESS_SANITIZER)
    include(${CMAKE_CURRENT_LIST_DIR}/check/check_asan.cmake)
    check_asan(HAS_ASAN)
    if(NOT HAS_ASAN)
      message(FATAL_ERROR "sanitizer is no supported with current tool-chains")
    endif()
    add_compile_options(
      -fsanitize=address # ,pointer-compare,pointer-subtract
      -fno-omit-frame-pointer
      -fno-common
      -fsanitize-recover=address
    )
    add_link_options(-fsanitize=address)
    find_program(SYMBOLIZER llvm-symbolizer)
    if(SYMBOLIZER-NOTFOUND)
      message(STATUS "use llvm-symbolizer to see sanitizer output: sudo apt install llvm")
    endif()
  endif()
  if(ENABLE_UNDEFINED_SANITIZER)
    add_compile_options(-fsanitize=undefined -Wno-error)
    add_link_options(-fsanitize=undefined)
  endif()
  if(ENABLE_MEMORY_SANITIZER AND NOT GNU)
    add_compile_options(-fsanitize=memory)
    add_link_options(-fsanitize=memory)
  endif()
  if(ENABLE_THREAD_SANITIZER)
    # Turn off other detectors to avoid collisions
    add_compile_options(-fno-sanitize=all -fsanitize=thread)
    add_link_options(-fno-sanitize=all -fsanitize=thread)
  endif()
  if(ENABLE_ADDRESS_SANITIZER OR ENABLE_UNDEFINED_SANITIZER OR ENABLE_THREAD_SANITIZER OR ENABLE_MEMORY_SANITIZER)
    configure_file(
      "${CMAKE_CURRENT_LIST_DIR}/templates/sanitizer/sanitizer_option.sh.in"
      "${CMAKE_CURRENT_BINARY_DIR}/sanitizer_option.sh"
      @ONLY
    )
  endif()
endif()

# ----------------------------------------------------------------------------
#   3rdparty options
# ----------------------------------------------------------------------------
macro(_rmvl_set_target_in_3rd _name)
  set(${_name}_IN_3RD ON CACHE INTERNAL "")
endmacro()

# opencv
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
  option(WITH_OPENCV "Enable opencv support" ON)
  # get the path of all 'DLLs' of OpenCV for Windows
  if(WIN32)
    if(TARGET opencv_world)
      get_target_property(opencv_world_dll_path opencv_world IMPORTED_LOCATION_RELEASE)
      get_target_property(opencv_world_lib_path opencv_world IMPORTED_IMPLIB_RELEASE)
      if(EXISTS ${opencv_world_dll_path} AND EXISTS ${opencv_world_lib_path})
        set(OpenCV_DLL_PATHS ${opencv_world_dll_path})
        set(OpenCV_LIB_PATHS ${opencv_world_lib_path})
      endif()
    endif()
    set(OpenCV_DLL_PATHS ${OpenCV_DLL_PATHS} CACHE INTERNAL "OpenCV DLL paths")
    set(OpenCV_LIB_PATHS ${OpenCV_LIB_PATHS} CACHE INTERNAL "OpenCV LIB paths")
    get_filename_component(OpenCV_DLL_NAME ${OpenCV_DLL_PATHS} NAME)
    get_filename_component(OpenCV_LIB_NAME ${OpenCV_LIB_PATHS} NAME)
    set(OpenCV_DLL_NAME ${OpenCV_DLL_NAME} CACHE INTERNAL "OpenCV DLL name")
    set(OpenCV_LIB_NAME ${OpenCV_LIB_NAME} CACHE INTERNAL "OpenCV LIB name")
  endif()
else()
  unset(WITH_OPENCV CACHE)
  option(WITH_OPENCV "Enable opencv support" OFF)
endif()

# onnxruntime
find_package(Ort QUIET)
if(Ort_FOUND)
  option(WITH_ONNXRUNTIME "Enable onnxruntime support (dep: opencv)" ON)
else()
  unset(WITH_ONNXRUNTIME CACHE)
  option(WITH_ONNXRUNTIME "Enable onnxruntime support (dep: opencv)" OFF)
endif()

# Set the "WITH_" and "BUILD_" compilation options of 3rdparty, with "build_default_status"
# as the default value of "BUILD_". If "BUILD_" is ON, automatically set "WITH_" to ON, otherwise,
# by default, find the 3rdparty from the system path as the default value of "WITH_".
macro(_rmvl_set_build_with_3rdparty _name build_default_status)
  # uppercase and lowercase
  string(TOUPPER ${_name} upper_name)
  string(TOLOWER ${_name} lower_name)
  # "build" 3rdparty option
  option(BUILD_${upper_name} "Build the 3rd party: ${_name}" ${build_default_status})
  if(BUILD_${upper_name})
    add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/${lower_name})
  endif()
  # "with" 3rdparty option
  if(BUILD_${upper_name})
    unset(WITH_${upper_name} CACHE)
    option(WITH_${upper_name} "Enable ${_name} support" ON)
  else()
    find_package(${_name} QUIET)
    if(${_name}_FOUND)
      option(WITH_${upper_name} "Enable ${_name} support" ON)
    else()
      unset(WITH_${upper_name} CACHE)
      option(WITH_${upper_name} "Enable ${_name} support" OFF)
    endif()
  endif()
endmacro()

# eigen3
_rmvl_set_build_with_3rdparty(Eigen3 OFF)
if(WITH_EIGEN3)
  include_directories(${EIGEN3_INCLUDE_DIR})
endif()

# apriltag
_rmvl_set_build_with_3rdparty(apriltag ON)

# open62541
_rmvl_set_build_with_3rdparty(open62541 OFF)

# ----------------------------------------------------------------------------
#   Module and other options
# ----------------------------------------------------------------------------
option(BUILD_WORLD "Build all modules as a single library" OFF)
if(BUILD_WORLD)
  if(BUILD_SHARED_LIBS)
    add_library(rmvl_world SHARED)
    set_target_properties(
      rmvl_world PROPERTIES
      DEFINE_SYMBOL RMVLAPI_EXPORTS
    )
  else()
    add_library(rmvl_world STATIC)
  endif()
endif()

option(BUILD_EXTRA "Build extra modules containing 4 data components and 4 function modules" OFF)
if(NOT WITH_OPENCV)
  unset(BUILD_EXTRA CACHE)
  option(BUILD_EXTRA "Build extra modules containing 4 data components and 4 function modules" OFF)
endif()
option(BUILD_EXAMPLES "Build RMVL all examples" ON)
option(BUILD_DOCS "Create build rules for RMVL Documentation" OFF)

option(BUILD_PYTHON "Build python bindings" OFF)
if(BUILD_PYTHON)
  find_package(Python3 COMPONENTS Interpreter Development QUIET)
  if(NOT Python3_FOUND)
    unset(BUILD_PYTHON CACHE)
    option(BUILD_PYTHON "Build python bindings" OFF)
    message(WARNING
      "Python3 not found, python bindings will not be built.\n"
      "If your OS is Ubuntu / Debian, please use the following command:\n"
      "  sudo apt install python3-dev\n"
      "If your OS is Windows, please install python3 in the official website:\n"
      "  https://www.python.org/downloads/"
    )
  else()
    find_package(pybind11 QUIET)
    if(NOT pybind11_FOUND)
      add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/pybind11)
    endif()
    set(pybind11_VERSION "${pybind11_VERSION}" CACHE INTERNAL "pybind11 version")
    set(RMVL_PYTHON_VERSION_MAJOR "${Python3_VERSION_MAJOR}")
    set(RMVL_PYTHON_VERSION_MINOR "${Python3_VERSION_MINOR}")
    set(RMVL_PYTHON_VERSION "${Python3_VERSION}")
    set(RMVL_PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
    set(RMVL_PYTHON_INTERPRETER_FOUND "${Python3_Interpreter_FOUND}")
    set(RMVL_PYTHON_LIBRARIES "${Python3_LIBRARIES}")

    set(RMVL_PYBIND_OUTPUT_DIR ${PROJECT_BINARY_DIR}/python/pybind-cpp)
    set(RMVL_PYTHON_OUTPUT_DIR ${PROJECT_BINARY_DIR}/python/rm)
    set(RMVL_PYDOC_OUTPUT_DIR ${PROJECT_BINARY_DIR}/python/docs)
    include(${CMAKE_CURRENT_LIST_DIR}/RMVLGenPython.cmake)
    # generate files for python bindings
    configure_file(
      "${CMAKE_CURRENT_LIST_DIR}/templates/python/rmvl_init.py"
      "${RMVL_PYTHON_OUTPUT_DIR}/__init__.py"
      @ONLY
    )
    configure_file(
      "${CMAKE_CURRENT_LIST_DIR}/templates/python/rmvl_init.pyi"
      "${RMVL_PYTHON_OUTPUT_DIR}/rmvl_typing.pyi"
      @ONLY
    )
    if(BUILD_DOCS)
      file(WRITE ${RMVL_PYDOC_OUTPUT_DIR}/pyrmvl_cst.cfg "# constants configuration for pyrmvl\n")
      configure_file(
        "${PROJECT_SOURCE_DIR}/doc/tools/pyrmvl_fns.cfg"
        "${RMVL_PYDOC_OUTPUT_DIR}/pyrmvl_fns.cfg"
        COPYONLY
      )
    endif()
    if(UNIX)
      execute_process(
        COMMAND ${RMVL_PYTHON_EXECUTABLE} -c "from sysconfig import *; print(get_path('purelib'))"
        RESULT_VARIABLE _rmvlpy_process
        OUTPUT_VARIABLE _std_packages_path
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      set(RMVL_PYTHON_INSTALL_PATH "${_std_packages_path}/rm")
    endif()
  endif()
endif()

# ----------------------------------------------------------------------------
#   Build performance and unit tests
# ----------------------------------------------------------------------------
# Unit tests
option(BUILD_TESTS "Build accuracy & regression tests" OFF)
if(BUILD_TESTS)
  enable_testing()
  # Using gtests command
  include(GoogleTest)
  # Find GoogleTest
  find_package(Threads)
  find_package(GTest QUIET)
  if(NOT GTest_FOUND)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    rmvl_download(googletest URL "https://github.com/google/googletest/archive/main.zip")
  endif()
endif()

# Performance tests
option(BUILD_PERF_TESTS "Build performance tests" OFF)
if(BUILD_PERF_TESTS)
  enable_testing()
  find_package(benchmark QUIET)
  if(NOT benchmark_FOUND)
    set(BENCHMARK_ENABLE_TESTING OFF)
    rmvl_download(benchmark GIT "https://github.com/google/benchmark.git : v1.8.0")
  endif()
endif(BUILD_PERF_TESTS)
