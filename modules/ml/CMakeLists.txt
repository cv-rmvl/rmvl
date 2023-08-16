find_package(OpenCV REQUIRED)

rmvl_add_module(
    ml INTERFACE
    DEPENDS core para
    EXTERNAL ${OpenCV_LIBS}
)

# add module: ort
find_package(Ort)
rmvl_generate_para(
    ort
    MODULE ml
)

if(Ort_FOUND)
    rmvl_add_module(
        ort
        DEPENDS ml
        EXTRA_HEADER ${Ort_INCLUDE_DIRS}
        EXTERNAL ${Ort_LIBS}
    )
else()
    rmvl_add_module(
        ort INTERFACE
        DEPENDS ml
    )
endif()

rmvl_generate_module_para(ml)

# test
if(BUILD_TESTS)

endif(BUILD_TESTS)
