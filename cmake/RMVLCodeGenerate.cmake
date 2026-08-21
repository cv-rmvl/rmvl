# =====================================================================================
# 参数生成模块，包含以下主要功能：
#
#   1. rmvl_generate_para:        根据给定目标及对应的参数规范文件 *.para 生成 C++ 文件
#   2. rmvl_generate_module_para: 根据给定模块下的所有 para 目标生成 C++ 文件
#   3. rmvl_generate_msg:         根据指定的消息描述文件 *.msg 生成 C++ 文件
#   4. rmvl_generate_srv:         根据指定的服务描述文件 *.srv 生成 C++ 文件
#
# 以及以下次要功能：
#
#   1. system_date:   获取系统日期
#   2. to_upperfirst: 将字符串的首字母转换为大写
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

# Parse one non-comment line shared by *.msg and each half of *.srv.
# KIND is EMPTY, CONSTANT or FIELD. FIELD also provides ARRAY_TYPE/ARRAY_SIZE.
function(_rmvl_parse_msg_line content prefix)
  string(REGEX REPLACE "#.*$" "" line "${content}")
  string(STRIP "${line}" line)
  if(NOT line)
    set(${prefix}_KIND EMPTY PARENT_SCOPE)
    return()
  endif()

  string(REGEX MATCH "^([a-zA-Z0-9_/]+)[ \t]+([a-zA-Z0-9_]+)[ \t]*=[ \t]*(.+)$" matched "${line}")
  if(matched)
    set(${prefix}_KIND CONSTANT PARENT_SCOPE)
    set(${prefix}_TYPE "${CMAKE_MATCH_1}" PARENT_SCOPE)
    set(${prefix}_ID "${CMAKE_MATCH_2}" PARENT_SCOPE)
    set(value "${CMAKE_MATCH_3}")
    string(STRIP "${value}" value)
    set(${prefix}_VALUE "${value}" PARENT_SCOPE)
    return()
  endif()

  string(REGEX MATCH "^([a-zA-Z0-9_/]+)(\\[[0-9]*\\])?[ \t]+([a-zA-Z0-9_]+)$" matched "${line}")
  if(NOT matched)
    message(FATAL_ERROR "Invalid message field: '${line}'")
  endif()
  set(type "${CMAKE_MATCH_1}")
  set(array_spec "${CMAKE_MATCH_2}")
  set(id "${CMAKE_MATCH_3}")
  set(array_type SCALAR)
  set(array_size 0)
  if(array_spec STREQUAL "[]")
    set(array_type VARIABLE_ARRAY)
  elseif(array_spec)
    set(array_type FIXED_ARRAY)
    string(REGEX MATCH "[0-9]+" array_size "${array_spec}")
  endif()
  set(${prefix}_KIND FIELD PARENT_SCOPE)
  set(${prefix}_TYPE "${type}" PARENT_SCOPE)
  set(${prefix}_ID "${id}" PARENT_SCOPE)
  set(${prefix}_ARRAY_TYPE "${array_type}" PARENT_SCOPE)
  set(${prefix}_ARRAY_SIZE "${array_size}" PARENT_SCOPE)
endfunction()

# Convert one IDL type to its C++ spelling and report custom-message metadata.
function(_rmvl_resolve_msg_type type prefix)
  cmake_parse_arguments(TYPE "CONSTANT" "" "" ${ARGN})
  set(idl_type "${type}")
  set(custom OFF)
  set(header)

  if(TYPE_CONSTANT AND type STREQUAL "string")
    set(cpp_type "const char *")
  elseif(TYPE_CONSTANT AND type STREQUAL "time")
    set(cpp_type "int64_t")
  elseif(type STREQUAL "string")
    set(cpp_type "std::string")
  elseif(type STREQUAL "float32")
    set(cpp_type "float")
  elseif(type STREQUAL "float64")
    set(cpp_type "double")
  elseif(type MATCHES "^(u?int)(8|16|32|64)$")
    set(cpp_type "${type}_t")
  elseif(type STREQUAL "bool" OR type STREQUAL "char")
    set(cpp_type "${type}")
  else()
    set(_std_types Header ColorRGBA time duration)
    if(type IN_LIST _std_types)
      set(type "std/${type}")
    endif()
    if(type MATCHES "^([^/]+)/([^/]+)$")
      set(folder "${CMAKE_MATCH_1}")
      set(type_name "${CMAKE_MATCH_2}")
      _to_snake_case("${type_name}" include_type_name)
      to_upperfirst("${type_name}" cpp_name)
      set(cpp_type "rm::msg::${cpp_name}")
      set(header "#include \"rmvlmsg/${folder}/${include_type_name}.hpp\"")
      set(custom ON)
    else()
      # Keep compatibility with unqualified user-defined message types.
      to_upperfirst("${type}" cpp_name)
      set(cpp_type "rm::msg::${cpp_name}")
      set(custom ON)
    endif()
  endif()

  set(${prefix}_IDL_TYPE "${idl_type}" PARENT_SCOPE)
  set(${prefix}_CPP_TYPE "${cpp_type}" PARENT_SCOPE)
  set(${prefix}_CUSTOM "${custom}" PARENT_SCOPE)
  set(${prefix}_HEADER "${header}" PARENT_SCOPE)
