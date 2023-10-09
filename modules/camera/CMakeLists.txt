find_package(OpenCV REQUIRED)

rmvl_generate_para(
  camera
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
find_package(MvSDK)
set(BUILD_rmvl_mv_video_capture_INIT ${MvSDK_FOUND})
# hik
find_package(HikSDK)
set(BUILD_rmvl_hik_video_capture_INIT ${HikSDK_FOUND})

# --------------------------------------------------------------------------
#   Build camera sub-library
# --------------------------------------------------------------------------
# mv
rmvl_add_module(
  mv_video_capture
  DEPENDS camera
  EXTRA_HEADER ${MvSDK_INCLUDE_DIRS}
  EXTERNAL ${MvSDK_LIBS}
)

# hik
rmvl_add_module(
  hik_video_capture
  DEPENDS camera
  EXTRA_HEADER ${HikSDK_INCLUDE_DIRS}
  EXTERNAL ${HikSDK_LIBS}
)

# --------------------------------------------------------------------------
#   Other functions
# --------------------------------------------------------------------------
# Build the test program
if(BUILD_TESTS)

endif(BUILD_TESTS)
