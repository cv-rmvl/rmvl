# =====================================================================================
# 参数生成模块，包含以下主要功能：
#
#   1. rmvl_generate_para:        根据给定目标及对应的参数规范文件 *.para 生成 C++ 文件
#   2. rmvl_generate_module_para: 根据给定模块下的所有 para 目标生成 C++ 文件
#
# 以及以下次要功能：
#
#   1. system_date:   获取系统日期
# =====================================================================================

# ----------------------------------------------------------------------------
#   获取系统日期
#   用法:
#     system_date(
#       <output year> <output month> <output day>
#     )
#   示例:
#     system_date(
#       year  # 年份，格式为 yyyy
#       month # 月份，格式为 mm
#       day   # 日期，格式为 dd
#     )
# ----------------------------------------------------------------------------
function(system_date out_y out_m out_d)
  if(UNIX)
    execute_process(
      COMMAND date "+%Y-%m-%d"
      OUTPUT_VARIABLE date OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  elseif(WIN32)
    execute_process(
      COMMAND cmd /c date /t
      OUTPUT_VARIABLE date
    )
  endif()
  # split
  string(SUBSTRING ${date} 0 4 year)
  string(SUBSTRING ${date} 5 2 month)
  string(SUBSTRING ${date} 8 2 day)
  set(${out_y} ${year} PARENT_SCOPE)
  set(${out_m} ${month} PARENT_SCOPE)
  set(${out_d} ${day} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------
#   修正类型符号: 增加 C++ 的作用域
#     string   -> std::string     vector   -> std::vector
#     Point... -> cv::Point...    Matx...  -> cv::Matx...
#   用法:
#     _type_correct(
#       <value_type> <out_value_type>
#     )
#   示例:
#     _type_correct(
#       "${type_sym}" # 传入字符串
#       type_sym      # 传出字符串: 已经修正过的字符串
#     )
# ----------------------------------------------------------------------------
function(_type_correct value_type out_value_type)
  if(NOT WITH_OPENCV)
    set(cmt_prefix "// ")
  endif()
  set(retval ${value_type})
  string(REGEX REPLACE "(size_t|string|vector)" "std::\\1" retval "${retval}")
  string(REGEX REPLACE "(Point|Vec|Mat)" "${cmt_prefix}cv::\\1" retval "${retval}")
  string(REGEX REPLACE "Matx([1-9])([1-9])f" "${cmt_prefix}Matx<float,\\1,\\2>" retval "${retval}")
  string(REGEX REPLACE "Matx([1-9])([1-9])d" "${cmt_prefix}Matx<double,\\1,\\2>" retval "${retval}")
  string(REGEX REPLACE "Vec([1-9])f" "${cmt_prefix}Vec<float,\\1>" retval "${retval}")
  string(REGEX REPLACE "Vec([1-9])d" "${cmt_prefix}Vec<double,\\1>" retval "${retval}")
  set(${out_value_type} ${retval} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------
#   按照常规的赋值模式解析参数规范文件的某一行内容
#   用法:
#     _parse_assign(
#       <line_str> <header> <source>
#     )
#   示例:
#     _parse_assign(
#       line_str         # 传入字符串: 一行的内容
#       ret_header       # 传出字符串: 头文件内容
#       ret_source_read  # read 函数传出字符串: 源文件内容
#       ret_source_write # write 函数传出字符串: 源文件内容
#     )
# ----------------------------------------------------------------------------
function(_parse_assign content_line header_line source_read_line source_write_line)
  list(LENGTH ${content_line} l)
  if(l GREATER 1)
    # 获取值类型符号
    list(GET ${content_line} 0 type_sym)
    # 修正值类型符号
    _type_correct("${type_sym}" type_sym_correct)
    # 获取标识符
    list(GET ${content_line} 1 id_sym)
    # 获取默认值和注释
    if(l GREATER 2)
      list(SUBLIST ${content_line} 2 -1 default_cmt)
    else()
      set(default_cmt "")
    endif()
    string(REGEX REPLACE ";" "" default_cmt "${default_cmt}")
    # 分离默认值和注释
    string(FIND "${default_cmt}" "#" cmt_idx)
    if(cmt_idx EQUAL -1)
      set(default_sym "${default_cmt}")
      set(comment_sym "${id_sym}")
    else()
      string(SUBSTRING "${default_cmt}" 0 ${cmt_idx} default_sym)
      math(EXPR cmt_idx "${cmt_idx} + 1")
      string(SUBSTRING "${default_cmt}" ${cmt_idx} -1 comment_sym)
    endif()
    # 添加默认值提示到注释中
    if(NOT default_sym STREQUAL "")
      set(comment_sym "${comment_sym} @note 默认值：`${default_sym}`")
    endif()
    # 修正非 string 类型的默认值
    if(NOT type_sym STREQUAL "string")
      _type_correct("${default_sym}" default_sym)
      string(REGEX REPLACE "," ", " default_sym "${default_sym}")
    endif()
  else()
    return()
  endif()
  # 获取 Header 部分的返回值
  set(ret_header_line "${ret_header_line}    //! ${comment_sym}\n")
  if("${default_sym}" STREQUAL "")
    set(ret_header_line "${ret_header_line}    ${type_sym_correct} ${id_sym}{};\n")
  else()
    set(ret_header_line "${ret_header_line}    ${type_sym_correct} ${id_sym} = ${default_sym};\n")
  endif()
  # 获取 Source 部分的返回值
  set(ret_source_read_line "${ret_source_read_line}    node = fs[\"${id_sym}\"];\n")
  if(type_sym MATCHES "^uint\\w*|size_t")
    set(ret_source_read_line "${ret_source_read_line}    if (!node.isNone())\n    {\n")
    set(ret_source_read_line "${ret_source_read_line}        int tmp{};\n        node >> tmp;\n")
    set(ret_source_read_line "${ret_source_read_line}        ${id_sym} = static_cast<${type_sym_correct}>(tmp);\n    }\n")
    set(ret_source_write_line "${ret_source_write_line}    int tmp_${id_sym} = static_cast<int>(${id_sym});\n")
    set(ret_source_write_line "${ret_source_write_line}    fs << \"${id_sym}\" << tmp_${id_sym};\n")
  elseif(type_sym MATCHES "int|float|double|string|vector|Point\\w*|Mat\\w*|Vec\\w*")
    set(ret_source_read_line "${ret_source_read_line}    node.isNone() ? void(0) : (node >> ${id_sym});\n")
    set(ret_source_write_line "${ret_source_write_line}    fs << \"${id_sym}\" << ${id_sym};\n")
  else()
    set(ret_source_read_line "${ret_source_read_line}    if (!node.isNone())\n    {\n")
    set(ret_source_read_line "${ret_source_read_line}        std::string tmp{};\n        node >> tmp;\n")
    set(ret_source_read_line "${ret_source_read_line}        ${id_sym} = s2t_${type_sym}.at(tmp);\n    }\n")
    set(ret_source_write_line "${ret_source_write_line}    fs << \"${id_sym}\" << t2s_${type_sym}.at(${id_sym});\n")
  endif()
  # 作用域提升
  set(${header_line} "${ret_header_line}" PARENT_SCOPE)
  set(${source_read_line} "${ret_source_read_line}" PARENT_SCOPE)
  set(${source_write_line} "${ret_source_write_line}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------
#   按照枚举定义模式解析参数规范文件的某一行内容
#   用法:
#     _parse_enumdef(
#       <line_str> <name of the enum>
#       <header_enum_line>
#       <source_enum_s2t_line> <source_enum_t2s_line>
#     )
#   示例:
#     _parse_enumdef(
#       line_str             # [in]  一行的内容
#       enum_name            # [in]  枚举名称
#       header_enum_line     # [out] 头文件额外内容
#       source_enum_s2t_line # [out] 源文件 string->tag 的额外内容
#       source_enum_t2s_line # [out] 源文件 tag->string 的额外内容
#     )
# ----------------------------------------------------------------------------
function(_parse_enumdef content_line enum_name header_enum_line source_enum_s2t_line source_enum_t2s_line)
  list(LENGTH ${content_line} l)
  # 获取标签符号
  list(GET ${content_line} 0 tag_sym)
  # 获取参考值和注释
  if(l GREATER 1)
    list(SUBLIST ${content_line} 1 -1 ref_cmt)
  else()
    set(ref_cmt "")
  endif()
  string(REGEX REPLACE ";" "" ref_cmt "${ref_cmt}")
  # 分离参考值和注释
  string(FIND "${ref_cmt}" "#" cmt_idx)
  if(cmt_idx EQUAL -1)
    set(ref_sym "${ref_cmt}")
    set(comment_sym "${tag_sym}")
  else()
    string(SUBSTRING "${ref_cmt}" 0 ${cmt_idx} ref_sym)
    math(EXPR cmt_idx "${cmt_idx} + 1")
    string(SUBSTRING "${ref_cmt}" ${cmt_idx} -1 comment_sym)
  endif()
  # 获取 Extra Header 部分的返回值
  set(ret_header_enum_line "    //! ${comment_sym}\n")
  if("${ref_sym}" STREQUAL "")
    set(ret_header_enum_line "${ret_header_enum_line}    ${tag_sym},\n")
  else()
    set(ret_header_enum_line "${ret_header_enum_line}    ${tag_sym} = ${ref_sym},\n")
  endif()
  # 获取 Extra Source 部分的返回值
  set(ret_source_enum_s2t_line "    {\"${tag_sym}\", ${enum_name}::${tag_sym}},\n")
  set(ret_source_enum_t2s_line "    {${enum_name}::${tag_sym}, \"${tag_sym}\"},\n")
  # 作用域提升
  set(${header_enum_line} "${ret_header_enum_line}" PARENT_SCOPE)
  set(${source_enum_s2t_line} "${ret_source_enum_s2t_line}" PARENT_SCOPE)
  set(${source_enum_t2s_line} "${ret_source_enum_t2s_line}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------
#   将指定的 *.para 参数规范文件解析成 C++ 风格的内容
#   用法:
#     _para_parser(
#       <file_name>
#       <header_details> <source_read> <source_write>
#       <header_enum> <source_enum_s2t> <source_enum_t2s>
#       <status>
#     )
#   示例:
#     _para_parser(core.para # 名为 core.para 的参数规范文件
#       para_header_details  # 对应 .h/.hpp 文件的细节
#       para_source_read     # 对应 .cpp 文件 read 函数的实现细节
#       para_source_write    # 对应 .cpp 文件 write 函数的实现细节
#       para_header_enum     # 对应 .h/.hpp 文件 enum 部分的细节
#       para_source_enum_s2t # 对应 .cpp 文件 enum 部分 string->tag 的细节
#       para_source_enum_t2s # 对应 .cpp 文件 enum 部分 tag->string 的细节
#       status               # 返回值: 解析是否成功，成功返回 TRUE，失败返回 FALSE
#     )
# ----------------------------------------------------------------------------
function(_para_parser file_name header_details source_read source_write header_enum source_enum_s2t source_enum_t2s status)
  # 初始化返回值
  file(READ ${file_name} out_val)
  if(NOT out_val)
    set(${status} FALSE PARENT_SCOPE)
    return()
  endif()
  string(REGEX REPLACE "\n" ";" out_val "${out_val}")
  # 解析每一行
  foreach(substr ${out_val})
    ################ get subing: line_str ################
    string(REGEX REPLACE "[ =]" ";" line_str "${substr}")
    set(tmp)
    foreach(word ${line_str})
      list(APPEND tmp "${word}")
    endforeach()
    set(line_str ${tmp})
    unset(tmp)
    # 判断解析模式
    if(line_str MATCHES "^enum")
      list(GET line_str 1 enum_name)
      string(REGEX REPLACE ";" "" enum_cmt "${line_str}")
      # 获取枚举声明的注释
      string(FIND "${enum_cmt}" "#" cmt_idx)
      if(cmt_idx EQUAL -1)
        set(enum_cmt "${enum_name} 枚举类型")
      else()
        math(EXPR cmt_idx "${cmt_idx} + 1")
        string(SUBSTRING "${enum_cmt}" ${cmt_idx} -1 enum_cmt)
      endif()
      set(ret_header_enum "${ret_header_enum}//! ${enum_cmt}\nenum class ${enum_name}\n{\n")
      set(ret_source_enum_s2t "${ret_source_enum_s2t}static const std::unordered_map<std::string, ${enum_name}> s2t_${enum_name} = {\n")
      set(ret_source_enum_t2s "${ret_source_enum_t2s}static const std::unordered_map<${enum_name}, std::string> t2s_${enum_name} = {\n")
      set(parse_mode "enum")
      continue()
    elseif(line_str MATCHES "^endenum")
      set(ret_header_enum "${ret_header_enum}};\n")
      set(ret_source_enum_s2t "${ret_source_enum_s2t}};\n")
      set(ret_source_enum_t2s "${ret_source_enum_t2s}};\n")
      unset(parse_mode)
      continue()
    endif()
    # 按照不同的模式进行解析
    unset(ret_header_enum_line)
    unset(ret_source_enum_s2t_line)
    unset(ret_source_enum_t2s_line)
    unset(ret_header_line)
    unset(ret_source_read_line)
    unset(ret_source_write_line)
    if(line_str MATCHES "^#")
      continue()
    elseif("${parse_mode}" STREQUAL "enum")
      _parse_enumdef(
        line_str "${enum_name}"
        ret_header_enum_line
        ret_source_enum_s2t_line ret_source_enum_t2s_line
      )
      set(ret_header_enum "${ret_header_enum}${ret_header_enum_line}")
      set(ret_source_enum_s2t "${ret_source_enum_s2t}${ret_source_enum_s2t_line}")
      set(ret_source_enum_t2s "${ret_source_enum_t2s}${ret_source_enum_t2s_line}")
    else()
      _parse_assign(line_str ret_header_line ret_source_read_line ret_source_write_line)
      set(ret_header "${ret_header}${ret_header_line}")
      set(ret_source_read "${ret_source_read}${ret_source_read_line}")
      set(ret_source_write "${ret_source_write}${ret_source_write_line}")
    endif()
  endforeach(substr ${out_val})
  set(${header_enum} "${ret_header_enum}" PARENT_SCOPE)
  set(${source_enum_s2t} "${ret_source_enum_s2t}" PARENT_SCOPE)
  set(${source_enum_t2s} "${ret_source_enum_t2s}" PARENT_SCOPE)
  set(${header_details} "${ret_header}" PARENT_SCOPE)
  set(${source_read} "${ret_source_read}" PARENT_SCOPE)
  set(${source_write} "${ret_source_write}" PARENT_SCOPE)
  set(${status} TRUE PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------
#   根据指定的目标名在 param 文件夹下对应的 *.para 参数规范文件和可选的模块名生成对应的 C++ 代码
#   用法:
#     rmvl_generate_para(
#       <target_name>
#       [MODULE module_name]
#     )
#   示例:
#     rmvl_generate_para(
#       mytarget        # 目标名称
#       MODULE mymodule # 模块名称为 mymodule
#     )
# ----------------------------------------------------------------------------
function(rmvl_generate_para target_name)
  set(one_value MODULE)
  cmake_parse_arguments(PARA "" "${one_value}" "" ${ARGN})
  ########################### message begin ###########################
  if("${PARA_MODULE}" STREQUAL "")
    set(module_name "${target_name}")
  else()
    set(module_name "${PARA_MODULE}")
  endif()
  set(file_name "param/${target_name}.para")
  set(para_msg "Performing Conversion ${target_name}.para")
  message(STATUS "${para_msg}")
  if(DEFINED BUILD_${the_module}_INIT AND NOT BUILD_${the_module}_INIT)
    message(STATUS "${para_msg} - skipped")
    return()
  endif()
  ################## snake to camel (get class name) ##################
  string(REGEX REPLACE "_" ";" para_name_cut "${target_name}_param")
  set(class_name "")
  foreach(_sub ${para_name_cut})
    string(SUBSTRING ${_sub} 0 1 first_c)
    string(TOUPPER ${first_c} first_c)
    string(SUBSTRING ${_sub} 1 -1 remain_c)
    list(APPEND class_name "${first_c}${remain_c}")
    string(REGEX REPLACE ";" "" class_name "${class_name}")
  endforeach()
  ###################### Generate C++ class file ######################
  system_date(year month day)
  string(FIND "${RMVLPARA_${module_name}}" "${target_name}" target_idx)
  if(target_idx EQUAL -1)
    set(RMVLPARA_${module_name} "${RMVLPARA_${module_name}}" "${target_name}" CACHE INTERNAL "${module_name} parameters")
  endif()  
  # parse *.para file
  _para_parser(
    ${file_name}
    para_header_details para_source_read para_source_write
    para_header_enum para_source_enum_s2t para_source_enum_t2s
    para_status
  )
  if(NOT para_status)
    message(STATUS "${para_msg} - failed")
    return()
  endif()
  set(para_include_path)
  # has module
  if(PARA_MODULE)
    set(header_ext "h")
    set(para_include_path "rmvlpara/${module_name}/${target_name}.${header_ext}")
    if(WITH_OPENCV)
      configure_file(
        ${para_template_path}/para_generator_source.in
        ${CMAKE_CURRENT_LIST_DIR}/src/${target_name}/para/param.cpp
        @ONLY
      )
    endif()
  # dosen't have module
  else()
    set(header_ext "hpp")
    set(para_include_path "rmvlpara/${module_name}.${header_ext}")
    if(WITH_OPENCV)
      configure_file(
        ${para_template_path}/para_generator_source.in
        ${CMAKE_CURRENT_LIST_DIR}/src/para/param.cpp
        @ONLY
      )
    endif()
    set(def_new_group "${def_new_group}//! @addtogroup para\n//! @{\n")
    set(def_new_group "${def_new_group}//! @defgroup para_${module_name} ${module_name} 的参数模块\n")
    set(def_new_group "${def_new_group}//! @addtogroup para_${module_name}\n//! @{\n")
    set(def_new_group "${def_new_group}//! @brief 与 @ref ${module_name} 相关的参数模块，包含...\n")
    set(def_new_group "${def_new_group}//! @} para_${module_name}\n//! @} para\n")
  endif()
  if(WITH_OPENCV)
    configure_file(
      ${para_template_path}/para_generator_header.in
      ${CMAKE_CURRENT_LIST_DIR}/include/${para_include_path}
      @ONLY
    )
  else()
    configure_file(
      ${para_template_path}/para_generator_header_without_cv.in
      ${CMAKE_CURRENT_LIST_DIR}/include/${para_include_path}
      @ONLY
    )
  endif()
  unset(para_include_path)
  ############################ message end ############################
  message(STATUS "${para_msg} - done")
endfunction()

# ----------------------------------------------------------------------------
#   根据给定模块下所有的 para 目标，生成对应的 C++ 代码
#   用法:
#     rmvl_generate_module_para(
#       <module_name>
#     )
#   示例:
#     rmvl_generate_module_para(combo)
# ----------------------------------------------------------------------------
function(rmvl_generate_module_para module_name)
  ########################### message begin ###########################
  set(para_msg "Performing Conversion ${module_name} Module")
  message(STATUS "${para_msg}")
  ######################## Generate C++ header ########################
  system_date(year month day)
  set(para_module_header_details "")
  foreach(_sub ${RMVLPARA_${module_name}})
    string(TOUPPER "${_sub}" _upper)
    set(para_module_header_details "${para_module_header_details}\n#ifdef HAVE_RMVL_${_upper}\n")
    set(para_module_header_details "${para_module_header_details}#include \"${module_name}/${_sub}.h\"\n")
    set(para_module_header_details "${para_module_header_details}#endif // HAVE_RMVL_${_upper}\n")
  endforeach()
  # generate C++ file
  configure_file(
    ${para_template_path}/para_generator_module.in
    ${CMAKE_CURRENT_LIST_DIR}/include/rmvlpara/${module_name}.hpp
    @ONLY
  )
  ############################ message end ############################
  message(STATUS "${para_msg} - done")
endfunction()
