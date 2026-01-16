# ----------------------------------------------------------------------------
#   检查 C++ 支持版本
#   用法示例:
#   rmvl_check_cxx(
#     HAVE_CXX17
#     cxx17.cpp "-std=c++17"
#   )
# ----------------------------------------------------------------------------
macro(rmvl_check_cxx result src standard)
  if(NOT "${standard}" STREQUAL "")
    set(build_args " ${standard}")
  endif()
  message(STATUS "Performing Test ${result} (check file: ${src}${build_args})")
  string(REGEX REPLACE ".*c\\+\\+([0-9]+)$" "\\1" standard_num "${standard}")
  try_compile(
    ${result}
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/cpp${standard_num}
    SOURCES ${PROJECT_SOURCE_DIR}/cmake/check/${src}
    COMPILE_DEFINITIONS "${standard}"
  )

  if(${result})
    set(${result} 1 CACHE INTERNAL "Test ${result}")
    message(STATUS "Performing Test ${result} - Success")
  else()
    set(${result} "" CACHE INTERNAL "Test ${result}")
    message(STATUS "Performing Test ${result} - Failed")
  endif()
  unset(build_args)
endmacro()

set(CMAKE_DOXYGEN_PREDEFINED "" CACHE INTERNAL "Doxygen Predefined" FORCE)

# ----------------------------------------------------------------------------
#   更新文档预定义
#   用法示例:
#   rmvl_update_doxygen_predefined(xxx)
# ----------------------------------------------------------------------------
macro(rmvl_update_doxygen_predefined _name)
  set(
    CMAKE_DOXYGEN_PREDEFINED "${CMAKE_DOXYGEN_PREDEFINED} ${_name}"
    CACHE INTERNAL "Doxygen Predefined" FORCE
  )
endmacro()

include(CheckIncludeFile)
include(CheckIncludeFileCXX)

# ----------------------------------------------------------------------------
#   检查指定的头文件是否存在，可选择将结果生成到指定的变量中，并更新文档预定义
#   用法示例:
#   rmvl_check_include_file(
#     FILES math.h stdio.h string.h
#     DETAILS DEFINITIONS
#   )
# ----------------------------------------------------------------------------
macro(rmvl_check_include_file)
  # parse arguments
  cmake_parse_arguments(CIF "CXX" "DETAILS" "FILES" ${ARGN})
  if(NOT CIF_FILES)
    message(FATAL_ERROR "rmvl_check_include_file: FILES not specified")
  endif()
  foreach(f ${CIF_FILES})
    # convert the name of the file to upper case
    string(TOUPPER "${f}" fupper)
    string(REPLACE "." "_" fupper "${fupper}")
    if(CIF_CXX)
      check_include_file_cxx(${f} HAVE_${fupper})
    else()
      check_include_file(${f} HAVE_${fupper})
    endif()
    # 如果 ${CIF_DETAILS} 不为空，则将结果保存到 ${CIF_DETAILS} 中
    if(CIF_DETAILS)
      if(HAVE_${fupper})
        list(APPEND ${CIF_DETAILS} "HAVE_${fupper}")
        rmvl_update_doxygen_predefined("HAVE_${fupper}")
      endif()
    else()
      if(HAVE_${fupper})
        rmvl_update_doxygen_predefined("HAVE_${fupper}")
      endif()
    endif()
  endforeach()
endmacro()

# ----------------------------------------------------------------------------
#   字符串配置，将文件内容配置到变量中，类似于 configure_file
#   用法示例:
#   rmvl_cmake_configure("xxx.txt" VARS @ONLY)
# ----------------------------------------------------------------------------
macro(rmvl_cmake_configure file_name var_name)
  file(READ "${file_name}" __config)
  string(CONFIGURE "${__config}" ${var_name} ${ARGN})
endmacro()

set(RMVL_BUILD_INFO_STR "" CACHE INTERNAL "")
function(rmvl_output_status msg)
  message(STATUS "${msg}")
  string(REPLACE "\\" "\\\\" msg "${msg}")
  string(REPLACE "\"" "\\\"" msg "${msg}")
  string(REGEX REPLACE "^\n+|\n+$" "" msg "${msg}")
  if(msg MATCHES "\n")
    message(WARNING "String to be inserted to version_string.inc has an unexpected line break: '${msg}'")
    string(REPLACE "\n" "\\n" msg "${msg}")
  endif()
  set(RMVL_BUILD_INFO_STR "${RMVL_BUILD_INFO_STR}\"${msg}\\n\"\n" CACHE INTERNAL "")
endfunction()

