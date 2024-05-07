# ------------------------------------------------------------------------------
#  find onnxruntime root path
# ------------------------------------------------------------------------------
if(NOT ort_root_path)
  set(ort_root_path "/usr/local")
endif()

# ------------------------------------------------------------------------------
#  find onnxruntime include directory
# ------------------------------------------------------------------------------
find_path(
  Ort_INCLUDE_DIR
  PATHS "${ort_root_path}/include/onnxruntime"
  NAMES cpu_provider_factory.h  onnxruntime_run_options_config_keys.h
        onnxruntime_c_api.h     onnxruntime_session_options_config_keys.h
        onnxruntime_cxx_api.h   provider_options.h
        onnxruntime_cxx_inline.h
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
