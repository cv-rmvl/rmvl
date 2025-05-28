# 局部变量（为每个模块所设置）
#
# _name      - 小写短名，例如 core
# the_module - 小写全名，例如 rmvl_core

# 全局变量
#
# RMVL_MODULES_BUILD
#
# RMVL_MODULE_${the_module}_LOCATION
# RMVL_MODULE_${the_module}_BINARY_DIR
#
# 专为 rmvl_world 模块设置的全局变量
#
# RMVL_WORLD_SOURCES
# RMVL_WORLD_EXTRA_INCLUDES
# RMVL_WORLD_EXTRA_LINKS
# RMVL_WORLD_EXTRA_DEFINITIONS

set(RMVL_MODULES_BUILD "" CACHE INTERNAL "List of RMVL modules included into the build")
set(RMVL_WORLD_SOURCES "" CACHE INTERNAL "Source files of RMVL modules")
set(RMVL_WORLD_EXTRA_INCLUDES "" CACHE INTERNAL "Source files of RMVL modules")
set(RMVL_WORLD_EXTRA_LINKS "" CACHE INTERNAL "Source files of RMVL modules")
set(RMVL_WORLD_EXTRA_DEFINITIONS "" CACHE INTERNAL "Source files of RMVL modules")

# ----------------------------------------------------------------------------
#   将预处理定义添加至指定目标
#   用法:
#   rmvl_compile_definitions(<target>
#     <INTERFACE | PUBLIC | PRIVATE> [items1...]
#     [<INTERFACE | PUBLIC | PRIVATE> [items2...] ...])
#   示例:
#   rmvl_compile_definitions(
#     aaa
#     INTERFACE HAVE_AAA
#   )
# ----------------------------------------------------------------------------
macro(rmvl_compile_definitions _target)
  if(BUILD_WORLD)
    target_compile_definitions(rmvl_world ${ARGN})
  else()
    target_compile_definitions(rmvl_${_target} ${ARGN})
  endif()
endmacro(rmvl_compile_definitions _target)

# ----------------------------------------------------------------------------
#   将指定路径下的所有文件安装至特定目标
#   用法:
#     rmvl_install_directories(<directory> [DESTINATION])
#   示例:
#     rmvl_install_directories(include/rmvl)
# ----------------------------------------------------------------------------
function(rmvl_install_directories _dir)
  cmake_parse_arguments(IS "" "DESTINATION" "" ${ARGN})
  # Search
  if(IS_ABSOLUTE ${_dir})
    set(current_dir "${_dir}")
  else()
    set(current_dir "${CMAKE_CURRENT_LIST_DIR}/${_dir}")
  endif()
  file(GLOB subs RELATIVE "${current_dir}" "${current_dir}/*")
  foreach(sub ${subs})
    if(IS_DIRECTORY ${current_dir}/${sub})
      set(dirs ${dirs} "${current_dir}/${sub}")
    else()
      set(files ${files} "${current_dir}/${sub}")
    endif()
  endforeach(sub ${subs})
  
  # Install
  if(IS_DESTINATION)
    set(install_dir ${IS_DESTINATION})
  else()
    set(install_dir ${_dir})
  endif()

  install(DIRECTORY ${dirs} DESTINATION ${install_dir} OPTIONAL)
  install(FILES ${files} DESTINATION ${install_dir} OPTIONAL)
endfunction(rmvl_install_directories)

# ----------------------------------------------------------------------------
#   更新 RMVL 目标构建的标志位，用于控制是否构建指定模块，不影响用户的选择
#   - 主要更新 `BUILD_rmvl_xxx_INIT` 变量
#   - 在满足条件时，设置 `BUILD_rmvl_xxx_INIT` 为 `ON`，否则为 `OFF`
#   用法:
#     rmvl_update_build(<name> [CONDITION])
#   示例:
#     rmvl_update_build(
#       hik_camera
#       HikSDK_FOUND
#     )
# ----------------------------------------------------------------------------
macro(rmvl_update_build _name)
  if(${ARGN})
    set(BUILD_rmvl_${_name}_INIT ON)
  else()
    set(BUILD_rmvl_${_name}_INIT OFF)
  endif()
