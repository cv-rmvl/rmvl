set(OPTLightCtrl_root_path "/opt/OPT/OPTController")

# add the include directories path
list(APPEND OPTLightCtrl_INCLUDE_DIR "${OPTLightCtrl_root_path}/include")
find_path(
    OPTLightCtrl_INCLUDE_DIR
    PATH "${OPTLightCtrl_INCLUDE_DIR}"
    NO_DEFAULT_PATH
)

# add libraries
find_library(
    OPTLightCtrl_LIB
    NAMES "libOPTController.so"
    PATHS "${OPTLightCtrl_root_path}/lib"
    NO_DEFAULT_PATH
)

mark_as_advanced(OPTLightCtrl_INCLUDE_DIR OPTLightCtrl_LIB)

set(OPTLightCtrl_INCLUDE_DIRS "${OPTLightCtrl_INCLUDE_DIR}")
set(OPTLightCtrl_LIBS "${OPTLightCtrl_LIB}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    OPTLightCtrl
    REQUIRED_VARS OPTLightCtrl_LIB
)