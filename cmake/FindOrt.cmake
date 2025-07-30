# --------------------------------------------------------------------------------------------
#  This file is used to find the ONNX-Runtime SDK, which provides the following variables:
#
#  Cache Variables:
#  - Ort_HEADER_FILES: Names of SDK header files
#
#  Advanced Variables:
#  - Ort_INCLUDE_DIR: Directory where SDK header files are located
#  - Ort_LIB:         Path to the SDK library file (import library on Windows, shared library
#                     on Linux)
#  - Ort_DLL:         Path to the SDK dynamic library file (only on Windows)
#
#  Local Variables:
#  - Ort_LIBS:         CMake target name for the SDK, which is "onnxruntime"
#  - Ort_INCLUDE_DIRS: Directory where SDK header files are located
# --------------------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
#  find onnxruntime root path
# ------------------------------------------------------------------------------
if(NOT ort_root_path)
  set(ort_root_path "/usr/local")
endif()

# ------------------------------------------------------------------------------
#  find onnxruntime include directory
# ------------------------------------------------------------------------------
set(Ort_HEADER_FILES
  cpu_provider_factory.h  onnxruntime_run_options_config_keys.h
  onnxruntime_c_api.h     onnxruntime_session_options_config_keys.h
  onnxruntime_cxx_api.h   provider_options.h
  onnxruntime_cxx_inline.h
  CACHE INTERNAL "ONNX Runtime header files"
)

find_path(
  Ort_INCLUDE_DIR
  PATHS "${ort_root_path}/include/onnxruntime"
  NAMES ${Ort_HEADER_FILES}
  NO_DEFAULT_PATH
)

# ------------------------------------------------------------------------------
#  find onnxruntime library file
# ------------------------------------------------------------------------------
find_library(
  Ort_LIB
  NAMES "libonnxruntime.so"
  PATHS "${ort_root_path}/lib"
  NO_DEFAULT_PATH
)

# ------------------------------------------------------------------------------
#  create imported target: onnxruntime
# ------------------------------------------------------------------------------
if(NOT TARGET onnxruntime)
  add_library(onnxruntime SHARED IMPORTED)
  set_target_properties(onnxruntime PROPERTIES
    IMPORTED_LOCATION "${Ort_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${Ort_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(Ort_INCLUDE_DIR Ort_LIB)

# ------------------------------------------------------------------------------
#  set onnxruntime cmake variables and version variables
# ------------------------------------------------------------------------------
set(Ort_LIBS "onnxruntime")
set(Ort_INCLUDE_DIRS "${Ort_INCLUDE_DIR}")

if(Ort_INCLUDE_DIR)
  file(STRINGS "${Ort_INCLUDE_DIR}/onnxruntime_c_api.h" Ort_VERSION
    REGEX "#define ORT_API_VERSION [0-9]+"
  )
  string(REGEX REPLACE "#define ORT_API_VERSION ([0-9]+)" "1.\\1" Ort_VERSION "${Ort_VERSION}")
endif()

# ------------------------------------------------------------------------------
#  handle the package
# ------------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  Ort
  VERSION_VAR Ort_VERSION
  REQUIRED_VARS Ort_LIB Ort_INCLUDE_DIR
)
