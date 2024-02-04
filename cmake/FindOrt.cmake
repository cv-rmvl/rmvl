if(NOT ort_root_path)
  set(ort_root_path "/usr/local")
endif()

# add the include directories path
find_path(
  Ort_INCLUDE_DIR
  PATHS "${ort_root_path}/include/onnxruntime"
  NAMES "onnxruntime_cxx_api.h"
  NO_DEFAULT_PATH
)

# add libraries
find_library(
  Ort_LIB
  NAMES "libonnxruntime.so"
  PATHS "${ort_root_path}/lib"
  NO_DEFAULT_PATH
)

set(Ort_INCLUDE_DIRS "${Ort_INCLUDE_DIR}")
set(Ort_LIBS "${Ort_LIB}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  Ort
  REQUIRED_VARS Ort_LIB
)
