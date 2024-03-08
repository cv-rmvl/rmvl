if(NOT BUILD_EXTRA)
  return()
endif()
if(camera_tmp)
  unset(camera_tmp)
endif()

set(camera_tmp hik mv)

rmvl_check_cxx(HAVE_STD_FILESYSTEM "cxx17-filesystem.cpp" "-std=c++17")

foreach(_sub ${camera_tmp})
  add_subdirectory(${_sub})
endforeach()