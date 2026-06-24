# =====================================================================================
# Python 代码生成模块，包含以下功能：
#
#   1. rmvl_generate_python: 根据给定目标生成 Python 绑定代码与接口文件
#   2. rmvl_finalize_python_generation: 聚合 Python 接口与文档配置
#
# =====================================================================================

# ----------------------------------------------------------------------------
#   根据给定目标生成 Python 绑定代码与接口文件
#   用法:
#     rmvl_generate_python(
#       <name>                          # 目标名称，将生成 rmvl_light_py 模块
#       [FILES <file1> [<file2> ...]]   # 参与绑定的 include/ 文件夹下的头文件
#       [EXT_NS <extra_namespace>]      # 额外的命名空间，已内置 rm 和 rm::para
#       [DEPENDS <dep1> [<dep2> ...]]   # 依赖的模块
#       [PREBIND <bind1> [<bind2> ...]] # 使用预绑定代码段
#     )
#   示例:
#     rmvl_generate_python(io
#       FILES rmvl/io/util.hpp rmvl/io/ipc.hpp rmvl/io/serial.hpp
#       EXT_NS rm::async
#       DEPENDS io
#       PREBIND json
#     )
# ----------------------------------------------------------------------------
function(rmvl_generate_python _name)
  set(msg_prefix "Generating Python bindings code for ${_name}")
  message(STATUS "${msg_prefix}")
  if(DEFINED BUILD_rmvl_${_name}_INIT AND NOT BUILD_rmvl_${_name}_INIT)
    message(STATUS "${msg_prefix} - skipped")
    return()
  endif()
  set(multi_value "EXT_NS" "DEPENDS" "FILES" "PREBIND")
  cmake_parse_arguments("PY" "${options}" "" "${multi_value}" ${ARGN})

  set(pybind_ws "${PROJECT_SOURCE_DIR}/cmake/templates/python")
  set(pybind_inc "${CMAKE_CURRENT_SOURCE_DIR}/include")
  set(pygen_script "${pybind_ws}/rmvl_pygen.py")

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

  # append extra namespaces
  foreach(ns ${PY_EXT_NS})
    set(RMVL_PYBIND_NS "${RMVL_PYBIND_NS}\nusing namespace ${ns};")
  endforeach()
  
  # set prebind contents
  if(PY_PREBIND)
    foreach(pb ${PY_PREBIND})
      rmvl_cmake_configure("${PROJECT_SOURCE_DIR}/cmake/templates/python/prebind/${pb}.cpp.in" pbctx @ONLY)
      set(pbctx "// ------------------ Prebind: ${pb} ------------------\n${pbctx}\n// ----------------------------------------------------")
      set(PREBIND_CONTENTS "${PREBIND_CONTENTS}\n${pbctx}")
    endforeach()
  else()
    set(PREBIND_CONTENTS "////////// No prebind contents //////////")
  endif()
  
  # Configure a stable wrapper and generate its contents during the build.
  set(RMVL_PYBIND_NAME "rm_${_name}_py")
  set(pybind_fragment "${RMVL_PYBIND_OUTPUT_DIR}/${RMVL_PYBIND_NAME}.inc")
  set(pyi_fragment "${RMVL_PYTHON_OUTPUT_DIR}/${RMVL_PYBIND_NAME}.typing.inc")
  set(RMVL_PYBIND_CONTENTS "#include \"${RMVL_PYBIND_NAME}.inc\"")
  configure_file(
    ${pybind_ws}/rmvl_pybind.cpp.in
    ${RMVL_PYBIND_OUTPUT_DIR}/${RMVL_PYBIND_NAME}.cpp
    @ONLY
  )

  set(pygen_misc_args "")
  set(pygen_misc_depends "")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/misc/python.json")
    set(pygen_misc "${CMAKE_CURRENT_SOURCE_DIR}/misc/python.json")
    list(APPEND pygen_misc_args "--misc=${pygen_misc}")
    list(APPEND pygen_misc_depends "${pygen_misc}")
  endif()
  add_custom_command(
    OUTPUT "${pybind_fragment}" "${pyi_fragment}"
    COMMAND ${RMVL_PYTHON_EXECUTABLE} "${pygen_script}"
            ${pybind_headers} all
            "--bind-output=${pybind_fragment}"
            "--pyi-output=${pyi_fragment}"
            "--module-name=${_name}"
            ${pygen_misc_args}
    DEPENDS ${pybind_headers} "${pygen_script}" ${pygen_misc_depends}
    COMMENT "Generating Python bindings and typing for ${_name}"
    VERBATIM
  )
  set_source_files_properties(
    "${pybind_fragment}" "${pyi_fragment}"
    PROPERTIES GENERATED TRUE HEADER_FILE_ONLY TRUE
  )
  set(pygen_target "${RMVL_PYBIND_NAME}_codegen")
  add_custom_target(
    ${pygen_target}
    DEPENDS "${pybind_fragment}" "${pyi_fragment}"
  )
  pybind11_add_module(
    ${RMVL_PYBIND_NAME}
    ${RMVL_PYBIND_OUTPUT_DIR}/${RMVL_PYBIND_NAME}.cpp
    "${pybind_fragment}"
  )
  add_dependencies(${RMVL_PYBIND_NAME} ${pygen_target})

  set_property(GLOBAL APPEND PROPERTY RMVL_PYTHON_TYPING_FRAGMENTS "${pyi_fragment}")
  set_property(GLOBAL APPEND PROPERTY RMVL_PYTHON_CODEGEN_TARGETS "${pygen_target}")
  set_property(GLOBAL APPEND PROPERTY RMVL_PYTHON_HEADERS ${pybind_headers})

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
  
  # generate __init__.py, rmvl_typing.pyi and submodules *.pyi file content
  file(APPEND ${RMVL_PYTHON_OUTPUT_DIR}/__init__.py "from .${RMVL_PYBIND_NAME} import *\n")
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

