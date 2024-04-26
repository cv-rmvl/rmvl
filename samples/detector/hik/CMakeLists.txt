if(NOT BUILD_rmvl_hik_camera OR NOT BUILD_rmvl_armor_detector)
  return()
endif()

find_package(OpenCV COMPONENTS highgui REQUIRED)

if(HAVE_FILESYSTEM)
  rmvl_add_exe(
    rmvl_hik_armor_collection
    SOURCES sample_hik_armor_collection.cpp
    DEPENDS hik_camera armor_detector
    EXTERNAL ${OpenCV_LIBS}
  )
endif()

rmvl_add_exe(
  rmvl_hik_armor_size_classify
  SOURCES sample_hik_armor_size_classify.cpp
  DEPENDS hik_camera armor_detector
  EXTERNAL ${OpenCV_LIBS}
)
