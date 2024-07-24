rmvl_update_build(ml WITH_OPENCV)
rmvl_add_module(
  ml INTERFACE
  DEPENDS algorithm
)

rmvl_update_build(ort WITH_ONNXRUNTIME AND BUILD_rmvl_ml_INIT)

rmvl_add_module(
  ort
  DEPENDS ml
  EXTRA_HEADER ${Ort_INCLUDE_DIRS}
  EXTERNAL ${Ort_LIBS}
)

# test
if(BUILD_TESTS)

endif(BUILD_TESTS)
