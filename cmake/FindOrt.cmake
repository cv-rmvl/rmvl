if(NOT ort_root_path)
  set(ort_root_path "/usr/local")
endif()

# add the include directories path
find_path(
  Ort_INCLUDE_DIRS
  PATHS "${ort_root_path}/include/onnxruntime"
  NAMES cpu_provider_factory.h  onnxruntime_run_options_config_keys.h
        onnxruntime_c_api.h     onnxruntime_session_options_config_keys.h
        onnxruntime_cxx_api.h   provider_options.h
        onnxruntime_cxx_inline.h
  NO_DEFAULT_PATH
)

# add libraries
find_library(
  Ort_LIBS
  NAMES "libonnxruntime.so"
  PATHS "${ort_root_path}/lib"
  NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  Ort
  REQUIRED_VARS Ort_LIBS Ort_INCLUDE_DIRS
)
