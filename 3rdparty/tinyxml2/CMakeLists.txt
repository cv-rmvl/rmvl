# ----------------------------------------------------------------------------
#   tinyxml2
# ----------------------------------------------------------------------------

project(
  tinyxml2
  LANGUAGES CXX
)

add_library(tinyxml2 src/tinyxml2.cpp)

target_include_directories(
  tinyxml2 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${RMVL_INCLUDE_INSTALL_PATH}>
)

install(
  TARGETS tinyxml2
  EXPORT RMVLModules
)

install(
  DIRECTORY include/tinyxml2
  DESTINATION ${RMVL_INCLUDE_INSTALL_PATH}
)
