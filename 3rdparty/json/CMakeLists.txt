# ----------------------------------------------------------------------------
#   nlohmann_json
# ----------------------------------------------------------------------------

project(
  nlohmann_json
  LANGUAGES CXX
)

add_library(json INTERFACE)

target_include_directories(
  json INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${RMVL_INCLUDE_INSTALL_PATH}>
)

install(
  TARGETS json
  EXPORT RMVLModules
)

install(
  DIRECTORY include/nlohmann
  DESTINATION ${RMVL_INCLUDE_INSTALL_PATH}
)
