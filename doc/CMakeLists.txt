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
set(tutorials_dir "${CMAKE_CURRENT_LIST_DIR}/tutorials")
set(tutorials_py_dir "${CMAKE_CURRENT_LIST_DIR}/py_tutorials")
set(doxyfile "${CMAKE_BINARY_DIR}/Doxyfile")
set(bibfile "${CMAKE_CURRENT_LIST_DIR}/rmvl.bib")
set(rootfile "${CMAKE_BINARY_DIR}/root.markdown")
set(introfile "${CMAKE_CURRENT_LIST_DIR}/intro.markdown")

# set export variables
string(REPLACE ";" " \\\n" CMAKE_DOXYGEN_INPUT_LIST "${paths_include}; ${rootfile}; ${introfile}; ${tutorials_dir}; ${tutorials_py_dir}")
string(REPLACE ";" " \\\n" CMAKE_DOXYGEN_INCLUDE_ROOTS "${paths_include}")
string(REPLACE ";" " \\\n" CMAKE_DOXYGEN_TEST_ROOTS "${paths_test}")
string(REPLACE ";" " \\\n" CMAKE_DOXYGEN_EXAMPLE_PATH "${example_path}")
set(CMAKE_DOXYGEN_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
set(CMAKE_EXTRA_BIB_FILES "${bibfile}")

list(APPEND CMAKE_DOXYGEN_HTML_FILES "${CMAKE_CURRENT_LIST_DIR}/rmvl-logo-small.png")
list(APPEND CMAKE_DOXYGEN_HTML_FILES "${CMAKE_CURRENT_LIST_DIR}/tutorial-utils.js")
string(REPLACE ";" " \\\n" CMAKE_DOXYGEN_HTML_FILES "${CMAKE_DOXYGEN_HTML_FILES}")

set(
  RMVL_DOXYFILE_IN Doxyfile.in
  CACHE PATH "RMVL_DOXYFILE_IN"
)

# pre-defined variables (cpp)
rmvl_update_doxygen_predefined("__cplusplus=202300L")
rmvl_update_doxygen_predefined("__cpp_lib_generator=202207L")
rmvl_update_doxygen_predefined("__cpp_lib_math_constants=201907L")

# List all the modules
set(refs_extra)
set(refs_modules)
foreach(m ${RMVL_EXTRA_MODULE} ${RMVL_MAIN_MODULE})
  # Reference entry
  set(one_ref "\t- ${m}. @ref ${m}\n")
  list(FIND RMVL_MAIN_MODULE ${m} _pos)
  if(${_pos} EQUAL -1)
    set(refs_extra "${refs_extra}${one_ref}")
  else()
    set(refs_modules "${refs_modules}${one_ref}")
  endif()
endforeach()

if(refs_extra)
  set(refs_extra "- Extra modules. 扩展模块\n${refs_extra}")
endif()
if(refs_modules)
  set(refs_modules "- Main modules. 主要模块\n${refs_modules}")
endif()

# writing file
configure_file(${RMVL_DOXYFILE_IN} ${doxyfile} @ONLY)
configure_file(root.markdown.in ${rootfile} @ONLY)

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
  add_custom_target(
    doxygen
    COMMENT "Generating Doxygen documentation"
  )

  set(DOXYGEN_VERSION "${DOXYGEN_VERSION}" CACHE INTERNAL "Doxygen version")
  add_custom_target(
    doxygen_cpp
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
    COMMENT "Generate Doxygen documentation for C++"
  )
  
  if(BUILD_PYTHON)
    macro(add_fns ns fn arg ret)
      if(NOT "${tmp_fns}" STREQUAL "")
        set(tmp_fns "${tmp_fns}#")
      endif()
      set(tmp_fns "${tmp_fns}${ns}#${fn}#${arg}#${ret}")
    endmacro()

    macro(add_cst ns n val)
      if(NOT "${tmp_consts}" STREQUAL "")
        set(tmp_consts "${tmp_consts}#")
      endif()
      set(tmp_consts "${tmp_consts}${ns}#${n}#${val}")
    endmacro()

    unset(file_content)
    # read from pyrmvl_fns.cfg
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/tools/pyrmvl_fns.cfg" file_content)
    string(REPLACE "\n" ";" file_content "${file_content}")
    foreach(line ${file_content})
      if(line MATCHES "^#")
        continue()
      endif()
      string(REGEX REPLACE " +" ";" line "${line}")
      string(REPLACE "\"" "" line "${line}")
      list(LENGTH line length)
      if(NOT length EQUAL 4)
        continue()
      endif()
      list(GET line 0 ns)
      list(GET line 1 fn)
      list(GET line 2 arg)
      list(GET line 3 ret)
      add_fns("${ns}" "${fn}" "${arg}" "${ret}")
    endforeach()

    # read from pyrmvl_cst.cfg
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/tools/pyrmvl_cst.cfg" file_content)
    string(REPLACE "\n" ";" file_content "${file_content}")
    foreach(line ${file_content})
      if(line MATCHES "^#")
        continue()
      endif()
      string(REGEX REPLACE " +" ";" line "${line}")
      string(REPLACE "\"" "" line "${line}")
      list(LENGTH line length)
      if(NOT length EQUAL 3)
        continue()
      endif()
      list(GET line 0 ns)
      list(GET line 1 n)
      list(GET line 2 val)
      add_cst("${ns}" "${n}" "${val}")
    endforeach()

    set(RMVL_PYTHON_SIGNATURES_FILE "${CMAKE_CURRENT_BINARY_DIR}/pyrmvl_signatures.json")
    add_custom_target(gen_pyrmvl_clear
      COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/gen.py" "${RMVL_PYTHON_SIGNATURES_FILE}" "--clear"
      COMMENT "Clearing rmvl python bindings signatures"
    )

    add_custom_target(gen_pyrmvl_signatures
      COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/gen.py" "${RMVL_PYTHON_SIGNATURES_FILE}" "--fns=${tmp_fns}" "--consts=${tmp_consts}"
      COMMENT "Generating rmvl python bindings signatures"
      DEPENDS gen_pyrmvl_clear
    )

    add_custom_target(doxygen_python
      COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tools/add_signatures.py" "${CMAKE_CURRENT_BINARY_DIR}/doxygen/html/" "${RMVL_PYTHON_SIGNATURES_FILE}" "python"
      DEPENDS doxygen_cpp gen_pyrmvl_signatures
      COMMENT "Inject Python signatures into documentation"
    )
  endif()

  add_dependencies(doxygen doxygen_cpp)
  if(BUILD_PYTHON)
    add_dependencies(doxygen doxygen_python)
  endif()

else()
  message(STATUS "Doxygen not found. Documentation will not be generated.")
  unset(DOXYGEN_VERSION CACHE)
endif()

install(
  DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doxygen/html
  DESTINATION "${RMVL_DOC_INSTALL_PATH}"
  OPTIONAL
)