endmacro()

# ----------------------------------------------------------------------------
#   在当前目录中添加新的 RMVL 模块，并会依次添加至
#   - 局部变量 modules_build
#   - 缓存变量 RMVL_MODULES_BUILD
#   中
#
#   用法:
#     rmvl_add_module(<name> [INTERFACE] [EXTRA_HEADER <list of other include directories>]
#       [EXTRA_SOURCE <list of other source directories>] [DEPENDS <list of rmvl dependencies>]
#       [EXTERNAL <list of 3rd party dependencies>])
#   示例:
#     rmvl_add_module(
#       my_module               # 需要生成的模块 (文件夹名)
#       EXTRA_HEADER xxx_h      # 参与构建的除 include 文件夹以外的其余头文件目录
#       EXTRA_SOURCE xxx_src    # 参与构建的除 src 文件夹以外的其余源文件目录
#       DEPENDS core            # 依赖的 RMVL 模块 (文件夹名)
#       EXTERNAL ${OpenCV_LIBS} # 依赖的第三方目标库
#     )
# ----------------------------------------------------------------------------
macro(rmvl_add_module _name)
  # Configure arguments parser
  set(options INTERFACE)
  set(multi_args DEPENDS EXTRA_HEADER EXTRA_SOURCE EXTERNAL)
  cmake_parse_arguments(MD "${options}" "" "${multi_args}" ${ARGN})

  # Module information
  set(the_module rmvl_${_name})
  set(
    RMVL_MODULE_${the_module}_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}"
    CACHE INTERNAL "Location of ${the_module} module sources"
  )

  if(NOT DEFINED BUILD_${the_module}_INIT)
    set(BUILD_${the_module}_INIT ON)
  endif()
  option(BUILD_${the_module} "Include ${the_module} module into the RMVL build" ${BUILD_${the_module}_INIT}) # create option to enable/disable this module
  
  if(BUILD_${the_module})
    # Add library
    if(MD_INTERFACE) # interface library
      add_library(${the_module} INTERFACE)
    else() # public library
      set(RMVL_MODULE_${the_module}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "")
      if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/src/${_name})
        set(module_dir ${CMAKE_CURRENT_LIST_DIR}/src/${_name})
      else()
        set(module_dir ${CMAKE_CURRENT_LIST_DIR}/src)
      endif()
      set(target_src "")
      if(module_dir)
        aux_source_directory(${module_dir} target_src)
      endif()

      # Bind parameter object
      if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/src/${_name}/para)
        set(para_dir ${CMAKE_CURRENT_LIST_DIR}/src/${_name}/para)
      else()
        set(para_dir ${CMAKE_CURRENT_LIST_DIR}/src/para)
      endif()
      set(para_src "")
      if(IS_DIRECTORY ${para_dir})
        if(para_dir)
          aux_source_directory(${para_dir} para_src)
        endif()
      endif(IS_DIRECTORY ${para_dir})
      # Bind extra source files
      set(extra_src "")
      if(MD_EXTRA_SOURCE)
        aux_source_directory(${module_dir}/${MD_EXTRA_SOURCE} extra_src)
      endif()
      # Build to *.so (*.dll), *.a (*.lib) or *.o (*.obj)
      if(BUILD_WORLD)
        set(
          RMVL_WORLD_SOURCES ${RMVL_WORLD_SOURCES} ${target_src} ${para_src} ${extra_src}
          CACHE INTERNAL "Source files of RMVL modules"
        )
      else()
        add_library(${the_module} ${target_src} ${para_src} ${extra_src})
        if(BUILD_SHARED_LIBS)
          set_target_properties(
            ${the_module} PROPERTIES
            DEFINE_SYMBOL RMVLAPI_EXPORTS
          )
        endif()
      endif()
    endif(MD_INTERFACE)

    # Add dependence
    if(MD_INTERFACE) # interface library
      target_include_directories(
        ${the_module}
        INTERFACE ${MD_EXTRA_HEADER}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:${RMVL_INCLUDE_INSTALL_PATH}>
      )
      foreach(_dep ${MD_DEPENDS})
        target_link_libraries(
          ${the_module}
          INTERFACE rmvl_${_dep}
        )
      endforeach(_dep ${MD_DEPENDS})
      target_link_libraries(
        ${the_module}
        INTERFACE ${MD_EXTERNAL}
      )
      # Install
      install(
        TARGETS ${the_module}
        EXPORT RMVLModules
      )
    else() # public library
      if(BUILD_WORLD)
        set(RMVL_WORLD_EXTRA_INCLUDES ${RMVL_WORLD_EXTRA_INCLUDES} ${MD_EXTRA_HEADER} CACHE INTERNAL "Source files of RMVL modules")
        set(RMVL_WORLD_EXTRA_LINKS ${RMVL_WORLD_EXTRA_LINKS} ${MD_EXTERNAL} CACHE INTERNAL "Source files of RMVL modules")
      else()
        target_include_directories(
          ${the_module}
          PUBLIC ${MD_EXTRA_HEADER}
          $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
          $<INSTALL_INTERFACE:${RMVL_INCLUDE_INSTALL_PATH}>
        )
        foreach(_dep ${MD_DEPENDS})
          target_link_libraries(
            ${the_module}
            PUBLIC rmvl_${_dep}
          )
        endforeach(_dep ${MD_DEPENDS})
        target_link_libraries(
          ${the_module}
          PUBLIC ${MD_EXTERNAL}
        )
        set_target_properties(
          ${the_module} PROPERTIES
          OUTPUT_NAME "${the_module}${RMVL_LIBVERSION_SUFFIX}"
          DEBUG_POSTFIX "${RMVL_DEBUG_POSTFIX}"
          COMPILE_PDB_NAME "${the_module}${RMVL_LIBVERSION_SUFFIX}"
          COMPILE_PDB_NAME_DEBUG "${the_module}${RMVL_LIBVERSION_SUFFIX}${RMVL_DEBUG_POSTFIX}"
        )
        # Install
        install(
          TARGETS ${the_module}
          EXPORT RMVLModules
          ARCHIVE DESTINATION ${RMVL_LIB_INSTALL_PATH}
          LIBRARY DESTINATION ${RMVL_LIB_INSTALL_PATH}
        )
      endif()
    endif()
    list(APPEND modules_build ${the_module})
    set(RMVL_MODULES_BUILD ${RMVL_MODULES_BUILD} "${the_module}" CACHE INTERNAL "List of RMVL modules included into the build" FORCE)
  endif()
  unset(the_module)
