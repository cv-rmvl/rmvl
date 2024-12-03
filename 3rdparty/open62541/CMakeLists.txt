# ----------------------------------------------------------------------------
#   open62541
# ----------------------------------------------------------------------------

project(
  open62541
  LANGUAGES C
)

option(UA_ENABLE_PUBSUB "Enable the PubSub protocol" ON)
rmvl_download(open62541 GIT "https://github.com/open62541/open62541.git : v1.3.8")
set(open62541_VERSION "1.3.8" CACHE INTERNAL "open62541 version")

install(
  TARGETS open62541
  EXPORT RMVLModules
  ARCHIVE DESTINATION ${RMVL_3P_LIB_INSTALL_PATH}
  OPTIONAL
)

if(UNIX)
  set(open62541_LIB "libopen62541.a" CACHE INTERNAL "name of the open62541 library")
elseif(WIN32)
  set(open62541_LIB "open62541.lib" CACHE INTERNAL "name of the open62541 library")
endif()
