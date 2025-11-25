# ----------------------------------------------------------------------------
#   fmt
# ----------------------------------------------------------------------------

project(
  fmt
  LANGUAGES CXX
)

aux_source_directory(src fmt_src)

add_library(fmt ${fmt_src})

target_include_directories(
  fmt PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${RMVL_INCLUDE_INSTALL_PATH}>
)

install(
  TARGETS fmt
  EXPORT RMVLModules
  ARCHIVE DESTINATION ${RMVL_LIB_INSTALL_PATH}
  LIBRARY DESTINATION ${RMVL_LIB_INSTALL_PATH}
)

install(
  DIRECTORY include/fmt
  DESTINATION ${RMVL_INCLUDE_INSTALL_PATH}
)

if(BUILD_TESTS)
  add_executable(rmvl_fmt_test test/test_fmt.cpp)
  target_link_libraries(rmvl_fmt_test PRIVATE fmt)
  add_test(FMT.str ${CMAKE_BINARY_DIR}/bin/rmvl_fmt_test)
endif()
  