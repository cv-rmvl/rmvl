if(NOT ort_root_path)
  set(ort_root_path "/usr/local")
endif()

# add the include directories path
find_path(
  Ort_INCLUDE_DIRS
  PATHS "${ort_root_path}/include/onnxruntime"
  NAMES "onnxruntime_cxx_api.h"
  NO_DEFAULT_PATH
)

# add libraries
find_library(
  Ort_LIBS
  NAMES "libonnxruntime.so"
  PATHS "${ort_root_path}/lib"
  NO_DEFAULT_PATH
)

mark_as_advanced(Ort_INCLUDE_DIRS Ort_LIBS)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  Ort
  REQUIRED_VARS Ort_LIBS Ort_INCLUDE_DIRS
)
