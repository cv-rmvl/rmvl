# ----------------------------------------------------------------------------
#   nlohmann_json
# ----------------------------------------------------------------------------

project(
  nlohmann_json
  LANGUAGES CXX
)

add_library(nlohmann_json INTERFACE)
# Add aliases to the local build system to ensure consistency with the libraries downloaded by "rmvl_download"
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)

target_include_directories(
  nlohmann_json INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${RMVL_INCLUDE_INSTALL_PATH}>
)

install(
  TARGETS nlohmann_json
  EXPORT nlohmann_jsonTargets
  OPTIONAL
)

# use the namespace "nlohmann_json::"" to guarantee RMVL has the same target name as the official library
# see https://github.com/nlohmann/json
install(
  EXPORT nlohmann_jsonTargets
  FILE nlohmann_jsonTargets.cmake
  NAMESPACE nlohmann_json::
  DESTINATION ${RMVL_CONFIG_INSTALL_PATH}
)

install(
  DIRECTORY include/
  DESTINATION ${RMVL_INCLUDE_INSTALL_PATH}
)
