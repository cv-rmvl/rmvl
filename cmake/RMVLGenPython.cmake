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
    set(pydoc_path "--doc=${PYDOC_DOC_PATH}")
  endif()
  set(misc_path "")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/misc/python.json")
    set(misc_path "--misc=${CMAKE_CURRENT_SOURCE_DIR}/misc/python.json")
  endif()
  foreach(header ${headers})
    execute_process(
      COMMAND ${RMVL_PYTHON_EXECUTABLE} ${pybind_ws}/rmvl_pygen.py ${header} ${mode} ${pydoc_path} ${misc_path}
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
#       [FILES <file1> [<file2> ...]] # 参与绑定的 include/ 文件夹下的头文件
#       [DEPENDS <dep1> [<dep2> ...]] # 依赖的模块
#       [USE_PREBIND]                 # 使用预绑定代码段，主要涉及到 OpenCV 的绑定
#     )
#   示例:
#     rmvl_generate_python(core
#       FILES core/io.hpp core/timer.hpp
#       DEPENDS core
#       USE_PREBIND
#     )
# ----------------------------------------------------------------------------
function(rmvl_generate_python _name)
  set(msg_prefix "Generating Python bindings code for ${_name}")
  message(STATUS "${msg_prefix}")
  if(DEFINED BUILD_rmvl_${_name}_INIT AND NOT BUILD_rmvl_${_name}_INIT)
    message(STATUS "${msg_prefix} - skipped")
    return()
  endif()
  set(options "USE_PREBIND")
  set(multi_value "DEPENDS" "FILES")
  cmake_parse_arguments("PY" "${options}" "" "${multi_value}" ${ARGN})

  set(pybind_ws "${PROJECT_SOURCE_DIR}/cmake/templates/python")
  set(pybind_inc "${CMAKE_CURRENT_SOURCE_DIR}/include")

  # obtain the header files
  unset(pybind_headers)
  if(NOT PY_FILES)
    return()
  else()
    foreach(file ${PY_FILES})
      list(APPEND pybind_headers "${pybind_inc}/${file}")
    endforeach()
  endif()
  set(RMVL_PYBIND_NS "using namespace rm;")
  set(RMVL_PYBIND_INC "#include \"rmvl/${_name}.hpp\"")
  if(EXISTS "${pybind_inc}/rmvlpara/${_name}.hpp")
    set(RMVL_PYBIND_INC "${RMVL_PYBIND_INC}\n#include \"rmvlpara/${_name}.hpp\"")
    set(RMVL_PYBIND_NS "${RMVL_PYBIND_NS}\nusing namespace rm::para;")
  endif()
  
  # set prebind contents
  if(PY_USE_PREBIND)
    rmvl_cmake_configure("${PROJECT_SOURCE_DIR}/cmake/templates/python/pre_bind.cpp.in" PREBIND_CONTENTS @ONLY)
  else()
    set(PREBIND_CONTENTS "// No prebind contents")
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

  add_custom_target(${RMVL_PYBIND_NAME}_msg ALL
    COMMENT "Python bindings for '${_name}' generated."
    DEPENDS ${RMVL_PYBIND_NAME}
  )

  if(PY_DEPENDS)
    if(BUILD_WORLD)
      target_link_libraries(${RMVL_PYBIND_NAME} PUBLIC rmvl_world)
    else()
      foreach(dep ${PY_DEPENDS})
        target_link_libraries(${RMVL_PYBIND_NAME} PUBLIC rmvl_${dep})
      endforeach()
    endif()
  endif()

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

  # generate __init__.py, rmvl_typing.pyi and submodules *.pyi file content
  file(APPEND ${RMVL_PYTHON_OUTPUT_DIR}/__init__.py "from .${RMVL_PYBIND_NAME} import *\n")
  set(rmvl_typing_content "# ========== DO NOT MODIFY THIS FILE ! ==========\n# ${_name} module\n# ===============================================\n${RMVL_PYI_CONTENTS}\n")
  file(APPEND ${RMVL_PYTHON_OUTPUT_DIR}/rmvl_typing.pyi "${rmvl_typing_content}")
  set(pyi_content "# DO NOT MODIFY THIS FILE !\nfrom .rmvl_typing import *")
  file(WRITE ${RMVL_PYTHON_OUTPUT_DIR}/${RMVL_PYBIND_NAME}.pyi "${pyi_content}")

  # install python interface file
  install(
    FILES ${RMVL_PYTHON_OUTPUT_DIR}/${RMVL_PYBIND_NAME}.pyi
    DESTINATION ${RMVL_PYTHON_INSTALL_PATH}
  )
  install(
    TARGETS ${RMVL_PYBIND_NAME}
    LIBRARY DESTINATION ${RMVL_PYTHON_INSTALL_PATH}
  )
  message(STATUS "${msg_prefix} - done")
endfunction()
