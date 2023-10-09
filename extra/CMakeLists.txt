# build lists of modules to be documented
set(cur_path "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB module_extra_st RELATIVE "${cur_path}" "${cur_path}/*")

set(RMVL_EXTRA_MODULE "")
foreach(m ${module_extra_st})
    if(IS_DIRECTORY ${cur_path}/${m} AND (NOT (${m} STREQUAL include)))
        list(APPEND RMVL_EXTRA_MODULE "${m}")
    endif()
endforeach()

set(RMVL_EXTRA_MODULE ${RMVL_EXTRA_MODULE} CACHE INTERNAL "List of main modules" FORCE)

# core modules
foreach(m ${RMVL_EXTRA_MODULE})
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${m})
endforeach()

unset(module_extra_st)
