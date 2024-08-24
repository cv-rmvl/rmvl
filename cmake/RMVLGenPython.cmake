# =====================================================================================
# Python 代码生成模块，包含以下功能：
#
#   1. _pygen: Python 内容生成
#   2. rmvl_generate_python: 根据给定目标生成 Python 绑定代码与接口文件
#
# =====================================================================================

# ----------------------------------------------------------------------------
#   Python 内容生成
#   用法:
#     _pygen(<headers> <mode> <outputs>)
#   示例:
#     _pygen("${pybind_headers}" "bind" RMVL_PYBIND_CONTENTS)
#     _pygen("${pybind_headers}" "pyi" RMVL_PYI_CONTENTS [DOC_PATH <file path>])
# ----------------------------------------------------------------------------
function(_pygen headers mode outputs)
  cmake_parse_arguments("PYDOC" "" "DOC_PATH" "" ${ARGN})
  set(pydoc_path "")
  if(PYDOC_DOC_PATH)
    set(pydoc_path "--doc_path=${PYDOC_DOC_PATH}")
  endif()
  foreach(header ${headers})
    execute_process(
      COMMAND ${RMVL_PYTHON_EXECUTABLE} ${pybind_ws}/rmvl_pygen.py ${header} ${mode} ${pydoc_path}
      OUTPUT_VARIABLE py_output
    )
    set(ret_outputs "${ret_outputs}\n${py_output}")
  endforeach()
  set(${outputs} "${ret_outputs}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------
#   根据给定目标生成 Python 绑定代码与接口文件
#   用法:
#     rmvl_generate_python(
#       <name>                        # 目标名称，将生成 rmvl_light_py 模块
#       [FILES <file1> [<file2> ...]] # 参与绑定的 include/rmvl/ 文件夹下的头文件
#       [DEPENDS <dep1> [<dep2> ...]] # 依赖的模块
#     )
#   示例:
#     rmvl_generate_python(core
#       FILES core/dataio.hpp core/timer.hpp
#       DEPENDS core
#     )
# ----------------------------------------------------------------------------
function(rmvl_generate_python _name)
  set(options "DEPENDS" "FILES")
  cmake_parse_arguments("PY" "" "" "${options}" ${ARGN})

  set(pybind_ws "${PROJECT_SOURCE_DIR}/cmake/templates/python")
  set(pybind_inc "${CMAKE_CURRENT_SOURCE_DIR}/include/rmvl")
  set(pybind_parainc "${CMAKE_CURRENT_SOURCE_DIR}/include/rmvlpara")

  # obtain the header files (rmvl)
  unset(pybind_headers)
  if(NOT PY_FILES)
    return()
  else()
    foreach(file ${PY_FILES})
      list(APPEND pybind_headers "${pybind_inc}/${file}")
    endforeach()
  endif()
  set(RMVL_PYBIND_NS "using namespace rm;")
  set(RMVL_PYBIND_INC "#include <rmvl/${_name}.hpp>")

  # obtain the header files (rmvlpara)
  if(IS_DIRECTORY ${pybind_parainc}/${_name})
    file(GLOB pybind_para_headers ${pybind_parainc}/${_name}/*.h*)
  else()
    if(EXISTS ${pybind_parainc}/${_name}.hpp)
      set(pybind_para_headers ${pybind_parainc}/${_name}.hpp)
    endif()
  endif()
  if(pybind_para_headers)
    set(RMVL_PYBIND_NS "${RMVL_PYBIND_NS}\nusing namespace rm::para;")
    set(RMVL_PYBIND_INC "${RMVL_PYBIND_INC}\n#include <rmvlpara/${_name}.hpp>")
  endif()
  list(APPEND pybind_headers ${pybind_para_headers})
  
  # generate wrapper code for python
  set(RMVL_PYBIND_NAME "rm_${_name}_py")
  _pygen("${pybind_headers}" "bind" RMVL_PYBIND_CONTENTS)
  configure_file(
    ${pybind_ws}/rmvl_pybind.cpp.in
    ${RMVL_PYBIND_OUTPUT_DIR}/${RMVL_PYBIND_NAME}.cpp
    @ONLY
  )
  pybind11_add_module(${RMVL_PYBIND_NAME} ${RMVL_PYBIND_OUTPUT_DIR}/${RMVL_PYBIND_NAME}.cpp)

  foreach(dep ${PY_DEPENDS})
    target_link_libraries(${RMVL_PYBIND_NAME} PRIVATE rmvl_${dep})
  endforeach()

  set_target_properties(
    ${RMVL_PYBIND_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${RMVL_PYTHON_OUTPUT_DIR}
  )
  
  # generate python interface file
  if(BUILD_DOCS)
    _pygen("${pybind_headers}" "pyi" RMVL_PYI_CONTENTS DOC_PATH ${RMVL_PYDOC_OUTPUT_DIR})
  else()
    _pygen("${pybind_headers}" "pyi" RMVL_PYI_CONTENTS)
  endif()
  configure_file(
    ${pybind_ws}/rmvl_pyi.pyi.in
    ${RMVL_PYTHON_OUTPUT_DIR}/${RMVL_PYBIND_NAME}.pyi
    @ONLY
  )

  # generate __init__.py file content
  file(APPEND ${RMVL_PYTHON_OUTPUT_DIR}/__init__.py "from .${RMVL_PYBIND_NAME} import *\n")

  # install python interface file
  install(
    FILES ${RMVL_PYTHON_OUTPUT_DIR}/${RMVL_PYBIND_NAME}.pyi
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${RMVL_PYTHON_INSTALL_SUFFIX}
  )
  install(
    TARGETS ${RMVL_PYBIND_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${RMVL_PYTHON_INSTALL_SUFFIX}
  )
endfunction()
