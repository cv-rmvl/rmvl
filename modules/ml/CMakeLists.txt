rmvl_add_module(
  ml INTERFACE
  DEPENDS core
)

# add module: ort
if(WITH_ONNXRUNTIME)
  find_package(Ort)
endif()

set(BUILD_rmvl_ort_INIT ${WITH_ONNXRUNTIME})

rmvl_add_module(
  ort
  DEPENDS ml
  EXTRA_HEADER ${Ort_INCLUDE_DIRS}
  EXTERNAL ${Ort_LIBS}
)

# test
if(BUILD_TESTS)

endif(BUILD_TESTS)
