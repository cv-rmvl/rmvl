find_package(OpenCV REQUIRED)

rmvl_add_module(
    ml INTERFACE
    DEPENDS core para
    EXTERNAL ${OpenCV_LIBS}
)

# add module: ort
find_package(Ort)
set(BUILD_rmvl_ort_INIT ${Ort_FOUND})

rmvl_generate_para(
    ort
    MODULE ml
)

rmvl_add_module(
    ort
    DEPENDS ml
    EXTRA_HEADER ${Ort_INCLUDE_DIRS}
    EXTERNAL ${Ort_LIBS}
)

rmvl_generate_module_para(ml)

# test
if(BUILD_TESTS)

endif(BUILD_TESTS)
