# ----------------------------------------------------------------------------
#   apriltag
# ----------------------------------------------------------------------------

project(
    ${APRILTAG_PKG}
    LANGUAGES C
)

unset(main_dir)
unset(common_dir)

aux_source_directory(. main_dir)
aux_source_directory(common common_dir)

add_library(
    ${APRILTAG_PKG}
    STATIC ${main_dir} ${common_dir}
)

target_compile_options(${APRILTAG_PKG} PRIVATE -w)

target_include_directories(
    ${APRILTAG_PKG} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

set_target_properties(
    ${APRILTAG_PKG} PROPERTIES
    OUTPUT_NAME ${APRILTAG_PKG}
    ARCHIVE_OUTPUT_DIRECTORY ${3P_LIBRARY_OUTPUT_PATH}
)

if(NOT BUILD_SHARED_LIBS)
    install(
        TARGETS ${APRILTAG_PKG}
        EXPORT RMVLModules
        ARCHIVE DESTINATION ${RMVL_3P_LIB_INSTALL_PATH}
        OPTIONAL
    )
endif()
