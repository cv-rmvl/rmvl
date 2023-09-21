rmvl_add_module(
    types
    DEPENDS core
)

# build the test program
if(BUILD_TESTS)
    rmvl_add_test(
        types Unit
        DEPENDS types
        DEPEND_TESTS GTest::gtest_main
    )
endif()
