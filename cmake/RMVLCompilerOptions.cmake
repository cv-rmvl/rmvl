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

set(CMAKE_JOB_POOL_COMPILE "compile")
set(CMAKE_JOB_POOL_LINK "link")

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
#   Build options
# ----------------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_PIC "Generate position independent code (necessary for shared libraries)" ON)
if(ENABLE_PIC)
  set(CMAKE_POSITION_INDEPENDENT_CODE ${ENABLE_PIC})
endif()

option(ENABLE_LTO "Enable Link Time Optimization" OFF)
if(ENABLE_LTO)
  include(CheckCXXCompilerFlag)
  if(RMVL_GNU OR RMVL_CLANG OR RMVL_APPLECLANG)
    check_cxx_compiler_flag("-flto=auto" HAS_LTO_AUTO_FLAG)
    if(HAS_LTO_AUTO_FLAG)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=auto")
    else()
      check_cxx_compiler_flag("-flto" HAS_LTO_FLAG)
      if(HAS_LTO_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
      endif()
    endif()
  elseif(RMVL_MSVC)
    check_cxx_compiler_flag("/GL" COMPILER_SUPPORTS_LTO)
    if(COMPILER_SUPPORTS_LTO)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GL")
      set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /LTCG")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LTCG")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
    endif()
  endif()
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
    add_compile_options("/W3")
  else()
    add_compile_options(
      -Wall
      -Wextra
      -Wpedantic
      # -Wconversion
      # -Werror
      # -Wfatal-errors
    )
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
# eigen3
find_package(Eigen3 QUIET)
if(Eigen3_FOUND)
  option(WITH_EIGEN3 "Enable libeigen3 support" ON)
else()
  unset(WITH_EIGEN3 CACHE)
  option(WITH_EIGEN3 "Enable libeigen3 support" OFF)
endif()

# opencv
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
  option(WITH_OPENCV "Enable opencv support" ON)
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

# apriltag
option(BUILD_APRILTAG "Build the 3rd party: apriltag" ON)
if(BUILD_APRILTAG)
  add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/apriltag)
  option(WITH_APRILTAG "Enable apriltag support" ON)
else()
  unset(WITH_APRILTAG CACHE)
  option(WITH_APRILTAG "Enable apriltag support" OFF)
endif()

# open62541
option(BUILD_OPEN62541 "Build the 3rd party: open62541" OFF)
if(BUILD_OPEN62541)
  add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/open62541)
endif()
if(BUILD_OPEN62541)
  unset(WITH_OPEN62541 CACHE)
  option(WITH_OPEN62541 "Enable open62541 support" ON)
else()
  find_package(open62541 QUIET)
  if(open62541_FOUND)
    option(WITH_OPEN62541 "Enable open62541 support" ON)
  else()
    unset(WITH_OPEN62541 CACHE)
    option(WITH_OPEN62541 "Enable open62541 support" OFF)
  endif()
endif()

# ----------------------------------------------------------------------------
#   Module and other options
# ----------------------------------------------------------------------------
option(BUILD_EXTRA "Build extra modules containing 4 data components and 4 function modules" OFF)
if(NOT WITH_OPENCV)
  unset(BUILD_EXTRA CACHE)
  option(BUILD_EXTRA "Build extra modules containing 4 data components and 4 function modules" OFF)
endif()
option(BUILD_EXAMPLES "Build RMVL all examples" ON)
option(BUILD_DOCS "Create build rules for RMVL Documentation" OFF)

option(BUILD_PYTHON "Build python bindings" OFF)
if(BUILD_PYTHON)
  find_package(SWIG QUIET)
  find_package(Python3 COMPONENTS Interpreter Development QUIET)
  if(NOT SWIG_FOUND OR NOT Python3_FOUND)
    unset(BUILD_PYTHON CACHE)
    option(BUILD_PYTHON "Build python bindings" OFF)
    message(WARNING "SWIG or Python3 not found, python bindings will not be built")
  else()
    include(UseSWIG)
    set(CMAKE_SWIG_OUTDIR ${PROJECT_BINARY_DIR}/python/rm)
    set(SWIG_OUTFILE_DIR ${PROJECT_BINARY_DIR}/pygen-cpp)
    include(${CMAKE_CURRENT_LIST_DIR}/RMVLGenPython.cmake)
    # create __init__.py
    file(WRITE ${CMAKE_SWIG_OUTDIR}/__init__.py "")
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
  find_package(GTest)
  find_package(Threads)
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
