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

_rmvl_set_target_in_3rd(open62541)