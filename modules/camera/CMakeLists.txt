# --------------------------------------------------------------------------
#  Camera base module
# --------------------------------------------------------------------------
rmvl_update_build(camera WITH_OPENCV AND WITH_EIGEN3)

rmvl_generate_para(
  camera
  MODULE camera
)

rmvl_add_module(
  camera
  EXTRA_HEADER ${EIGEN3_INCLUDE_DIRS}
  DEPENDS algorithm
)

# --------------------------------------------------------------------------
#  Search the specific SDK
# --------------------------------------------------------------------------
# mv
find_package(MvSDK QUIET)
rmvl_update_build(mv_camera MvSDK_FOUND AND BUILD_rmvl_camera)
# hik
find_package(HikSDK QUIET)
rmvl_update_build(hik_camera HikSDK_FOUND AND BUILD_rmvl_camera)
# opt
find_package(OPTCameraSDK QUIET)
rmvl_update_build(opt_camera OPTCameraSDK_FOUND AND BUILD_rmvl_camera)

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

# opt
rmvl_generate_para(
  opt_camera
  MODULE camera
)
rmvl_add_module(
  opt_camera
  DEPENDS camera
  EXTRA_HEADER ${OPTCameraSDK_INCLUDE_DIRS}
  EXTERNAL ${OPTCameraSDK_LIBS}
)

rmvl_generate_module_para(camera)

# --------------------------------------------------------------------------
#   Other functions
# --------------------------------------------------------------------------
# Build the test program
if(BUILD_TESTS)

endif(BUILD_TESTS)

# --------------------------------------------------------------------------
#   Build Python bindings
# --------------------------------------------------------------------------
if(BUILD_PYTHON)
  if(BUILD_rmvl_camera)
    list(APPEND cam_inc rmvl/camera/camutils.hpp) 
    list(APPEND cam_dep camera)
  endif()
  foreach(m mv hik opt)
    if(BUILD_rmvl_${m}_camera)
      list(APPEND cam_inc rmvl/camera/${m}_camera.h rmvlpara/camera/${m}_camera.h) 
      list(APPEND cam_dep ${m}_camera)
    endif()
  endforeach()
  
  rmvl_generate_python(camera
    FILES ${cam_inc}
    DEPENDS ${cam_dep}
  )
endif()