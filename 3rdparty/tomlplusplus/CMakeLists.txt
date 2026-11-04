# ----------------------------------------------------------------------------
#   tomlplusplus
#   https://github.com/marzer/tomlplusplus
# ----------------------------------------------------------------------------

project(
  tomlplusplus
  VERSION 3.4.0
  LANGUAGES CXX
)

add_library(tomlplusplus INTERFACE)
add_library(tomlplusplus::tomlplusplus ALIAS tomlplusplus)

target_include_directories(
  tomlplusplus SYSTEM INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${RMVL_INCLUDE_INSTALL_PATH}>
)

target_compile_features(tomlplusplus INTERFACE cxx_std_17)

install(
  TARGETS tomlplusplus
  EXPORT RMVLModules
)

install(
  DIRECTORY include/
  DESTINATION ${RMVL_INCLUDE_INSTALL_PATH}
)

install(
  FILES LICENSE
  DESTINATION ${RMVL_INCLUDE_INSTALL_PATH}/toml++
)
