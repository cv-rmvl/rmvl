execute_process(COMMAND ls ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE sample_tmp)
string(REPLACE "\n" ";" sample_tmp "${sample_tmp}")

foreach(_sub ${sample_tmp})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_sub})
        add_subdirectory(${_sub})
    endif()
endforeach()