macro(rmvl_finalize_status)
  set(RMVL_BUILD_INFO_FILE "${CMAKE_BINARY_DIR}/version_string.tmp")
  if(EXISTS "${RMVL_BUILD_INFO_FILE}")
    file(READ "${RMVL_BUILD_INFO_FILE}" _content)
  else()
    set(_content "")
  endif()
  if(NOT "${_content}" STREQUAL "${RMVL_BUILD_INFO_STR}")
    file(WRITE "${RMVL_BUILD_INFO_FILE}" "${RMVL_BUILD_INFO_STR}")
  endif()
  unset(_content)
  unset(RMVL_BUILD_INFO_STR CACHE)

  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${RMVL_BUILD_INFO_FILE}" "${PROJECT_BINARY_DIR}/version_string.inc"
    OUTPUT_QUIET
  )
endmacro()

# ----------------------------------------------------------------------------
#   状态报告，自动对齐右列，并在条件语句中自动选择文本
#   用法:
#     status(<text>)
#     status(<heading> <value1> [<value2> ...])
#     status(<heading> <condition> THEN <text for TRUE> else <text for FALSE> )
# ----------------------------------------------------------------------------
function(status text)
  set(status_cond)
  set(status_then)
  set(status_else)

  set(status_current_name "cond")
  foreach(arg ${ARGN})
    if(arg STREQUAL "THEN")
      set(status_current_name "then")
    elseif(arg STREQUAL "ELSE")
      set(status_current_name "else")
    else()
      list(APPEND status_${status_current_name} ${arg})
    endif()
  endforeach()

  if(DEFINED status_cond)
    set(status_placeholder_length 32)
    string(RANDOM LENGTH ${status_placeholder_length} ALPHABET " " status_placeholder)
    string(LENGTH "${text}" status_text_length)
    if(status_text_length LESS status_placeholder_length)
      string(SUBSTRING "${text}${status_placeholder}" 0 ${status_placeholder_length} status_text)
    elseif(DEFINED status_then OR DEFINED status_else)
      rmvl_output_status("${text}")
      set(status_text "${status_placeholder}")
    else()
      set(status_text "${text}")
    endif()

    if(DEFINED status_then OR DEFINED status_else)
      if(${status_cond})
        string(REPLACE ";" " " status_then "${status_then}")
        string(REGEX REPLACE "^[ \t]+" "" status_then "${status_then}")
        rmvl_output_status("${status_text} ${status_then}")
      else()
        string(REPLACE ";" " " status_else "${status_else}")
        string(REGEX REPLACE "^[ \t]+" "" status_else "${status_else}")
        rmvl_output_status("${status_text} ${status_else}")
      endif()
    else()
      string(REPLACE ";" " " status_cond "${status_cond}")
      string(REGEX REPLACE "^[ \t]+" "" status_cond "${status_cond}")
      rmvl_output_status("${status_text} ${status_cond}")
    endif()
  else()
    rmvl_output_status("${text}")
  endif()
endfunction()

# ----------------------------------------------------------------------------
#   下载并参与 RMVL 的构建，支持 URL 和 GIT
#   用法:
#     rmvl_download(
#       <dl_name> <dl_kind>
#       <...>
#     )
#   示例 1:
#     rmvl_download(
#       googletest URL "xxx"
#     )
#   示例 2:
#     rmvl_download(
#       benchmark GIT "xxx@master"
#     )
# ----------------------------------------------------------------------------
function(rmvl_download dl_name dl_kind dl_info)
  include(FetchContent)
  string(TOLOWER "${dl_kind}" dl_kind_lower)

  set(dl_args)
  # DOWNLOAD_EXTRACT_TIMESTAMP is added in CMake 3.24
  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
    list(APPEND dl_args DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  endif()

  # use git
  if("${dl_kind_lower}" STREQUAL "git")
    string(REGEX MATCH "(.*)@(.*)" RESULT "${dl_info}")
    set(git_url "${CMAKE_MATCH_1}")
    set(git_tag "${CMAKE_MATCH_2}")
    message(STATUS "Download ${dl_name} from \"${git_url}\" (tag: ${git_tag})")
    FetchContent_Declare(
      ${dl_name}
      GIT_REPOSITORY ${git_url}
      GIT_TAG ${git_tag}
      ${dl_args}
    )
  # use url
  elseif("${dl_kind_lower}" STREQUAL "url")
    message(STATUS "Download ${dl_name} from \"${dl_info}\"")
    FetchContent_Declare(
      ${dl_name}
      URL ${dl_info}
      ${dl_args}
    )
  else()
    message(FATAL_ERROR "Unknown download kind: ${dl_kind}")
  endif()
  # make available
  FetchContent_MakeAvailable(${dl_name})
endfunction()