endmacro(rmvl_add_module _name)

# ----------------------------------------------------------------------------
#   添加 rmvl_world 模块
#   用法:
#     rmvl_add_world()
# ----------------------------------------------------------------------------
macro(rmvl_add_world)
  # Add library
  target_sources(rmvl_world PRIVATE ${RMVL_WORLD_SOURCES})
  # Add dependence
  target_include_directories(rmvl_world PUBLIC ${RMVL_WORLD_EXTRA_INCLUDES})
  foreach(m ${RMVL_MODULES_BUILD})
    target_include_directories(rmvl_world PUBLIC
      $<BUILD_INTERFACE:${RMVL_MODULE_${m}_LOCATION}/include>
    )
  endforeach()
  target_include_directories(
    rmvl_world PUBLIC
    $<INSTALL_INTERFACE:${RMVL_INCLUDE_INSTALL_PATH}>
  )
  target_link_libraries(rmvl_world PUBLIC ${RMVL_WORLD_EXTRA_LINKS})
  set_target_properties(
    rmvl_world PROPERTIES
    OUTPUT_NAME "rmvl_world${RMVL_LIBVERSION_SUFFIX}"
    DEBUG_POSTFIX "${RMVL_DEBUG_POSTFIX}"
    COMPILE_PDB_NAME "rmvl_world${RMVL_LIBVERSION_SUFFIX}"
    COMPILE_PDB_NAME_DEBUG "rmvl_world${RMVL_LIBVERSION_SUFFIX}${RMVL_DEBUG_POSTFIX}"
  )
  # Install
  install(
    TARGETS rmvl_world
    EXPORT RMVLModules
    ARCHIVE DESTINATION ${RMVL_LIB_INSTALL_PATH}
    LIBRARY DESTINATION ${RMVL_LIB_INSTALL_PATH}
  )
  unset(all_obj_modules)
