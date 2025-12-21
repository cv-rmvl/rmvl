# =====================================================================================
# 参数生成模块，包含以下主要功能：
#
#   1. rmvl_generate_para:        根据给定目标及对应的参数规范文件 *.para 生成 C++ 文件
#   2. rmvl_generate_module_para: 根据给定模块下的所有 para 目标生成 C++ 文件
#   3. rmvl_generate_msg:         根据指定的消息描述文件 *.msg 生成 C++ 文件
#
# 以及以下次要功能：
#
#   1. system_date:   获取系统日期
#   2. to_upperfirst: 将字符串的首字母转换为大写
#   3. to_lowerfirst: 将字符串的首字母转换为小写
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
      OUTPUT_VARIABLE date
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  elseif(WIN32)
    execute_process(
      COMMAND powershell -Command "Get-Date -Format 'yyyy-MM-dd'"
      OUTPUT_VARIABLE date
      OUTPUT_STRIP_TRAILING_WHITESPACE
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

system_date(year month day)
set(year ${year} CACHE INTERNAL "year")
set(month ${month} CACHE INTERNAL "month")
set(day ${day} CACHE INTERNAL "day")

# ----------------------------------------------------------------------------
#   将字符串的首字母转换为大写
#   用法:
#     to_upperfirst(
#       <input_string> <output_string>
#     )
#   示例:
#     to_upperfirst(
#       "${input_str}" # 输入字符串
#       output_str     # 输出字符串: 首字母大写
#     )
# ----------------------------------------------------------------------------
function(to_upperfirst input_str output_str)
  string(SUBSTRING ${input_str} 0 1 first_c)
  string(TOUPPER ${first_c} first_c_upper)
  string(SUBSTRING ${input_str} 1 -1 rest_c)
  set(${output_str} "${first_c_upper}${rest_c}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------
#   将字符串的首字母转换为小写
#   用法:
#     to_lowerfirst(
#       <input_string> <output_string>
#     )
#   示例:
#     to_lowerfirst(
#       "${input_str}" # 输入字符串
#       output_str     # 输出字符串: 首字母小写
#     )
# ----------------------------------------------------------------------------
function(to_lowerfirst input_str output_str)
  string(SUBSTRING ${input_str} 0 1 first_c)
  string(TOLOWER ${first_c} first_c_lower)
  string(SUBSTRING ${input_str} 1 -1 rest_c)
  set(${output_str} "${first_c_lower}${rest_c}" PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------
#   将字符串转换为蛇形命名法
#   用法:
#     _to_snake_case(
#       <input_string> <output_string>
#     )
#   示例:
#     _to_snake_case(
#       "ColorRGBA" # 输入字符串
#       output_str  # 输出字符串: color_rgba
#     )
# ----------------------------------------------------------------------------
function(_to_snake_case input_str output_str)
  # Add underscore before capital letters that are preceded by lowercase or digits
  string(REGEX REPLACE "([a-z0-9])([A-Z])" "\\1_\\2" result "${input_str}")
  # Convert to lower case
  # e.g., Color_RGBA -> color_rgba, UInt8 -> uint8
  string(TOLOWER "${result}" result)
  set(${output_str} "${result}" PARENT_SCOPE)
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
    string(REGEX REPLACE ";" " " default_cmt "${default_cmt}")
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
    string(STRIP "${default_sym}" default_sym)
    string(STRIP "${comment_sym}" comment_sym)
    # 添加默认值提示到注释中
    if(NOT default_sym STREQUAL "")
      set(comment_sym "${comment_sym} @details 默认值：`${default_sym}`")
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
    set(ret_header_line "${ret_header_line}    RMVL_W_RW ${type_sym_correct} ${id_sym}{};\n")
  else()
    set(ret_header_line "${ret_header_line}    RMVL_W_RW ${type_sym_correct} ${id_sym} = ${default_sym};\n")
  endif()
  # 获取 Source 部分的返回值
  set(ret_source_read_line "${ret_source_read_line}    _node__ = _fs__[\"${id_sym}\"];\n")
  if(type_sym MATCHES "^bool|uint\\w*|size_t")
    set(ret_source_read_line "${ret_source_read_line}    if (!_node__.isNone())\n    {\n")
    set(ret_source_read_line "${ret_source_read_line}        int tmp{};\n        _node__ >> tmp;\n")
    set(ret_source_read_line "${ret_source_read_line}        ${id_sym} = static_cast<${type_sym_correct}>(tmp);\n    }\n")
    set(ret_source_write_line "${ret_source_write_line}    int tmp_${id_sym} = static_cast<int>(${id_sym});\n")
    set(ret_source_write_line "${ret_source_write_line}    _fs__ << \"${id_sym}\" << tmp_${id_sym};\n")
  elseif(type_sym MATCHES "int|float|double|string|vector|Point\\w*|Mat\\w*|Vec\\w*")
    set(ret_source_read_line "${ret_source_read_line}    _node__.isNone() ? void(0) : (_node__ >> ${id_sym});\n")
    set(ret_source_write_line "${ret_source_write_line}    _fs__ << \"${id_sym}\" << ${id_sym};\n")
  else()
    set(ret_source_read_line "${ret_source_read_line}    if (!_node__.isNone())\n    {\n")
    set(ret_source_read_line "${ret_source_read_line}        std::string tmp{};\n        _node__ >> tmp;\n")
    set(ret_source_read_line "${ret_source_read_line}        ${id_sym} = s2t_${type_sym}.at(tmp);\n    }\n")
    set(ret_source_write_line "${ret_source_write_line}    _fs__ << \"${id_sym}\" << t2s_${type_sym}.at(${id_sym});\n")
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
  string(STRIP "${ref_sym}" ref_sym)
  string(STRIP "${comment_sym}" comment_sym)
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
      set(ret_header_enum "${ret_header_enum}//! ${enum_cmt}\nenum class ${enum_name} {\n")
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
  ############################## message ##############################
  if("${PARA_MODULE}" STREQUAL "")
    set(module_name "${target_name}")
  else()
    set(module_name "${PARA_MODULE}")
  endif()
  set(file_name "param/${target_name}.para")
  set(para_msg "Generating IDL ${target_name}.para")
  if(DEFINED BUILD_rmvl_${target_name}_INIT AND NOT BUILD_rmvl_${target_name}_INIT)
    message(STATUS "${para_msg} - skipped")
    return()
  endif()
  ################## snake to camel (get class name) ##################
  string(REGEX REPLACE "_" ";" para_name_cut "${target_name}_param")
  set(class_name "")
  foreach(_sub ${para_name_cut})
    to_upperfirst("${_sub}" class_name_part)
    string(APPEND class_name "${class_name_part}")
  endforeach()
  ###################### Generate C++ class file ######################
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
        ${codegen_template_path}/para_generator_source.in
        ${CMAKE_CURRENT_LIST_DIR}/src/${target_name}/_rm_codegen_param.cpp
        @ONLY
      )
    endif()
  # dosen't have module
  else()
    set(header_ext "hpp")
    set(para_include_path "rmvlpara/${module_name}.${header_ext}")
    if(WITH_OPENCV)
      configure_file(
        ${codegen_template_path}/para_generator_source.in
        ${CMAKE_CURRENT_LIST_DIR}/src/_rm_codegen_param.cpp
        @ONLY
      )
    endif()
    set(def_new_group "${def_new_group}//! @addtogroup rmvlpara\n//! @{\n")
    set(def_new_group "${def_new_group}//! @defgroup para_${module_name} ${module_name} 的参数模块\n")
    set(def_new_group "${def_new_group}//! @addtogroup para_${module_name}\n//! @{\n")
    set(def_new_group "${def_new_group}//! @brief 与 @ref ${module_name} 相关的参数模块，包含...\n")
    set(def_new_group "${def_new_group}//! @} para_${module_name}\n//! @} rmvlpara\n")
  endif()
  if(WITH_OPENCV)
    configure_file(
      ${codegen_template_path}/para_generator_header.in
      ${CMAKE_CURRENT_LIST_DIR}/include/${para_include_path} 
      @ONLY
    )
  else()
    configure_file(
      ${codegen_template_path}/para_generator_header_without_cv.in
      ${CMAKE_CURRENT_LIST_DIR}/include/${para_include_path}
      @ONLY
    )
  endif()
  unset(para_include_path)
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
  ############################## message ##############################
  set(para_msg "Generating IDL ${module_name} Module")
  ######################## Generate C++ header ########################
  set(para_module_header_details "")
  foreach(_sub ${RMVLPARA_${module_name}})
    string(TOUPPER "${_sub}" _upper)
    set(para_module_header_details "${para_module_header_details}\n#ifdef HAVE_RMVL_${_upper}\n")
    set(para_module_header_details "${para_module_header_details}#include \"${module_name}/${_sub}.h\"\n")
    set(para_module_header_details "${para_module_header_details}#endif // HAVE_RMVL_${_upper}\n")
  endforeach()
  # generate C++ file
  configure_file(
    ${codegen_template_path}/para_generator_module.in
    ${CMAKE_CURRENT_LIST_DIR}/include/rmvlpara/${module_name}.hpp
    @ONLY
  )
  message(STATUS "${para_msg} - done")
endfunction()

# ----------------------------------------------------------------------------
#   根据指定的消息描述文件 *.msg 和可选的模块名生成对应的 C++ 代码
#   用法:
#     rmvl_generate_msg(
#       <message_file>
#       [MODULE module_name]
#     )
#   示例:
#     rmvl_generate_msg(
#       data
#       MODULE testdata
#     )
# ----------------------------------------------------------------------------

function(rmvl_generate_msg file)
  cmake_parse_arguments(MSG "" "MODULE" "" ${ARGN})

  set(msg_file "${CMAKE_CURRENT_LIST_DIR}/msg/${file}.msg")
  _to_snake_case("${file}" file_snake_case)
  set(inc_file "${CMAKE_CURRENT_LIST_DIR}/include/rmvlmsg/${file_snake_case}.hpp")
  set(MSG_INCLUDE_CONTENT "rmvlmsg/${file_snake_case}.hpp")
  set(CLASS_NAME_COMMENT "${file}")
  get_filename_component(CLASS_NAME "${file}" NAME)
  _to_snake_case("${CLASS_NAME}" name)

  # define the target path of the src and include files
  if(MSG_MODULE)
    set(src_file "${CMAKE_CURRENT_LIST_DIR}/src/${name}/_rm_codegen_msg_${name}.cpp")
    set(module_name "${MSG_MODULE}")
  else()
    set(src_file "${CMAKE_CURRENT_LIST_DIR}/src/_rm_codegen_msg_${name}.cpp")
    set(module_name "${name}")
  endif()

  set(MSG_EXTRA_HEADERS "")

  # Read msg file content
  file(READ ${msg_file} MSG_CONTENT)
  if(NOT MSG_CONTENT)
    return()
  endif()
    
  # Parse msg file content
  set(type_and_ids)
  string(REPLACE "\n" ";" MSG_LINES ${MSG_CONTENT})
    
  foreach(line ${MSG_LINES})
    # Remove comments
    string(REGEX REPLACE "#.*$" "" line "${line}")
    # Remove leading and trailing whitespace
    string(STRIP "${line}" line)
    if(NOT line)
      continue()
    endif()
    # Parse format: type name
    string(REGEX MATCH "^([a-zA-Z0-9_/]+)(\\[[0-9]*\\])?[ \t]+([a-zA-Z0-9_]+)$" MATCHED "${line}")

    if(MATCHED)
      set(base_type ${CMAKE_MATCH_1})
      set(array_spec ${CMAKE_MATCH_2})
      set(field_name ${CMAKE_MATCH_3})

      # Handle default std types
      if(base_type STREQUAL "Header")
        set(base_type "std/${base_type}")
      endif()
      # Handle 'folder/type' format
      if(base_type MATCHES ".+/.+")
        string(REPLACE "/" ";" type_parts "${base_type}")
        list(GET type_parts 0 folder)
        list(GET type_parts 1 type_name)
        to_lowerfirst("${type_name}" include_type_name)
        set(msg_header_ext_line "#include \"rmvlmsg/${folder}/${include_type_name}.hpp\"")
        list(FIND MSG_EXTRA_HEADERS_LIST "${msg_header_ext_line}" _header_found_idx)
        if(_header_found_idx EQUAL -1)
          list(APPEND MSG_EXTRA_HEADERS_LIST "${msg_header_ext_line}")
          set(MSG_EXTRA_HEADERS "${MSG_EXTRA_HEADERS}${msg_header_ext_line}\n")
        endif()
        to_upperfirst("${type_name}" base_type)
      endif()

      set(array_info "SCALAR@")
      if(array_spec)
        if(array_spec STREQUAL "[]")
          set(array_info "VARIABLE_ARRAY@")
        else()
          string(REGEX MATCH "[0-9]+" array_size "${array_spec}")
          set(array_info "FIXED_ARRAY@${array_size}")
        endif()
      endif()

      list(APPEND type_and_ids "${base_type}@${field_name}@${array_info}")
    endif()
  endforeach()

  # Validate parsed fields
  list(LENGTH type_and_ids FIELD_COUNT)
  if(FIELD_COUNT EQUAL 0)
    return()
  endif()

  # Generate type_and_ids_cpp
  set(type_and_ids_cpp)
  set(size_list)
  set(serialize_content)
  set(deserialize_content)

  foreach(n ${type_and_ids})
    string(REPLACE "@" ";" parts "${n}")
    list(GET parts 0 type)
    list(GET parts 1 id)
    list(GET parts 2 array_type)
    list(GET parts 3 array_size)

    # Generate base C++ type
    set(is_custom_type OFF)
    if(type STREQUAL "string")
      set(cpp_base_type "std::string")
    elseif(type STREQUAL "bool")
      set(cpp_base_type "bool")
    elseif(type STREQUAL "float32")
      set(cpp_base_type "float")
    elseif(type STREQUAL "float64")
      set(cpp_base_type "double")
    elseif(type MATCHES "^int(8|16|32|64)$")
      set(cpp_base_type "${type}_t")
    elseif(type MATCHES "^uint(8|16|32|64)$")
      set(cpp_base_type "${type}_t")
    else()
      to_upperfirst("${type}" type)
      set(cpp_base_type "${type}")
      set(is_custom_type ON)
    endif()

    # Generate final C++ type based on array type
    if(array_type STREQUAL "FIXED_ARRAY")
      string(APPEND type_and_ids_cpp "    std::array<${cpp_base_type}, ${array_size}> ${id}{};\n")
    elseif(array_type STREQUAL "VARIABLE_ARRAY")
      string(APPEND type_and_ids_cpp "    std::vector<${cpp_base_type}> ${id}{};\n")
    else() # SCALAR
      string(APPEND type_and_ids_cpp "    ${cpp_base_type} ${id}{};\n")
    endif()

    # Calculate size
    if(array_type STREQUAL "FIXED_ARRAY")
      if(is_custom_type)
        set(size_expr "std::accumulate(${id}.begin(), ${id}.end(), size_t(0), [](size_t a, const auto& i) { return a + i.compact_size(); })")
        string(REPLACE ";" "\\;" size_expr "${size_expr}")
        list(APPEND size_list "${size_expr}")
      else()
        list(APPEND size_list "sizeof(${id})")
      endif()
    elseif(array_type STREQUAL "VARIABLE_ARRAY")
      if(is_custom_type)
        set(size_expr "sizeof(uint8_t) + std::accumulate(${id}.begin(), ${id}.end(), size_t(0), [](size_t a, const auto& i) { return a + i.compact_size(); })")
        string(REPLACE ";" "\\;" size_expr "${size_expr}")
        list(APPEND size_list "${size_expr}")
      else()
        if(type STREQUAL "string")
          set(size_expr "sizeof(uint8_t) + std::accumulate(${id}.begin(), ${id}.end(), size_t(0), [](size_t a, const auto& i) { return a + sizeof(uint8_t) + i.size(); })")
          string(REPLACE ";" "\\;" size_expr "${size_expr}")
          list(APPEND size_list "${size_expr}")
        else()
          list(APPEND size_list "sizeof(uint8_t) + ${id}.size() * sizeof(${cpp_base_type})")
        endif()
      endif()
    elseif(type STREQUAL "string")
      list(APPEND size_list "sizeof(uint8_t) + ${id}.size()")
    elseif(is_custom_type)
      list(APPEND size_list "${id}.compact_size()")
    else() # SCALAR
      list(APPEND size_list "sizeof(${id})")
    endif()

    # Generate serialization code
    if(array_type STREQUAL "FIXED_ARRAY")
      if(is_custom_type)
        string(APPEND serialize_content "    for (const auto& v : ${id})\n        _res_.append(v.serialize());\n")
      else()
        if(type STREQUAL "string")
          string(APPEND serialize_content "    for (const auto &v : ${id}) {\n")
          string(APPEND serialize_content "        uint8_t v_size__ = static_cast<uint8_t>(v.size());\n")
          string(APPEND serialize_content "        _res_.append(reinterpret_cast<const char *>(&v_size__), sizeof(uint8_t));\n")
          string(APPEND serialize_content "        _res_.append(v.data(), v.size());\n")
          string(APPEND serialize_content "    }\n")
        else()
          string(APPEND serialize_content "    _res_.append(reinterpret_cast<const char *>(${id}.data()), ${id}.size() * sizeof(${cpp_base_type}));\n")
        endif()
      endif()
    elseif(array_type STREQUAL "VARIABLE_ARRAY")
      string(APPEND serialize_content "    uint8_t ${id}_size__ = static_cast<uint8_t>(${id}.size());\n")
      string(APPEND serialize_content "    _res_.append(reinterpret_cast<const char *>(&${id}_size__), sizeof(uint8_t));\n")
      if(is_custom_type)
        string(APPEND serialize_content "    for (const auto &v : ${id})\n        _res_.append(v.serialize());\n")
      else()
        if(type STREQUAL "string")
          string(APPEND serialize_content "    for (const auto &v : ${id}) {\n")
          string(APPEND serialize_content "        uint8_t v_size__ = static_cast<uint8_t>(v.size());\n")
          string(APPEND serialize_content "        _res_.append(reinterpret_cast<const char *>(&v_size__), sizeof(uint8_t));\n")
          string(APPEND serialize_content "        _res_.append(v.data(), v.size());\n")
          string(APPEND serialize_content "    }\n")
        else()
          string(APPEND serialize_content "    _res_.append(reinterpret_cast<const char *>(${id}.data()), ${id}.size() * sizeof(${cpp_base_type}));\n")
        endif()
      endif()
    elseif(type STREQUAL "string")
      string(APPEND serialize_content "    uint8_t ${id}_size__ = static_cast<uint8_t>(${id}.size());\n")
      string(APPEND serialize_content "    _res_.append(reinterpret_cast<const char *>(&${id}_size__), sizeof(uint8_t));\n")
      string(APPEND serialize_content "    _res_.append(${id}.data(), ${id}.size());\n")
    elseif(is_custom_type)
      string(APPEND serialize_content "    _res_.append(${id}.serialize());\n")
    else()
      string(APPEND serialize_content "    _res_.append(reinterpret_cast<const char *>(&${id}), sizeof(${id}));\n")
    endif()

    # Generate deserialization code
    if(array_type STREQUAL "FIXED_ARRAY")
      if(is_custom_type)
        string(APPEND deserialize_content "    for (auto& v : _msg__.${id})\n        v = ${cpp_base_type}::deserialize(_p__);\n")
      else()
        string(APPEND deserialize_content "    const auto src_ptr_${id}__ = reinterpret_cast<const ${cpp_base_type} *>(_p__);\n")
        string(APPEND deserialize_content "    std::copy_n(src_ptr_${id}__, ${array_size}, _msg__.${id}.data());\n")
        string(APPEND deserialize_content "    _p__ += sizeof(${cpp_base_type}) * ${array_size};\n")
      endif()
    elseif(array_type STREQUAL "VARIABLE_ARRAY")
      string(APPEND deserialize_content "    uint8_t ${id}_size__ = *reinterpret_cast<const uint8_t *>(_p__);\n")
      string(APPEND deserialize_content "    _p__ += sizeof(uint8_t);\n")
      string(APPEND deserialize_content "    _msg__.${id}.resize(${id}_size__);\n")
      if(is_custom_type)
        string(APPEND deserialize_content "    for (auto& v : _msg__.${id})\n        v = ${cpp_base_type}::deserialize(_p__);\n")
      else()
        if(type STREQUAL "string")
          string(APPEND deserialize_content "    for (size_t i = 0; i < ${id}_size__; ++i) {\n")
          string(APPEND deserialize_content "        uint8_t v_size__ = *reinterpret_cast<const uint8_t *>(_p__);\n")
          string(APPEND deserialize_content "        _p__ += sizeof(uint8_t);\n")
          string(APPEND deserialize_content "        _msg__.${id}[i].assign(_p__, v_size__);\n")
          string(APPEND deserialize_content "        _p__ += v_size__;\n")
          string(APPEND deserialize_content "    }\n")
        else()
          string(APPEND deserialize_content "    const auto src_ptr_${id}__ = reinterpret_cast<const ${cpp_base_type} *>(_p__);\n")
          string(APPEND deserialize_content "    std::copy_n(src_ptr_${id}__, ${id}_size__, _msg__.${id}.data());\n")
          string(APPEND deserialize_content "    _p__ += sizeof(${cpp_base_type}) * ${id}_size__;\n")
        endif()
      endif()
    elseif(type STREQUAL "string")
      string(APPEND deserialize_content "    uint8_t ${id}_size__ = *reinterpret_cast<const uint8_t *>(_p__);\n")
      string(APPEND deserialize_content "    _p__ += sizeof(uint8_t);\n")
      string(APPEND deserialize_content "    _msg__.${id}.assign(_p__, ${id}_size__);\n")
      string(APPEND deserialize_content "    _p__ += ${id}_size__;\n")
    elseif(is_custom_type)
      string(APPEND deserialize_content "    _msg__.${id} = ${cpp_base_type}::deserialize(_p__);\n")
      string(APPEND deserialize_content "    _p__ += _msg__.${id}.compact_size();\n")
    else() # SCALAR
      string(APPEND deserialize_content "    _msg__.${id} = *reinterpret_cast<const ${cpp_base_type} *>(_p__);\n")
      string(APPEND deserialize_content "    _p__ += sizeof(${cpp_base_type});\n")
    endif()
  endforeach()

  # Concat size string
  list(JOIN size_list " + " size_str)
  string(REPLACE "\\;" ";" size_str "${size_str}")
  if(NOT size_str)
    set(size_str "0")
  endif()

  set(MSG_NAME ${name})

  # Generate cpp files
  configure_file(
    ${codegen_template_path}/msg_generator.hpp.in
    ${inc_file}
    @ONLY
  )
  configure_file(
    ${codegen_template_path}/msg_generator.cpp.in
    ${src_file}
    @ONLY
  )
endfunction()
