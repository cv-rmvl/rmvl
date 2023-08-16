# build lists of modules to be documented
set(RMVL_AUTOAIM_MODULE "")

execute_process(COMMAND ls ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE module_autoaim_st)
string(REPLACE "\n" ";" module_autoaim_st "${module_autoaim_st}")

foreach(m ${module_autoaim_st})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${m} AND (NOT (${m} STREQUAL include)))
        list(APPEND RMVL_AUTOAIM_MODULE "${m}")
    endif()
endforeach()

set(RMVL_AUTOAIM_MODULE ${RMVL_AUTOAIM_MODULE} CACHE INTERNAL "List of main modules" FORCE)

# core modules
foreach(m ${RMVL_AUTOAIM_MODULE})
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${m})
endforeach()

unset(module_autoaim_st)
