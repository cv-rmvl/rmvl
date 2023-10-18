if(NOT BUILD_rmvl_hik_video_capture OR NOT BUILD_rmvl_armor_detector)
  return()
endif()

rmvl_add_exe(
  rmvl_hik_armor_collection
  SOURCES sample_hik_armor_collection.cpp
  DEPENDS hik_video_capture armor_detector
)

rmvl_add_exe(
  rmvl_hik_armor_size_classify
  SOURCES sample_hik_armor_size_classify.cpp
  DEPENDS hik_video_capture armor_detector
)
