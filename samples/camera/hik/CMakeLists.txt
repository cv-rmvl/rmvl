if(NOT BUILD_rmvl_hik_video_capture)
  return()
endif()

rmvl_add_exe(
  rmvl_hik_mono
  SOURCES sample_hik_mono.cpp
  DEPENDS hik_video_capture
  EXTERNAL ${OpenCV_LIBS}
)

rmvl_add_exe(
  rmvl_hik_multi
  SOURCES sample_hik_multi.cpp
  DEPENDS hik_video_capture
  EXTERNAL ${OpenCV_LIBS}
)

rmvl_add_exe(
  rmvl_hik_writer
  SOURCES sample_hik_writer.cpp
  DEPENDS hik_video_capture
  EXTERNAL ${OpenCV_LIBS}
)

rmvl_add_exe(
  rmvl_hik_manual_calib
  SOURCES sample_hik_manual_calib.cpp
  DEPENDS hik_video_capture
)
