find_package(OpenCV REQUIRED)

rmvl_generate_para(
  camera
  MODULE camera
)
rmvl_add_module(
  camera
  DEPENDS rmath para
  EXTERNAL ${OpenCV_LIBS}
)

# --------------------------------------------------------------------------
#   Search the specific SDK
# --------------------------------------------------------------------------
# mv
find_package(MvSDK QUIET)
set(BUILD_rmvl_mv_camera_INIT ${MvSDK_FOUND})
# hik
find_package(HikSDK QUIET)
set(BUILD_rmvl_hik_camera_INIT ${HikSDK_FOUND})

# --------------------------------------------------------------------------
#   Build camera sub-library
# --------------------------------------------------------------------------
# mv
rmvl_generate_para(
    mv_camera
    MODULE camera
)
rmvl_add_module(
  mv_camera
  DEPENDS camera
  EXTRA_HEADER ${MvSDK_INCLUDE_DIRS}
  EXTERNAL ${MvSDK_LIBS}
)

# hik
rmvl_generate_para(
    hik_camera
    MODULE camera
)
rmvl_add_module(
  hik_camera
  DEPENDS camera
  EXTRA_HEADER ${HikSDK_INCLUDE_DIRS}
  EXTERNAL ${HikSDK_LIBS}
)

rmvl_generate_module_para(camera)

# --------------------------------------------------------------------------
#   Other functions
# --------------------------------------------------------------------------
# Build the test program
if(BUILD_TESTS)

endif(BUILD_TESTS)
