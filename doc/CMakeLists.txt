# gathering headers
set(paths_include)
set(paths_test)
set(example_path "${CMAKE_SOURCE_DIR}/samples")
foreach(m ${RMVL_MODULES_BUILD})
    set(the_module "${m}")
    if(NOT the_module MATCHES "^rmvl_")
        set(the_module "rmvl_${m}")
    endif()
    # include files
    set(header_dir "${RMVL_MODULE_${the_module}_LOCATION}/include")
    if(EXISTS "${header_dir}")
        list(APPEND paths_include "${header_dir}")
    endif()
    # test files
    set(test_dir "${RMVL_MODULE_${the_module}_LOCATION}/test")
    if(EXISTS "${test_dir}")
        list(APPEND paths_test "${test_dir}")
    endif()
endforeach(m ${RMVL_MODULES_BUILD})

# additional config
set(tutorialdir "${CMAKE_CURRENT_LIST_DIR}/tutorials")
set(doxyfile "${CMAKE_BINARY_DIR}/Doxyfile")
set(bibfile "${CMAKE_CURRENT_LIST_DIR}/rmvl.bib")
set(rootfile "${CMAKE_BINARY_DIR}/root.markdown")
set(introfile "${CMAKE_CURRENT_LIST_DIR}/intro.markdown")

# set export variables
string(REPLACE ";" " \\\n" CMAKE_DOXYGEN_INPUT_LIST "${paths_include}; ${rootfile}; ${introfile}; ${tutorialdir}")
string(REPLACE ";" " \\\n" CMAKE_DOXYGEN_INCLUDE_ROOTS "${paths_include}")
string(REPLACE ";" " \\\n" CMAKE_DOXYGEN_TEST_ROOTS "${paths_test}")
string(REPLACE ";" " \\\n" CMAKE_DOXYGEN_EXAMPLE_PATH "${example_path}")
set(CMAKE_DOXYGEN_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(CMAKE_EXTRA_BIB_FILES "${bibfile}")

list(APPEND CMAKE_DOXYGEN_HTML_FILES "${CMAKE_CURRENT_LIST_DIR}/rmvl-logo-small.png")
list(APPEND CMAKE_DOXYGEN_HTML_FILES "${CMAKE_CURRENT_LIST_DIR}/tutorial-utils.js")
string(REPLACE ";" " \\\n" CMAKE_DOXYGEN_HTML_FILES "${CMAKE_DOXYGEN_HTML_FILES}")

set(
    RMVL_DOXYFILE_IN Doxyfile.in
    CACHE PATH "RMVL_DOXYFILE_IN"
)

# List all the modules
set(refs_autoaim)
set(refs_modules)
foreach(m ${RMVL_AUTOAIM_MODULE} ${RMVL_MAIN_MODULE})
    # Reference entry
    if(NOT "${m}" STREQUAL "para")
        set(one_ref "\t- ${m}. @ref ${m}\n")
        list(FIND RMVL_MAIN_MODULE ${m} _pos)
        if(${_pos} EQUAL -1)
            set(refs_autoaim "${refs_autoaim}${one_ref}")
        else()
            set(refs_modules "${refs_modules}${one_ref}")
        endif()
    endif()
endforeach()

if(refs_autoaim)
    set(refs_autoaim "- Autoaim modules. 自瞄模块\n${refs_autoaim}")
endif()
if(refs_modules)
    set(refs_modules "- Main modules. 主要模块\n${refs_modules}")
endif()

# writing file
configure_file(${RMVL_DOXYFILE_IN} ${doxyfile} @ONLY)
configure_file(root.markdown.in ${rootfile} @ONLY)
