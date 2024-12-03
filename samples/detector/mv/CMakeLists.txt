if(NOT BUILD_rmvl_mv_camera OR NOT BUILD_rmvl_armor_detector)
  return()
endif()

if(HAVE_FILESYSTEM)
  rmvl_add_exe(
    rmvl_mv_armor_collection
    SOURCES sample_mv_armor_collection.cpp
    DEPENDS mv_camera armor_detector
  )
endif()

rmvl_add_exe(
  rmvl_mv_armor_size_classify
  SOURCES sample_mv_armor_size_classify.cpp
  DEPENDS mv_camera armor_detector
)
