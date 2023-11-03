# ----------------------------------------------------------------------------
#  find open62541 package (find_package) if the open62541 package exists
#  otherwise build the 3rdparty/open62541
#
#  note: now 'WITH_OPEN62541' is ON !
# ----------------------------------------------------------------------------

unset(open62541_FOUND)

# no 'BUILD'
if(NOT BUILD_OPEN62541)
  message(STATUS "Looking for open62541")
  find_package(open62541 QUIET)
  if(NOT open62541_FOUND)
    message(STATUS "Looking for open62541 - not found")
    rmvl_option(WITH_OPEN62541 "Enable open62541 support" OFF)
  else()
    message(STATUS "Looking for open62541 - found")
  endif()
# has 'BUILD'
else()
  add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/open62541)
endif()
