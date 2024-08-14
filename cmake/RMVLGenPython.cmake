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
# ----------------------------------------------------------------------------
function(_pygen headers mode outputs)
  foreach(header ${headers})
    execute_process(
      COMMAND ${RMVL_PYTHON_EXECUTABLE} ${pybind_ws}/rmvl_pygen.py ${header} ${mode}
      OUTPUT_VARIABLE py_output
    )
    set(ret_outputs "${ret_outputs}\n${py_output}")
  endforeach()
  set(${outputs} "${ret_outputs}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------
#   根据给定目标生成 Python 绑定代码与接口文件
#   用法:
#     rmvl_generate_python(<name> [DEPENDS <dependencies>])
#   示例:
#     rmvl_generate_python(
#       light                     # 目标名称，将生成 rmvl_light_py 模块
#       DEPENDS opt_light_control # 依赖的模块
#     )
# ----------------------------------------------------------------------------
function(rmvl_generate_python _name)
  cmake_parse_arguments("PY" "" "" "DEPENDS" ${ARGN})

  set(pybind_ws "${PROJECT_SOURCE_DIR}/cmake/python")
  set(pybind_inc "${CMAKE_CURRENT_SOURCE_DIR}/include/rmvl")

  # obtain all the header files
  if(IS_DIRECTORY ${pybind_inc}/${_name})
    file(GLOB pybind_headers ${pybind_inc}/${_name}/*.h*)
  else()
    set(pybind_headers ${pybind_inc}/${_name}.hpp)
  endif()
  
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
  _pygen("${pybind_headers}" "pyi" RMVL_PYI_CONTENTS)
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
