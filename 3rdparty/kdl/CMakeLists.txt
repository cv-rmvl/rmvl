# ----------------------------------------------------------------------------
#   KDL (Kinematics and Dynamics Library)
#   https://github.com/orocos/orocos_kinematics_dynamics
# ----------------------------------------------------------------------------
project(kdl LANGUAGES CXX)

# ----------------------------------------------------------------------------
#   补全 config.h.in 的两个 cmakedefine
# ----------------------------------------------------------------------------
configure_file(
  src/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  @ONLY
)

# ----------------------------------------------------------------------------
#   编译
# ----------------------------------------------------------------------------
aux_source_directory(src kdl_srcs)
aux_source_directory(src/utilities kdl_utility_srcs)

add_library(kdl ${kdl_srcs} ${kdl_utility_srcs})

target_include_directories(
  kdl PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_include_directories(
  kdl PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)
  
target_link_libraries(kdl PUBLIC Eigen3::Eigen)

# if(RMVL_GNU OR RMVL_CLANG OR RMVL_APPLECLANG)
#   target_compile_options(kdl PRIVATE -w)
# elseif(RMVL_MSVC)
#   target_compile_options(kdl PRIVATE /W0)
# endif()

# ----------------------------------------------------------------------------
#   安装
# ----------------------------------------------------------------------------
install(
  TARGETS kdl
  EXPORT kdlTargets
  ARCHIVE DESTINATION ${RMVL_3P_LIB_INSTALL_PATH}
  OPTIONAL
)
install(
  EXPORT kdlTargets
  FILE kdlTargets.cmake
  DESTINATION ${RMVL_CONFIG_INSTALL_PATH}
)