if(NOT BUILD_rmvl_mv_video_capture OR NOT BUILD_rmvl_armor_detector)
    return()
endif()

rmvl_add_exe(
    mv_armor_collection
    SOURCES sample_mv_armor_collection.cpp
    DEPENDS mv_video_capture armor_detector
)

rmvl_add_exe(
    mv_armor_size_classify
    SOURCES sample_mv_armor_size_classify.cpp
    DEPENDS mv_video_capture armor_detector
)