endfunction()

# Parse one half of a service definition. The generated wire representation is
# intentionally identical to rmvl_generate_msg.
function(_rmvl_parse_srv_part content prefix)
  set(fields)
  set(constants)
  set(items)
  set(extra_headers)
  set(extra_header_list)
  string(REPLACE "\n" ";" lines "${content}")

  foreach(line ${lines})
    _rmvl_parse_msg_line("${line}" parsed)
    if(parsed_KIND STREQUAL "EMPTY")
      continue()
    endif()
    if(parsed_KIND STREQUAL "CONSTANT")
      _rmvl_resolve_msg_type("${parsed_TYPE}" resolved CONSTANT)
      if(parsed_TYPE STREQUAL "string")
        set(parsed_VALUE "\"${parsed_VALUE}\"")
      endif()
      string(APPEND constants "    static constexpr ${resolved_CPP_TYPE} ${parsed_ID} = ${parsed_VALUE};\n")
      continue()
    endif()

    _rmvl_resolve_msg_type("${parsed_TYPE}" resolved)
    if(resolved_HEADER)
      list(FIND extra_header_list "${resolved_HEADER}" header_index)
      if(header_index EQUAL -1)
        list(APPEND extra_header_list "${resolved_HEADER}")
        string(APPEND extra_headers "${resolved_HEADER}\n")
      endif()
    endif()
    list(APPEND items "${resolved_IDL_TYPE}@${resolved_CPP_TYPE}@${resolved_CUSTOM}@${parsed_ID}@${parsed_ARRAY_TYPE}@${parsed_ARRAY_SIZE}")
  endforeach()

  set(size_list)
  set(serialize)
  set(deserialize)
  foreach(item ${items})
    string(REPLACE "@" ";" parts "${item}")
    list(GET parts 0 type)
    list(GET parts 1 cpp_type)
    list(GET parts 2 custom)
    list(GET parts 3 id)
    list(GET parts 4 array_type)
    list(GET parts 5 array_size)

    if(array_type STREQUAL "FIXED_ARRAY")
      string(APPEND fields "        std::array<${cpp_type}, ${array_size}> ${id}{};\n")
    elseif(array_type STREQUAL "VARIABLE_ARRAY")
      string(APPEND fields "        std::vector<${cpp_type}> ${id}{};\n")
    else()
      string(APPEND fields "        ${cpp_type} ${id}{};\n")
    endif()

    if(array_type STREQUAL "FIXED_ARRAY")
      if(custom)
        set(expr "std::accumulate(${id}.begin(), ${id}.end(), size_t(0), [](size_t a, const auto &v) { return a + v.compact_size(); })")
        string(REPLACE ";" "\\;" expr "${expr}")
        list(APPEND size_list "${expr}")
        string(APPEND serialize "    for (const auto &v : ${id})\n        _res_.append(v.serialize());\n")
        string(APPEND deserialize "    for (auto &v : _msg__.${id}) {\n        v = ${cpp_type}::deserialize(_p__);\n        _p__ += v.compact_size();\n    }\n")
      elseif(type STREQUAL "string")
        set(expr "std::accumulate(${id}.begin(), ${id}.end(), size_t(0), [](size_t a, const auto &v) { return a + sizeof(uint32_t) + v.size(); })")
        string(REPLACE ";" "\\;" expr "${expr}")
        list(APPEND size_list "${expr}")
        string(APPEND serialize "    for (const auto &v : ${id}) {\n        auto n = static_cast<uint32_t>(v.size());\n        append_scalar(_res_, n);\n        _res_.append(v);\n    }\n")
        string(APPEND deserialize "    for (auto &v : _msg__.${id}) {\n        auto n = read_scalar<uint32_t>(_p__);\n        v.assign(_p__, n);\n        _p__ += n;\n    }\n")
      else()
        list(APPEND size_list "sizeof(${id})")
        string(APPEND serialize "    append_array(_res_, ${id}.data(), ${id}.size());\n")
        string(APPEND deserialize "    read_array(_p__, _msg__.${id}.data(), ${array_size});\n")
      endif()
    elseif(array_type STREQUAL "VARIABLE_ARRAY")
      if(custom)
        set(expr "sizeof(uint32_t) + std::accumulate(${id}.begin(), ${id}.end(), size_t(0), [](size_t a, const auto &v) { return a + v.compact_size(); })")
        string(REPLACE ";" "\\;" expr "${expr}")
        list(APPEND size_list "${expr}")
      elseif(type STREQUAL "string")
        set(expr "sizeof(uint32_t) + std::accumulate(${id}.begin(), ${id}.end(), size_t(0), [](size_t a, const auto &v) { return a + sizeof(uint32_t) + v.size(); })")
        string(REPLACE ";" "\\;" expr "${expr}")
        list(APPEND size_list "${expr}")
      else()
        list(APPEND size_list "sizeof(uint32_t) + ${id}.size() * sizeof(${cpp_type})")
      endif()
      string(APPEND serialize "    auto ${id}_size__ = static_cast<uint32_t>(${id}.size());\n    append_scalar(_res_, ${id}_size__);\n")
      if(custom)
        string(APPEND serialize "    for (const auto &v : ${id})\n        _res_.append(v.serialize());\n")
        string(APPEND deserialize "    auto ${id}_size__ = read_scalar<uint32_t>(_p__);\n    _msg__.${id}.resize(${id}_size__);\n    for (auto &v : _msg__.${id}) {\n        v = ${cpp_type}::deserialize(_p__);\n        _p__ += v.compact_size();\n    }\n")
      elseif(type STREQUAL "string")
        string(APPEND serialize "    for (const auto &v : ${id}) {\n        auto n = static_cast<uint32_t>(v.size());\n        append_scalar(_res_, n);\n        _res_.append(v);\n    }\n")
        string(APPEND deserialize "    auto ${id}_size__ = read_scalar<uint32_t>(_p__);\n    _msg__.${id}.resize(${id}_size__);\n    for (auto &v : _msg__.${id}) {\n        auto n = read_scalar<uint32_t>(_p__);\n        v.assign(_p__, n);\n        _p__ += n;\n    }\n")
      else()
        string(APPEND serialize "    append_array(_res_, ${id}.data(), ${id}.size());\n")
        string(APPEND deserialize "    auto ${id}_size__ = read_scalar<uint32_t>(_p__);\n    _msg__.${id}.resize(${id}_size__);\n    read_array(_p__, _msg__.${id}.data(), ${id}_size__);\n")
      endif()
    elseif(type STREQUAL "string")
      list(APPEND size_list "sizeof(uint32_t) + ${id}.size()")
      string(APPEND serialize "    auto ${id}_size__ = static_cast<uint32_t>(${id}.size());\n    append_scalar(_res_, ${id}_size__);\n    _res_.append(${id});\n")
      string(APPEND deserialize "    auto ${id}_size__ = read_scalar<uint32_t>(_p__);\n    _msg__.${id}.assign(_p__, ${id}_size__);\n    _p__ += ${id}_size__;\n")
    elseif(custom)
      list(APPEND size_list "${id}.compact_size()")
      string(APPEND serialize "    _res_.append(${id}.serialize());\n")
      string(APPEND deserialize "    _msg__.${id} = ${cpp_type}::deserialize(_p__);\n    _p__ += _msg__.${id}.compact_size();\n")
    else()
      list(APPEND size_list "sizeof(${id})")
      string(APPEND serialize "    append_scalar(_res_, ${id});\n")
      string(APPEND deserialize "    _msg__.${id} = read_scalar<${cpp_type}>(_p__);\n")
    endif()
  endforeach()

  if(constants)
    if(fields)
      string(APPEND fields "\n")
    endif()
    string(APPEND fields "${constants}")
  endif()
  list(JOIN size_list " + " size)
  string(REPLACE "\\;" ";" size "${size}")
  if(NOT size)
    set(size 0)
  endif()

  set(${prefix}_FIELDS "${fields}" PARENT_SCOPE)
  set(${prefix}_SERIALIZE "${serialize}" PARENT_SCOPE)
  set(${prefix}_DESERIALIZE "${deserialize}" PARENT_SCOPE)
  set(${prefix}_SIZE "${size}" PARENT_SCOPE)
  set(${prefix}_EXTRA_HEADERS "${extra_headers}" PARENT_SCOPE)
