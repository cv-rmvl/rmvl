if(NOT BUILD_EXTRA)
  return()
endif()
if(camera_tmp)
  unset(camera_tmp)
endif()

set(camera_tmp hik mv)

foreach(_sub ${camera_tmp})
  add_subdirectory(${_sub})
endforeach()