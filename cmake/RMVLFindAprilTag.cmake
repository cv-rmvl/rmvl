# ----------------------------------------------------------------------------
#  find apriltag package (find_package) if the apriltag package exists
#  otherwise build the 3rdparty/apriltag
#
#  note: now 'WITH_APRILTAG' is ON !
# ----------------------------------------------------------------------------

unset(apriltag_FOUND)

# no 'BUILD'
if(NOT BUILD_APRILTAG)
  find_package(apriltag QUIET)
  if(apriltag_FOUND)
    message(WARNING "Enable apriltag support but disable to build the 3rdparty/apritag, please check the data of \"tag25h9.c\"")
  else()
    message(WARNING "Invalid apriltag support, please enable BUILD_APRILTAG")
    option(WITH_APRILTAG "Enable open62541 support" OFF)
  endif()
# has 'BUILD'
else()
  set(APRILTAG_PKG apriltag)
  add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/apriltag)
endif()