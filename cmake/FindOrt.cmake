set(ort_root_path "/usr/local")

# add the include directories path
set(Ort_INCLUDE_DIR "${ort_root_path}/include/onnxruntime")
find_path(
  Ort_INCLUDE_DIR
  PATH "${Ort_INCLUDE_DIR}"
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
