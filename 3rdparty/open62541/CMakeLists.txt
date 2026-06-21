# ----------------------------------------------------------------------------
#   open62541
# ----------------------------------------------------------------------------

project(
  open62541
  LANGUAGES C
)

set(SAVED_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

option(UA_ENABLE_PUBSUB "Enable the PubSub protocol" ON)
rmvl_download(open62541 GIT "https://github.com/open62541/open62541.git@v1.5.4")

set(CMAKE_C_FLAGS "${SAVED_CMAKE_C_FLAGS}" CACHE INTERNAL "C Compiler Flags")

set(open62541_VERSION "1.5.4" CACHE INTERNAL "open62541 version")
