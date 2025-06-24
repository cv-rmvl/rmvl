# ----------------------------------------------------------------------------
#  3rdparty and predefined macros
# ----------------------------------------------------------------------------
find_package(Threads REQUIRED)

rmvl_add_module(core EXTERNAL ${CMAKE_THREAD_LIBS_INIT})
if(WITH_OPENCV)
  rmvl_link_libraries(core PUBLIC ${OpenCV_LIBS})
  if(WIN32)
    rmvl_link_libraries(core PUBLIC opencv_world)
    install(DIRECTORY "${OpenCV_INCLUDE_DIRS}/opencv2" DESTINATION ${RMVL_INCLUDE_INSTALL_PATH})
    install(FILES ${OpenCV_DLL_PATHS} DESTINATION ${RMVL_BIN_INSTALL_PATH})
    install(FILES ${OpenCV_LIB_PATHS} DESTINATION ${RMVL_3P_LIB_INSTALL_PATH})
  endif()
endif()

if(WITH_EIGEN3)
  rmvl_link_libraries(core PUBLIC Eigen3::Eigen)
endif()

# ----------------------------------------------------------------------------
#  Build Python bindings
# ----------------------------------------------------------------------------
if(BUILD_PYTHON)
  foreach(m io timer version)
    list(APPEND core_inc rmvl/core/${m}.hpp)
  endforeach()
  rmvl_generate_python(core
    FILES ${core_inc}
    DEPENDS core
  )
endif()

# ----------------------------------------------------------------------------
#  Build the test program
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  rmvl_add_test(
    core Unit
    DEPENDS core
    EXTERNAL GTest::gtest_main
  )
endif(BUILD_TESTS)
