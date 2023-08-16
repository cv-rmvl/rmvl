find_package(OpenCV REQUIRED)

rmvl_add_exe(
    version
    SOURCES sample_version.cpp
    DEPENDS core
    EXTERNAL opencv_core
)