endmacro()

# ----------------------------------------------------------------------------
#   将编译选项添加至指定目标
#   用法:
#   rmvl_compile_options(<target> [BEFORE]
#     <INTERFACE|PUBLIC|PRIVATE> [items1...]
#     [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])
#   示例:
#   rmvl_compile_options(
#     xxxx
#     PRIVATE -w
#   )
# ----------------------------------------------------------------------------
macro(rmvl_compile_options _target)
  if(BUILD_WORLD)
    target_compile_options(rmvl_world ${ARGN})
  else()
    target_compile_options(rmvl_${_target} ${ARGN})
  endif()
endmacro(rmvl_compile_options _target)

# ----------------------------------------------------------------------------
#   此命令用于为指定模块添加新的 RMVL 测试
#   用法:
#   rmvl_add_test(<name> <Unit|Performance> <DEPENDS> [rmvl_target...]
#     <EXTERNAL> [test_target...])
#   示例:
#   rmvl_add_test(
#     detector Unit              # 测试名
#     DEPENDS armor_detector     # 需要依赖的 RMVL 目标库
#     EXTERNAL GTest::gtest_main # 需要依赖的第三方目标库，一般是测试工具库
#   )
# ----------------------------------------------------------------------------
function(rmvl_add_test test_name test_kind)
  if(NOT BUILD_rmvl_${test_name})
    return()
  endif()
  # Add arguments variable
  set(multi_args DEPENDS EXTERNAL)
  if(NOT "${test_kind}" MATCHES "^(Unit|Performance)$")
    message(FATAL_ERROR "Unknown test kind: ${test_kind}")
  endif()
  cmake_parse_arguments(TS "" "" "${multi_args}" ${ARGN})
  string(TOLOWER "${test_kind}" test_kind_lower)
  set(test_report_dir "${CMAKE_BINARY_DIR}/test-reports/${test_kind_lower}")
  file(MAKE_DIRECTORY "${test_report_dir}")

  # Add testing executable
  if("${test_kind_lower}" STREQUAL "performance")
    set(test_suffix "perf_test")
    set(test_dir "perf")
  else()
    set(test_suffix "test")
    set(test_dir "test")
  endif()
  if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${test_dir}/${test_name})
    set(test_dir ${CMAKE_CURRENT_LIST_DIR}/${test_dir}/${test_name})
  else()
    set(test_dir ${CMAKE_CURRENT_LIST_DIR}/${test_dir})
  endif()
  aux_source_directory(${test_dir} test_src)
  set(the_target rmvl_${test_name}_${test_suffix})
  add_executable(${the_target} ${test_src})

  # Local include directories
  target_include_directories(
    ${the_target}
    PRIVATE ${test_dir}
  )

  # Add depends
  if(TS_DEPENDS)
    if(BUILD_WORLD)
      target_link_libraries(${the_target} PRIVATE rmvl_world)
    else()
      foreach(_dep ${TS_DEPENDS})
        if(TARGET rmvl_${_dep})
          target_link_libraries(${the_target} PRIVATE rmvl_${_dep})
        endif()
      endforeach()
    endif()
  endif()

  # Test depends
  target_link_libraries(
    ${the_target}
    PRIVATE ${TS_EXTERNAL}
  )

  # Add test
  if("${test_kind_lower}" STREQUAL "unit")
    gtest_discover_tests(
      ${the_target}
      WORKING_DIRECTORY "${test_report_dir}"
      EXTRA_ARGS "--gtest_output=xml:${test_name}-report.xml"
    )
  else()
    add_test(
      NAME "${the_target}"
      WORKING_DIRECTORY "${test_report_dir}"
      COMMAND "${the_target}" --benchmark_out=${test_name}-perf-report.txt
    )
  endif()
