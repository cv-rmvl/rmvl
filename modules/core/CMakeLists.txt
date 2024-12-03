# ----------------------------------------------------------------------------
#  3rdparty and predefined macros
# ----------------------------------------------------------------------------
find_package(Threads REQUIRED)

if(WITH_OPENCV)
  rmvl_add_module(core EXTERNAL ${CMAKE_THREAD_LIBS_INIT} ${OpenCV_LIBS})
  if(WIN32)
    install(DIRECTORY "${OpenCV_INCLUDE_DIRS}/opencv2" DESTINATION ${RMVL_INCLUDE_INSTALL_PATH})
    install(FILES ${OpenCV_DLL_PATHS} DESTINATION ${RMVL_BIN_INSTALL_PATH})
    install(FILES ${OpenCV_LIB_PATHS} DESTINATION ${RMVL_3P_LIB_INSTALL_PATH})
  endif()
else()
  rmvl_add_module(core EXTERNAL ${CMAKE_THREAD_LIBS_INIT})
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
