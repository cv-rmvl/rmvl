# ----------------------------------------------------------------------------
#   检查 C++ 支持版本
#   用法示例:
#   rmvl_check_cxx(
#       HAVE_CXX17
#       cxx17.cpp "-std=c++17"
#   )
# ----------------------------------------------------------------------------
macro(rmvl_check_cxx result src standard)
    if(NOT "${standard}" STREQUAL "")
        set(build_args " ${standard}")
    endif()
    message(STATUS "Preforming Test ${result} (check file: ${src}${build_args})")
    try_compile(
        ${result}
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/cpp${standard}
        SOURCES ${CMAKE_CURRENT_LIST_DIR}/check/${src}
        COMPILE_DEFINITIONS "${standard}"
    )

    if(${result})
        set(${result} 1 CACHE INTERNAL "Test ${result}")
        message(STATUS "Performing Test ${result} - Success")
    else()
        set(${result} "" CACHE INTERNAL "Test ${result}")
        message(STATUS "Performing Test ${result} - Failed")
    endif()
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
        file(READ "${RMVL_BUILD_INFO_FILE}" __content)
    else()
        set(__content "")
    endif()
    if(NOT "${__content}" STREQUAL "${RMVL_BUILD_INFO_STR}")
        file(WRITE "${RMVL_BUILD_INFO_FILE}" "${RMVL_BUILD_INFO_STR}")
    endif()
    unset(__content)
    unset(RMVL_BUILD_INFO_STR CACHE)

    if(DEFINED RMVL_MODULE_rmvl_core_BINARY_DIR)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${RMVL_BUILD_INFO_FILE}" "${RMVL_MODULE_rmvl_core_BINARY_DIR}/version_string.inc"
            OUTPUT_QUIET
        )
    endif()
endmacro()

# ----------------------------------------------------------------------------
#   状态报告，自动对齐右列，并在条件语句中自动选择文本
#   用法:
#       status(<text>)
#       status(<heading> <value1> [<value2> ...])
#       status(<heading> <condition> THEN <text for TRUE> else <text for FALSE> )
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
#   下载并参与 RMVL 的构建
#   用法:
#       rmvl_download(
#           <dl_name> <dl_kind>
#           SOURCE <...>
#       )
#   示例 1:
#       rmvl_download(
#           googletest URL
#           xxx
#       )
#   示例 2:
#       rmvl_download(
#           benchmark GIT
#           xxx : master
#       )
# ----------------------------------------------------------------------------
function(rmvl_download dl_name dl_kind)
    if(NOT "${dl_kind}" MATCHES "^(GIT|URL)$")
        message(FATAL_ERROR "Unknown download kind : ${dl_kind}")
    endif()
    include(FetchContent)
    string(TOLOWER "${dl_kind}" dl_kind_lower)
    # get url-string
    list(LENGTH ARGN argn_length)
    if(NOT argn_length EQUAL 1)
        message(FATAL_ERROR "Argument \"\$\{ARGN\}\" count must be 1 in the function \"rmvl_download\"")
    endif()
    list(GET ARGN 0 web_url)
    # use git
    if("${dl_kind_lower}" STREQUAL "git")
        string(REGEX MATCH "(.*) : (.*)" RESULT "${web_url}")
        set(git_url "${CMAKE_MATCH_1}")
        set(git_tag "${CMAKE_MATCH_2}")
        message(STATUS "Download ${dl_name} from \"${git_url}\" (tag: ${git_tag})")
        FetchContent_Declare(
            ${dl_name}
            GIT_REPOSITORY ${git_url}
            GIT_TAG ${git_tag}
        )
    # use url
    else()
        message(STATUS "Download ${dl_name} from \"${web_url}\"")
        FetchContent_Declare(
            ${dl_name}
            URL ${web_url}
        )
    endif()
    # make available
    FetchContent_MakeAvailable(${dl_name})
endfunction()