endfunction(rmvl_add_test test_name test_kind)

# ----------------------------------------------------------------------------
#   此命令用于为指定模块添加新的 RMVL 可执行文件
#   用法:
#   rmvl_add_exe(<name> SOURCES <file_name>
#     [DEPENDS <list of rmvl dependencies>]
#     [EXTERNAL <list of 3rd party dependencies>]
#   )
#   示例:
#   rmvl_add_exe(
#     sample_armor_collection
#     SOURCES armor-collection.cpp
#     DEPENDS mv_camera armor_detector
#   )
# ----------------------------------------------------------------------------
macro(rmvl_add_exe exe_name)
  # Add module options
  set(multi_args SOURCES DEPENDS EXTERNAL)
  cmake_parse_arguments(EXE "" "" "${multi_args}" ${ARGN})

  # Add executable
  add_executable(${exe_name} ${EXE_SOURCES})

  # Add dependence
  if(EXE_DEPENDS)
    if(BUILD_WORLD)
      target_link_libraries(${exe_name} rmvl_world)
    else()
      foreach(_dep ${EXE_DEPENDS})
        target_link_libraries(${exe_name} rmvl_${_dep})
      endforeach(_dep ${EXE_DEPENDS})
    endif()
  endif()
  target_link_libraries(${exe_name} ${EXE_EXTERNAL})

  # Install
  install(
    TARGETS ${exe_name}
    RUNTIME DESTINATION ${RMVL_BIN_INSTALL_PATH}
  )
endmacro(rmvl_add_exe exe_name)

# ----------------------------------------------------------------------------
#   设置如何构建指定 Target 的属性
#   用法:
#   rmvl_set_properties(target1 target2 ...
#             PROPERTIES prop1 value1
#             prop2 value2 ...)
#   示例:
#   rmvl_set_properties(
#     detector                   # 目标名
#     PROPERTIES CXX_STANDARD 17 # 属性
#   )
# ----------------------------------------------------------------------------
macro(rmvl_set_properties _target)
  set_target_properties(rmvl_${_target} ${ARGN})
endmacro(rmvl_set_properties _target)

# ----------------------------------------------------------------------------
#   将指定目录添加至运行时动态库链接的搜索路径
#   用法:
#   rmvl_link_directories(<target> [BEFORE]
#     <INTERFACE|PUBLIC|PRIVATE> [items1...]
#     [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])
#   示例:
#   rmvl_link_directories(
#     xxxx
#     PRIVATE /home/xxx/mylib/lib
#   )
# ----------------------------------------------------------------------------
macro(rmvl_link_directories _target)
  target_link_directories(rmvl_${_target} ${ARGN})
endmacro()

# ----------------------------------------------------------------------------
#   将指定目标链接至指定的库
#   用法:
#   rmvl_link_libraries(<target> [BEFORE]
#     <INTERFACE|PUBLIC|PRIVATE> [items1...]
#     [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])
#   示例:
#   rmvl_link_libraries(
#     my_module
#     PRIVATE mylib
#   )
# ----------------------------------------------------------------------------
macro(rmvl_link_libraries _target)
  target_link_libraries(rmvl_${_target} ${ARGN})
endmacro()