# ----------------------------------------------------------------------------
#   Finalize generated Python files after all modules have been visited.
# ----------------------------------------------------------------------------
function(rmvl_finalize_python_generation)
  get_property(pyi_fragments GLOBAL PROPERTY RMVL_PYTHON_TYPING_FRAGMENTS)
  get_property(pygen_targets GLOBAL PROPERTY RMVL_PYTHON_CODEGEN_TARGETS)
  if(NOT pyi_fragments)
    return()
  endif()

  set(pygen_script "${PROJECT_SOURCE_DIR}/cmake/templates/python/rmvl_pygen.py")
  set(pyi_base "${RMVL_PYTHON_OUTPUT_DIR}/rmvl_typing.base.pyi")
  set(pyi_output "${RMVL_PYTHON_OUTPUT_DIR}/rmvl_typing.pyi")
  # CMake 3.16 Makefiles cannot consume a generated file rule from another
  # directory. Depend on per-module codegen targets instead of the fragments.
  add_custom_target(rmvl_python_typing ALL
    COMMAND ${RMVL_PYTHON_EXECUTABLE} "${pygen_script}"
            ${pyi_fragments} aggregate
            "--base=${pyi_base}"
            "--pyi-output=${pyi_output}"
    DEPENDS "${pyi_base}" "${pygen_script}"
    BYPRODUCTS "${pyi_output}"
    COMMENT "Aggregating Python typing declarations"
    VERBATIM
  )
  add_dependencies(rmvl_python_typing ${pygen_targets})

  if(BUILD_DOCS)
    get_property(pybind_headers GLOBAL PROPERTY RMVL_PYTHON_HEADERS)
    list(REMOVE_DUPLICATES pybind_headers)
    execute_process(
      COMMAND ${RMVL_PYTHON_EXECUTABLE} "${pygen_script}"
              ${pybind_headers} docs "--doc=${RMVL_PYDOC_OUTPUT_DIR}"
      RESULT_VARIABLE pygen_result
      ERROR_VARIABLE pygen_error
    )
    if(NOT pygen_result EQUAL 0)
      message(FATAL_ERROR "Failed to generate Python documentation configuration: ${pygen_error}")
    endif()
  endif()
endfunction()
