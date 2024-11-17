# build lists of modules to be documented
set(cur_path "${CMAKE_CURRENT_SOURCE_DIR}")

set(RMVL_EXTRA_MODULE "")
foreach(m types feature combo tracker group detector compensator predictor decider) # The order of addition must be ensured
  if(IS_DIRECTORY ${cur_path}/${m})
    list(APPEND RMVL_EXTRA_MODULE "${m}")
  endif()
endforeach()

set(RMVL_EXTRA_MODULE ${RMVL_EXTRA_MODULE} CACHE INTERNAL "List of main modules")

# core modules
foreach(m ${RMVL_EXTRA_MODULE})
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${m})
endforeach()

unset(module_extra_st)
