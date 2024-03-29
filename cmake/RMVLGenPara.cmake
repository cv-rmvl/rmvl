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
      COMMAND cmd /c "wmic path win32_localtime get year^,month^,day ^| findstr /r [0-9]"
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
#   修正类型符号: 增加作用域
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
  string(REGEX REPLACE "string" "std::string" retval "${retval}")
  string(REGEX REPLACE "vector" "std::vector" retval "${retval}")
  string(REGEX REPLACE "Point" "cv::Point" retval "${retval}")
  string(REGEX REPLACE "Vec" "cv::Vec" retval "${retval}")
  string(REGEX REPLACE "Mat" "cv::Mat" retval "${retval}")
  string(REGEX REPLACE "([^\\.])51f" "\\1<float, 5, 1>" retval "${retval}")
  string(REGEX REPLACE "([^\\.])51d" "\\1<double, 5, 1>" retval "${retval}")
  string(REGEX REPLACE "([^\\.])15f" "\\1<float, 1, 5>" retval "${retval}")
  string(REGEX REPLACE "([^\\.])15d" "\\1<double, 1, 5>" retval "${retval}")
  string(REGEX REPLACE "([^\\.])61f" "\\1<float, 6, 1>" retval "${retval}")
  string(REGEX REPLACE "([^\\.])61d" "\\1<double, 6, 1>" retval "${retval}")
  string(REGEX REPLACE "([^\\.])16f" "\\1<float, 1, 6>" retval "${retval}")
  string(REGEX REPLACE "([^\\.])16d" "\\1<double, 1, 6>" retval "${retval}")
  set(${out_value_type} ${retval} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------------
#   将指定的 *.para 参数规范文件解析成 C++ 风格的内容
#   用法:
#     _para_parser(
#       <file_name>
#       <header_details> <source_details>
#     )
#   示例:
#     _para_parser(
#       core.para           # 名为 core.para 的参数规范文件
#       core_header_details # 对应 .h/.hpp 文件的细节
#       core_source_details # 对应 .cpp 文件的实现细节
#       status              # 返回值: 解析是否成功，成功返回 TRUE，失败返回 FALSE
#     )
# ----------------------------------------------------------------------------
function(_para_parser file_name header_details source_details status)
  # init
  set(ret_header "")
  set(ret_source "")
  file(READ ${file_name} out_val)
  if(NOT out_val)
    set(${status} FALSE PARENT_SCOPE)
    return()
  endif()
  string(REGEX REPLACE "\n" ";" out_val "${out_val}")
  # parser each line
  foreach(substr ${out_val})
    # get substring: line_str
    string(REGEX REPLACE "[ =]" ";" line_str "${substr}")
    set(tmp)
    foreach(word ${line_str})
      list(APPEND tmp "${word}")
    endforeach()
    set(line_str ${tmp})
    unset(tmp)
    # parser
    list(LENGTH line_str l)
    if(l GREATER 1)
      # get value type symbol
      list(GET line_str 0 type_sym)
      if(type_sym MATCHES "^#")
        continue()
      endif()
      # correct the value type
      _type_correct("${type_sym}" type_sym)
      # get id symbol
      list(GET line_str 1 id_sym)
      # get default value and comment
      list(SUBLIST line_str 2 -1 default_cmt)
      string(REGEX REPLACE ";" "" default_cmt "${default_cmt}")
      # split default value and comment
      string(FIND "${default_cmt}" "#" cmt_idx)
      if(cmt_idx EQUAL -1)
        set(default_sym "${default_cmt}")
        set(comment_sym "${id_sym}")
      else()
        string(SUBSTRING "${default_cmt}" 0 ${cmt_idx} default_sym)
        math(EXPR cmt_idx "${cmt_idx} + 1")
        string(SUBSTRING "${default_cmt}" ${cmt_idx} -1 comment_sym)
      endif()
      # add default value to comment
      if(NOT default_sym STREQUAL "")
        set(comment_sym "${comment_sym} @note 默认值：`${default_sym}`")
      endif()
      # correct default_sym
      if(NOT type_sym STREQUAL "std::string")
        _type_correct("${default_sym}" default_sym)
        string(REGEX REPLACE "," ", " default_sym "${default_sym}")
      endif()
    else()
      continue()
    endif()
    # get return value (header)
    set(ret_header "${ret_header}    //! ${comment_sym}\n")
    if("${default_sym}" STREQUAL "")
      set(ret_header "${ret_header}    ${type_sym} ${id_sym}{};\n")
    else()
      set(ret_header "${ret_header}    ${type_sym} ${id_sym} = ${default_sym};\n")
    endif()
    # get return value (source)
    set(ret_source "${ret_source}    node = fs[\"${id_sym}\"];\n")
    if(type_sym MATCHES "uint" OR type_sym STREQUAL "size_t")
      set(ret_source "${ret_source}    if (!node.isNone())\n    {\n")
      set(ret_source "${ret_source}        int tmp_${id_sym}{};\n        node >> tmp_${id_sym};\n")
      set(ret_source "${ret_source}        ${id_sym} = static_cast<${type_sym}>(tmp_${id_sym});\n    }\n")
    else()
      set(ret_source "${ret_source}    node.isNone() ? void(0) : (node >> ${id_sym});\n")
    endif()
  endforeach(substr ${out_val})
  set(${header_details} "${ret_header}" PARENT_SCOPE)
  set(${source_details} "${ret_source}" PARENT_SCOPE)
  set(${status} TRUE PARENT_SCOPE)
endfunction(_para_parser file_name header_details source_details)

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
    message(STATUS "Performing Conversion ${target_name}.para - skipped")
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
  # parser
  _para_parser(${file_name} para_header_details para_source_details para_status)
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
      ${para_template_path}/para_generator_source.in
      ${CMAKE_CURRENT_LIST_DIR}/src/${target_name}/para/param.cpp
      @ONLY
    )
  # dosen't have module
  else()
    set(header_ext "hpp")
    set(para_include_path "rmvlpara/${module_name}.${header_ext}")
    configure_file(
      ${para_template_path}/para_generator_source.in
      ${CMAKE_CURRENT_LIST_DIR}/src/para/param.cpp
      @ONLY
    )
    set(def_new_group "${def_new_group}//! @addtogroup para\n//! @{\n")
    set(def_new_group "${def_new_group}//! @defgroup para_${module_name} ${module_name} 的参数模块\n")
    set(def_new_group "${def_new_group}//! @addtogroup para_${module_name}\n//! @{\n")
    set(def_new_group "${def_new_group}//! @brief 与 @ref ${module_name} 相关的参数模块，包含...\n")
    set(def_new_group "${def_new_group}//! @} para_${module_name}\n//! @} para\n")
  endif()
  configure_file(
    ${para_template_path}/para_generator_header.in
    ${CMAKE_CURRENT_LIST_DIR}/include/${para_include_path}
    @ONLY
  )
  unset(para_include_path)
  ############################ message end ############################
  message(STATUS "${para_msg} - done")
endfunction(rmvl_generate_para target_name)

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
    string(TOUPPER "${_sub}" upper)
    set(para_module_header_details "${para_module_header_details}\n#ifdef HAVE_RMVL_${upper}\n")
    set(para_module_header_details "${para_module_header_details}#include \"${module_name}/${_sub}.h\"\n")
    set(para_module_header_details "${para_module_header_details}#endif // HAVE_RMVL_${upper}\n")
  endforeach()
  # generate C++ file
  configure_file(
    ${para_template_path}/para_generator_module.in
    ${CMAKE_CURRENT_LIST_DIR}/include/rmvlpara/${module_name}.hpp
    @ONLY
  )
  ############################ message end ############################
  message(STATUS "${para_msg} - done")
endfunction(rmvl_generate_module_para module_name)
