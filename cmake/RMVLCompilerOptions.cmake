# ----------------------------------------------------------------------------
#   RMVL compile configure
# ----------------------------------------------------------------------------

# Compile Standard
set(max_version 17)
foreach(ver 17 20)
  rmvl_check_cxx(HAVE_CXX${ver} "cxx${ver}.cpp" "")
  if(HAVE_CXX${ver})
    set(max_version ${ver})
  else()
    rmvl_check_cxx(HAVE_STD_CXX${ver} "cxx${ver}.cpp" "-std=c++${ver}")
    if(HAVE_STD_CXX${ver})
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++${ver}")
      set(HAVE_CXX${ver} ON)
      set(max_version ${ver})
    endif()
  endif()
endforeach()

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
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # Here is the compile-generate-path for Linux and MacOS
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
  set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
  set(3P_LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/3rdparty/lib)
else()
  # Here is the compile-generate-path for Windows
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}\\bin)
  set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}\\lib)
  set(3P_LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}\\3rdparty\\lib)
endif()

# ----------------------------------------------------------------------------
#   Build options
# ----------------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_PIC "Generate position independent code (necessary for shared libraries)" ON)
if(ENABLE_PIC)
  set(CMAKE_POSITION_INDEPENDENT_CODE ${ENABLE_PIC})
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
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    list(APPEND MSVC_OPTIONS "/W3")
    if(MSVC_VERSION GREATER 1900) # Allow non fatal security warnings for msvc 2015
      list(APPEND MSVC_OPTIONS "/WX")
    endif()
    add_compile_options(MSVC_OPTIONS)
  else()
    add_compile_options(
      -Wall
      -Wextra
      # -Wconversion
      # -Wpedantic
      # -Werror
      # -Wfatal-errors
    )
  endif()
endif()

# Coverage test
option(ENABLE_COVERAGE "Build with unit test coverage" OFF)
if(ENABLE_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
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
    include(${CMAKE_MODULE_PATH}/check/check_asan.cmake)
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
#   Module and other options
# ----------------------------------------------------------------------------
option(BUILD_EXTRA "Build extra modules containing 4 data components and 4 function modules" ON)
option(BUILD_EXAMPLES "Build RMVL all examples" ON)
option(BUILD_DOCS "Create build rules for RMVL Documentation" OFF)

# ----------------------------------------------------------------------------
#   3rdparty options
# ----------------------------------------------------------------------------
option(BUILD_APRILTAG "Build the 3rd party: apriltag" ON)
option(BUILD_OPEN62541 "Build the 3rd party: open62541" OFF)

option(WITH_APRILTAG "Enable apriltag support" ON)
if(WITH_APRILTAG)
  include(RMVLFindAprilTag)
endif()
option(WITH_OPEN62541 "Enable open62541 support" ON)
if(WITH_OPEN62541)
  include(RMVLFindOpen62541)
endif()
option(WITH_ONNXRUNTIME "Enable onnxruntime support" ON)
if(WITH_ONNXRUNTIME)
  find_package(Ort QUIET)
  option(WITH_ONNXRUNTIME "Enable onnxruntime support" ${Ort_FOUND})
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
  set(BENCHMARK_ENABLE_TESTING OFF)
  rmvl_download(benchmark GIT "https://github.com/google/benchmark.git : v1.8.0")
endif(BUILD_PERF_TESTS)