endfunction()

# Generate a service type from srv/<file>.srv. The request and response are
# separated by a line containing exactly `---` after trimming whitespace.
function(rmvl_generate_srv file)
  cmake_parse_arguments(SRV "" "MODULE" "" ${ARGN})
  set(srv_file "${CMAKE_CURRENT_LIST_DIR}/srv/${file}.srv")
  if(NOT EXISTS "${srv_file}")
    message(FATAL_ERROR "Service definition not found: ${srv_file}")
  endif()
  file(READ "${srv_file}" SRV_CONTENT)
  string(REGEX MATCHALL "(^|\n)[ \t]*---[ \t]*(\r?\n|$)" separators "${SRV_CONTENT}")
  list(LENGTH separators separator_count)
  if(NOT separator_count EQUAL 1)
    message(FATAL_ERROR "Service definition '${srv_file}' must contain exactly one --- separator")
  endif()
  string(REGEX REPLACE "(^|\n)[ \t]*---[ \t]*(\r?\n|$)" "@@RMVL_SRV_SEPARATOR@@" split_content "${SRV_CONTENT}")
  string(REPLACE "@@RMVL_SRV_SEPARATOR@@" ";" parts "${split_content}")
  list(GET parts 0 request_content)
  list(GET parts 1 response_content)

  _rmvl_parse_srv_part("${request_content}" REQUEST)
  _rmvl_parse_srv_part("${response_content}" RESPONSE)

  get_filename_component(CLASS_NAME "${file}" NAME)
  set(CLASS_NAME_COMMENT "${file}")
  _to_snake_case("${file}" file_snake_case)
  _to_snake_case("${CLASS_NAME}" SRV_NAME)
  set(SRV_INCLUDE_CONTENT "rmvlsrv/${file_snake_case}.hpp")
  set(SRV_EXTRA_HEADERS "${REQUEST_EXTRA_HEADERS}${RESPONSE_EXTRA_HEADERS}")
  set(inc_file "${CMAKE_CURRENT_LIST_DIR}/include/rmvlsrv/${file_snake_case}.hpp")
  if(SRV_MODULE)
    set(src_file "${CMAKE_CURRENT_LIST_DIR}/src/${SRV_NAME}/_rm_codegen_srv_${SRV_NAME}.cpp")
  else()
    set(src_file "${CMAKE_CURRENT_LIST_DIR}/src/_rm_codegen_srv_${SRV_NAME}.cpp")
  endif()
  get_filename_component(inc_dir "${inc_file}" DIRECTORY)
  get_filename_component(src_dir "${src_file}" DIRECTORY)
  file(MAKE_DIRECTORY "${inc_dir}")
  file(MAKE_DIRECTORY "${src_dir}")

  configure_file(${codegen_template_path}/srv_generator.hpp.in "${inc_file}" @ONLY)
  configure_file(${codegen_template_path}/srv_generator.cpp.in "${src_file}" @ONLY)
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
  # e.g., ColorRGBA -> color_rgba, UInt8 -> uint8
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
  set(retval ${value_type})
  string(REGEX REPLACE "(size_t|string|vector)" "std::\\1" retval "${retval}")
  string(REGEX REPLACE "Matx([1-9])([1-9])f" "Matx<float,\\1,\\2>" retval "${retval}")
  string(REGEX REPLACE "Matx([1-9])([1-9])d" "Matx<double,\\1,\\2>" retval "${retval}")
  string(REGEX REPLACE "Vec([1-9])f" "Vec<float,\\1>" retval "${retval}")
  string(REGEX REPLACE "Vec([1-9])d" "Vec<double,\\1>" retval "${retval}")
  string(REGEX REPLACE "(Point3|Point|Size|Complex|Rect|Matx|Mat|Vec|Scalar|Range|KeyPoint|DMatch)" "cv::\\1" retval "${retval}")
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
    if(type_sym MATCHES "Point|Size|Complex|Rect|Matx|Mat|Vec|Scalar|Range|KeyPoint|DMatch")
      set(is_cv_type TRUE)
    else()
      set(is_cv_type FALSE)
    endif()
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
  if(is_cv_type)
    set(ret_header_line "${ret_header_line}#ifdef HAVE_OPENCV\n")
  endif()
  set(ret_header_line "${ret_header_line}    //! ${comment_sym}\n")
  if("${default_sym}" STREQUAL "")
    set(ret_header_line "${ret_header_line}    RMVL_W_RW ${type_sym_correct} ${id_sym}{};\n")
  else()
    set(ret_header_line "${ret_header_line}    RMVL_W_RW ${type_sym_correct} ${id_sym} = ${default_sym};\n")
  endif()
  if(is_cv_type)
    set(ret_header_line "${ret_header_line}#endif // HAVE_OPENCV\n")
  endif()
  # 获取 Source 部分的返回值
  if(is_cv_type)
    set(ret_source_read_line "${ret_source_read_line}#ifdef HAVE_OPENCV\n")
    set(ret_source_write_line "${ret_source_write_line}#ifdef HAVE_OPENCV\n")
  endif()
  set(ret_source_read_line "${ret_source_read_line}    _node__ = _root__[\"${id_sym}\"];\n")
  if(type_sym MATCHES "bool|int|float|double|string|vector|size_t|Point|Size|Complex|Rect|Mat|Vec|Scalar|Range|KeyPoint|DMatch")
    set(ret_source_read_line "${ret_source_read_line}    if (_node__.valid())\n        _ok__ = _node__.read(${id_sym}) && _ok__;\n")
    set(ret_source_write_line "${ret_source_write_line}    _ok__ = _root__.set(\"${id_sym}\", ${id_sym}) && _ok__;\n")
  else()
    set(ret_source_read_line "${ret_source_read_line}    if (_node__.valid()) {\n        std::string _value__;\n        const auto _valid__ = _node__.read(_value__);\n        const auto _iter__ = _valid__ ? s2t_${type_sym}.find(_value__) : s2t_${type_sym}.end();\n        if (_iter__ == s2t_${type_sym}.end())\n            _ok__ = false;\n        else\n            ${id_sym} = _iter__->second;\n    }\n")
    set(ret_source_write_line "${ret_source_write_line}    {\n        const auto _iter__ = t2s_${type_sym}.find(${id_sym});\n        if (_iter__ == t2s_${type_sym}.end())\n            _ok__ = false;\n        else\n            _ok__ = _root__.set(\"${id_sym}\", _iter__->second) && _ok__;\n    }\n")
  endif()
  if(is_cv_type)
    set(ret_source_read_line "${ret_source_read_line}#endif // HAVE_OPENCV\n")
    set(ret_source_write_line "${ret_source_write_line}#endif // HAVE_OPENCV\n")
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
    configure_file(
      ${codegen_template_path}/para_generator_source.in
      ${CMAKE_CURRENT_LIST_DIR}/src/${target_name}/_rm_codegen_param.cpp
      @ONLY
    )
  # dosen't have module
  else()
    set(header_ext "hpp")
    set(para_include_path "rmvlpara/${module_name}.${header_ext}")
    configure_file(
      ${codegen_template_path}/para_generator_source.in
      ${CMAKE_CURRENT_LIST_DIR}/src/_rm_codegen_param.cpp
      @ONLY
    )
    set(def_new_group "${def_new_group}//! @addtogroup rmvlpara\n//! @{\n")
    set(def_new_group "${def_new_group}//! @defgroup para_${module_name} ${module_name} 的参数模块\n")
    set(def_new_group "${def_new_group}//! @addtogroup para_${module_name}\n//! @{\n")
    set(def_new_group "${def_new_group}//! @brief 与 @ref ${module_name} 相关的参数模块，包含...\n")
    set(def_new_group "${def_new_group}//! @} para_${module_name}\n//! @} rmvlpara\n")
  endif()
  configure_file(
    ${codegen_template_path}/para_generator_header.in
    ${CMAKE_CURRENT_LIST_DIR}/include/${para_include_path}
    @ONLY
  )
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
  set(constexpr_fields_cpp)
  set(json_fields)
  string(REPLACE "\n" ";" MSG_LINES ${MSG_CONTENT})
    
  foreach(line ${MSG_LINES})
    _rmvl_parse_msg_line("${line}" parsed)
    if(parsed_KIND STREQUAL "EMPTY")
      continue()
    endif()

    if(parsed_KIND STREQUAL "CONSTANT")
      _rmvl_resolve_msg_type("${parsed_TYPE}" resolved CONSTANT)
      if(parsed_TYPE STREQUAL "string")
        set(parsed_VALUE "\"${parsed_VALUE}\"")
      endif()
      string(APPEND constexpr_fields_cpp "    static constexpr ${resolved_CPP_TYPE} ${parsed_ID} = ${parsed_VALUE};\n")
      continue()
    endif()

    _rmvl_resolve_msg_type("${parsed_TYPE}" resolved)
    if(resolved_HEADER)
      list(FIND MSG_EXTRA_HEADERS_LIST "${resolved_HEADER}" _header_found_idx)
      if(_header_found_idx EQUAL -1)
        list(APPEND MSG_EXTRA_HEADERS_LIST "${resolved_HEADER}")
        string(APPEND MSG_EXTRA_HEADERS "${resolved_HEADER}\n")
      endif()
    endif()
    list(APPEND type_and_ids "${resolved_IDL_TYPE}@${resolved_CPP_TYPE}@${resolved_CUSTOM}@${parsed_ID}@${parsed_ARRAY_TYPE}@${parsed_ARRAY_SIZE}")
    list(APPEND json_fields "${parsed_ID}")
  endforeach()

  # Validate parsed fields
  list(LENGTH type_and_ids FIELD_COUNT)
  if(FIELD_COUNT EQUAL 0 AND NOT constexpr_fields_cpp)
    return()
  endif()

  # Generate type_and_ids_cpp
  set(type_and_ids_cpp)
  set(size_list)
  set(serialize_content)
  set(deserialize_content)
  set(json_field_info) # Store field type info for JSON serialization

  foreach(n ${type_and_ids})
    string(REPLACE "@" ";" parts "${n}")
    list(GET parts 0 type)
    list(GET parts 1 cpp_base_type)
    list(GET parts 2 is_custom_type)
    list(GET parts 3 id)
    list(GET parts 4 array_type)
    list(GET parts 5 array_size)

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
        set(size_expr "sizeof(uint32_t) + std::accumulate(${id}.begin(), ${id}.end(), size_t(0), [](size_t a, const auto& i) { return a + i.compact_size(); })")
        string(REPLACE ";" "\\;" size_expr "${size_expr}")
        list(APPEND size_list "${size_expr}")
      else()
        if(type STREQUAL "string")
          set(size_expr "sizeof(uint32_t) + std::accumulate(${id}.begin(), ${id}.end(), size_t(0), [](size_t a, const auto& i) { return a + sizeof(uint32_t) + i.size(); })")
          string(REPLACE ";" "\\;" size_expr "${size_expr}")
          list(APPEND size_list "${size_expr}")
        else()
          list(APPEND size_list "sizeof(uint32_t) + ${id}.size() * sizeof(${cpp_base_type})")
        endif()
      endif()
    elseif(type STREQUAL "string")
      list(APPEND size_list "sizeof(uint32_t) + ${id}.size()")
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
          string(APPEND serialize_content "        uint32_t v_size__ = static_cast<uint32_t>(v.size());\n")
          string(APPEND serialize_content "        append_scalar(_res_, v_size__);\n")
          string(APPEND serialize_content "        _res_.append(v.data(), v_size__);\n")
          string(APPEND serialize_content "    }\n")
        else()
          string(APPEND serialize_content "    append_array(_res_, ${id}.data(), ${id}.size());\n")
        endif()
      endif()
    elseif(array_type STREQUAL "VARIABLE_ARRAY")
      string(APPEND serialize_content "    uint32_t ${id}_size__ = static_cast<uint32_t>(${id}.size());\n")
      string(APPEND serialize_content "    append_scalar(_res_, ${id}_size__);\n")
      if(is_custom_type)
        string(APPEND serialize_content "    for (const auto &v : ${id})\n        _res_.append(v.serialize());\n")
      else()
        if(type STREQUAL "string")
          string(APPEND serialize_content "    for (const auto &v : ${id}) {\n")
          string(APPEND serialize_content "        uint32_t v_size__ = static_cast<uint32_t>(v.size());\n")
          string(APPEND serialize_content "        append_scalar(_res_, v_size__);\n")
          string(APPEND serialize_content "        _res_.append(v.data(), v_size__);\n")
          string(APPEND serialize_content "    }\n")
        else()
          string(APPEND serialize_content "    append_array(_res_, ${id}.data(), ${id}.size());\n")
        endif()
      endif()
    elseif(type STREQUAL "string")
      string(APPEND serialize_content "    uint32_t ${id}_size__ = static_cast<uint32_t>(${id}.size());\n")
      string(APPEND serialize_content "    append_scalar(_res_, ${id}_size__);\n")
      string(APPEND serialize_content "    _res_.append(${id}.data(), ${id}_size__);\n")
    elseif(is_custom_type)
      string(APPEND serialize_content "    _res_.append(${id}.serialize());\n")
    else()
      string(APPEND serialize_content "    append_scalar(_res_, ${id});\n")
    endif() 

    # Generate deserialization code
    if(array_type STREQUAL "FIXED_ARRAY")
      if(is_custom_type)
        string(APPEND deserialize_content "    for (auto& v : _msg__.${id}) {\n")
        string(APPEND deserialize_content "        v = ${cpp_base_type}::deserialize(_p__);\n")
        string(APPEND deserialize_content "        _p__ += v.compact_size();\n")
        string(APPEND deserialize_content "    }\n")
      else()
        string(APPEND deserialize_content "    read_array(_p__, _msg__.${id}.data(), ${array_size});\n")
      endif()
    elseif(array_type STREQUAL "VARIABLE_ARRAY")
      string(APPEND deserialize_content "    uint32_t ${id}_size__ = read_scalar<uint32_t>(_p__);\n")
      string(APPEND deserialize_content "    _msg__.${id}.resize(${id}_size__);\n")
      if(is_custom_type)
        string(APPEND deserialize_content "    for (auto& v : _msg__.${id}) {\n")
        string(APPEND deserialize_content "        v = ${cpp_base_type}::deserialize(_p__);\n")
        string(APPEND deserialize_content "        _p__ += v.compact_size();\n")
        string(APPEND deserialize_content "    }\n")
      else()
        if(type STREQUAL "string")
          string(APPEND deserialize_content "    for (size_t i = 0; i < ${id}_size__; ++i) {\n")
          string(APPEND deserialize_content "        uint32_t v_size__ = read_scalar<uint32_t>(_p__);\n")
          string(APPEND deserialize_content "        _msg__.${id}[i].assign(_p__, v_size__);\n")
          string(APPEND deserialize_content "        _p__ += v_size__;\n")
          string(APPEND deserialize_content "    }\n")
        else()
          string(APPEND deserialize_content "    read_array(_p__, _msg__.${id}.data(), ${id}_size__);\n")
        endif()
      endif()
    elseif(type STREQUAL "string")
      string(APPEND deserialize_content "    uint32_t ${id}_size__ = read_scalar<uint32_t>(_p__);\n")
      string(APPEND deserialize_content "    _msg__.${id}.assign(_p__, ${id}_size__);\n")
      string(APPEND deserialize_content "    _p__ += ${id}_size__;\n")
    elseif(is_custom_type)
      string(APPEND deserialize_content "    _msg__.${id} = ${cpp_base_type}::deserialize(_p__);\n")
      string(APPEND deserialize_content "    _p__ += _msg__.${id}.compact_size();\n")
    else() # SCALAR
      string(APPEND deserialize_content "    _msg__.${id} = read_scalar<${cpp_base_type}>(_p__);\n")
    endif()

    # Record field type info for JSON serialization
    list(FIND json_fields "${id}" json_idx)
    if(NOT json_idx EQUAL -1)
      list(APPEND json_field_info "${type}@${id}@${array_type}@${is_custom_type}")
    endif()
  endforeach()

  # Append constexpr fields to type_and_ids_cpp
  list(JOIN json_fields ", " json_field_list)
  
  # Generate manual JSON serialization code instead of using NLOHMANN macro
  if(json_field_list)
    set(json_serialize_content "    std::string _json_str__ = \"{\";\n")
    set(first_field TRUE)
    foreach(field_info ${json_field_info})
      string(REPLACE "@" ";" parts "${field_info}")
      list(GET parts 0 field_type)
      list(GET parts 1 field_id)
      list(GET parts 2 field_array_type)
      list(GET parts 3 field_is_custom)

      if(NOT first_field)
        string(APPEND json_serialize_content "    _json_str__ += \",\";\n")
      endif()
      set(first_field FALSE)

      # Generate JSON key
      string(APPEND json_serialize_content "    _json_str__ += \"\\\"${field_id}\\\":\";\n")

      # Generate JSON value based on type
      if(field_array_type STREQUAL "VARIABLE_ARRAY")
        string(APPEND json_serialize_content "    _json_str__ += \"[\";\n")
        string(APPEND json_serialize_content "    for (size_t i = 0; i < ${field_id}.size(); ++i) {\n")
        string(APPEND json_serialize_content "        if (i > 0)\n            _json_str__ += \",\";\n")
        if(field_is_custom)
          string(APPEND json_serialize_content "        _json_str__ += ${field_id}[i].json();\n")
        elseif(field_type STREQUAL "string")
          string(APPEND json_serialize_content "        _json_str__ += \"\\\"\" + ${field_id}[i] + \"\\\"\";\n")
        elseif(field_type STREQUAL "bool")
          string(APPEND json_serialize_content "        _json_str__ += (${field_id}[i] ? \"true\" : \"false\");\n")
        else()
          string(APPEND json_serialize_content "        _json_str__ += std::to_string(${field_id}[i]);\n")
        endif()
        string(APPEND json_serialize_content "    }\n")
        string(APPEND json_serialize_content "    _json_str__ += \"]\";\n")
      elseif(field_array_type STREQUAL "FIXED_ARRAY")
        string(APPEND json_serialize_content "    _json_str__ += \"[\";\n")
        string(APPEND json_serialize_content "    for (size_t i = 0; i < ${field_id}.size(); ++i) {\n")
        string(APPEND json_serialize_content "        if (i > 0) _json_str__ += \",\";\n")
        if(field_is_custom)
          string(APPEND json_serialize_content "        _json_str__ += ${field_id}[i].json();\n")
        elseif(field_type STREQUAL "string")
          string(APPEND json_serialize_content "        _json_str__ += \"\\\"\" + ${field_id}[i] + \"\\\"\";\n")
        elseif(field_type STREQUAL "bool")
          string(APPEND json_serialize_content "        _json_str__ += (${field_id}[i] ? \"true\" : \"false\");\n")
        else()
          string(APPEND json_serialize_content "        _json_str__ += std::to_string(${field_id}[i]);\n")
        endif()
        string(APPEND json_serialize_content "    }\n")
        string(APPEND json_serialize_content "    _json_str__ += \"]\";\n")
      else() # SCALAR
        if(field_is_custom)
          string(APPEND json_serialize_content "    _json_str__ += ${field_id}.json();\n")
        elseif(field_type STREQUAL "string")
          string(APPEND json_serialize_content "    _json_str__ += \"\\\"\" + ${field_id} + \"\\\"\";\n")
        elseif(field_type STREQUAL "bool")
          string(APPEND json_serialize_content "    _json_str__ += (${field_id} ? \"true\" : \"false\");\n")
        else()
          string(APPEND json_serialize_content "    _json_str__ += std::to_string(${field_id});\n")
        endif()
      endif()
    endforeach()
    string(APPEND json_serialize_content "    _json_str__ += \"}\";\n")
    string(APPEND json_serialize_content "    return _json_str__;\n")
    set(json_return_stmt "${json_serialize_content}")
    set(json_define_cpp "")
  else()
    set(json_return_stmt "    return \"{}\";\n")
    set(json_define_cpp "")
  endif()

  if(constexpr_fields_cpp)
    if(type_and_ids_cpp)
      string(APPEND type_and_ids_cpp "\n")
    endif()
    string(APPEND type_and_ids_cpp "${constexpr_fields_cpp}")
  endif()

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
