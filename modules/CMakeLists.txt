# build lists of modules to be documented
set(cur_path "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB module_main_st RELATIVE "${cur_path}" "${cur_path}/*")

set(RMVL_MAIN_MODULE "")
foreach(m ${module_main_st})
  if(IS_DIRECTORY ${cur_path}/${m})
    add_subdirectory(${cur_path}/${m})
    list(APPEND RMVL_MAIN_MODULE "${m}")
  endif()
endforeach()

set(
  RMVL_MAIN_MODULE ${RMVL_MAIN_MODULE}
  CACHE INTERNAL "List of common modules" FORCE
)

unset(module_main_st)
