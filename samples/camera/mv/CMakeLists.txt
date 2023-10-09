if(NOT BUILD_rmvl_mv_video_capture)
  return()
endif()

rmvl_add_exe(
  mv_mono
  SOURCES sample_mv_mono.cpp
  DEPENDS mv_video_capture
  EXTERNAL ${OpenCV_LIBS}
)

rmvl_add_exe(
  mv_multi
  SOURCES sample_mv_multi.cpp
  DEPENDS mv_video_capture
)

rmvl_add_exe(
  mv_writer
  SOURCES sample_mv_writer.cpp
  DEPENDS mv_video_capture
)

rmvl_add_exe(
  mv_auto_calib
  SOURCES sample_mv_auto_calib.cpp
  DEPENDS mv_video_capture
)

rmvl_add_exe(
  mv_manual_calib
  SOURCES sample_mv_manual_calib.cpp
  DEPENDS mv_video_capture
)
