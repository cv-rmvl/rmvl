if(NOT ort_root_path)
  set(ort_root_path "/usr/local")
endif()

# add the include directories path
find_path(
  Ort_INCLUDE_DIR
  PATHS "${ort_root_path}/include/onnxruntime"
  NAMES cpu_provider_factory.h  onnxruntime_run_options_config_keys.h
        onnxruntime_c_api.h     onnxruntime_session_options_config_keys.h
        onnxruntime_cxx_api.h   provider_options.h
        onnxruntime_cxx_inline.h
  NO_DEFAULT_PATH
)

# add libraries
find_library(
  Ort_LIB
  NAMES "libonnxruntime.so"
  PATHS "${ort_root_path}/lib"
  NO_DEFAULT_PATH
)

if(NOT TARGET onnxruntime)
  add_library(onnxruntime SHARED IMPORTED)
  set_target_properties(onnxruntime PROPERTIES
    IMPORTED_LOCATION "${Ort_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${Ort_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(Ort_INCLUDE_DIR Ort_LIB)

set(Ort_LIBS "onnxruntime")
set(Ort_INCLUDE_DIRS "${Ort_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  Ort
  REQUIRED_VARS Ort_LIB Ort_INCLUDE_DIR
)
