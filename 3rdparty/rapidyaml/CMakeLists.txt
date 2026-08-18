# ----------------------------------------------------------------------------
#   rapidyaml
#   https://github.com/biojppm/rapidyaml
# ----------------------------------------------------------------------------

project(
  rapidyaml
  VERSION 0.16.0
  LANGUAGES CXX
)

aux_source_directory(src/c4 rapidyaml_c4_src)
aux_source_directory(src/c4/yml rapidyaml_c4_yml_src)
set(rapidyaml_src ${rapidyaml_c4_src} ${rapidyaml_c4_yml_src})

add_library(rapidyaml ${rapidyaml_src})

target_include_directories(
  rapidyaml PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${RMVL_INCLUDE_INSTALL_PATH}>
)

# rapidyaml 与内嵌的 c4core 在 Windows 共享库中需要显式导出符号。
if(BUILD_SHARED_LIBS)
  target_compile_definitions(
    rapidyaml
    PUBLIC RYML_SHARED C4CORE_SHARED
    PRIVATE RYML_EXPORTS C4CORE_EXPORTS
  )
endif()

if(RMVL_GNU OR RMVL_CLANG OR RMVL_APPLECLANG)
  target_compile_options(rapidyaml PRIVATE -w)
elseif(RMVL_MSVC)
  target_compile_options(rapidyaml PRIVATE /W0)
endif()

set_target_properties(
  rapidyaml PROPERTIES
  OUTPUT_NAME rapidyaml
  DEBUG_POSTFIX "${RMVL_DEBUG_POSTFIX}"
  ARCHIVE_OUTPUT_DIRECTORY "${3P_LIBRARY_OUTPUT_PATH}"
  LIBRARY_OUTPUT_DIRECTORY "${3P_LIBRARY_OUTPUT_PATH}"
)

install(
  TARGETS rapidyaml
  EXPORT RMVLModules
  ARCHIVE DESTINATION ${RMVL_LIB_INSTALL_PATH}
  LIBRARY DESTINATION ${RMVL_LIB_INSTALL_PATH}
  RUNTIME DESTINATION ${RMVL_BIN_INSTALL_PATH}
)

install(
  DIRECTORY include/
  DESTINATION ${RMVL_INCLUDE_INSTALL_PATH}
)

if(BUILD_TESTS)
  add_executable(rmvl_rapidyaml_test test/test_rapidyaml.cpp)
  target_link_libraries(rmvl_rapidyaml_test PRIVATE rapidyaml)
  add_test(NAME RapidYAML.parse COMMAND rmvl_rapidyaml_test)
endif()
