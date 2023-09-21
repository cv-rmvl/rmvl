# build lists of modules to be documented
set(RMVL_EXTRA_MODULE "")

execute_process(COMMAND ls ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE module_extra_st)
string(REPLACE "\n" ";" module_extra_st "${module_extra_st}")

foreach(m ${module_extra_st})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${m} AND (NOT (${m} STREQUAL include)))
        list(APPEND RMVL_EXTRA_MODULE "${m}")
    endif()
endforeach()

set(RMVL_EXTRA_MODULE ${RMVL_EXTRA_MODULE} CACHE INTERNAL "List of main modules" FORCE)

# core modules
foreach(m ${RMVL_EXTRA_MODULE})
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${m})
endforeach()

unset(module_extra_st)
