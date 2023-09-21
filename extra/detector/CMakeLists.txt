rmvl_add_module(
    detector INTERFACE
    DEPENDS imgproc group
)

# armor_detector
rmvl_generate_para(
    armor_detector
    MODULE detector
)
rmvl_add_module(
    armor_detector
    DEPENDS detector ort planar_tracker
)

# gyro_detector
rmvl_generate_para(
    gyro_detector
    MODULE detector
)
rmvl_add_module(
    gyro_detector
    DEPENDS detector ort gyro_tracker gyro_group
)

# rune_detector
rmvl_generate_para(
    rune_detector
    MODULE detector
)
rmvl_add_module(
    rune_detector
    DEPENDS detector rune_group
)

# tag_detector
rmvl_generate_para(
    tag_detector
    MODULE detector
)
rmvl_add_module(
    tag_detector
    DEPENDS detector
    EXTRA_SOURCE common
)

rmvl_generate_module_para(detector)

# build the test program
if(BUILD_TESTS)
    rmvl_add_test(
        detector Unit
        DEPENDS armor_detector rune_detector gyro_detector
        DEPEND_TESTS GTest::gtest_main
    )
endif(BUILD_TESTS)
