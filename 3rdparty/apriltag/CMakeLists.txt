# ----------------------------------------------------------------------------
#   apriltag
# ----------------------------------------------------------------------------

project(
  apriltag
  LANGUAGES C
)

unset(main_dir)
unset(common_dir)

aux_source_directory(. main_dir)
aux_source_directory(common common_dir)

add_library(
  apriltag
  STATIC ${main_dir} ${common_dir}
)

if(RMVL_GNU OR RMVL_CLANG OR RMVL_APPLECLANG)
  target_compile_options(apriltag PRIVATE -w)
elseif(RMVL_MSVC)
  target_compile_options(apriltag PRIVATE /W0)
endif()

target_include_directories(
  apriltag PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)

set_target_properties(
  apriltag PROPERTIES
  OUTPUT_NAME apriltag
  ARCHIVE_OUTPUT_DIRECTORY ${3P_LIBRARY_OUTPUT_PATH}
)

install(
  TARGETS apriltag
  EXPORT RMVLModules
  ARCHIVE DESTINATION ${RMVL_3P_LIB_INSTALL_PATH}
  OPTIONAL
)

if(UNIX)
  set(apriltag_LIB "libapriltag.a" CACHE INTERNAL "name of the apriltag library")
elseif(WIN32)
  set(apriltag_LIB "apriltag.lib" CACHE INTERNAL "name of the apriltag library")
endif()
