if(NOT BUILD_EXTRA)
  return()
endif()
if(camera_tmp)
  unset(camera_tmp)
endif()

set(camera_tmp hik mv)

rmvl_check_include_file(
  CXX FILES filesystem
)

foreach(_sub ${camera_tmp})
  add_subdirectory(${_sub})
endforeach()