# build lists of modules to be documented
set(RMVL_MAIN_MODULE "")

execute_process(COMMAND ls ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE module_main_st)
string(REPLACE "\n" ";" module_main_st "${module_main_st}")
foreach(m ${module_main_st})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${m})
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${m})
        list(APPEND RMVL_MAIN_MODULE "${m}")
    endif()
endforeach()

set(
    RMVL_MAIN_MODULE ${RMVL_MAIN_MODULE}
    CACHE INTERNAL "List of common modules" FORCE
)

unset(module_main_st)
