# ----------------------------------------------------------------------------
#   Uninstall target, for "make uninstall"
# ----------------------------------------------------------------------------
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/templates/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    @ONLY
  )
  add_custom_target(
    uninstall
    COMMENT "Uninstall all files created by the build process."
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  )
endif()
