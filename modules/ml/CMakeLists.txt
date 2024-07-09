set(BUILD_rmvl_ml_INIT ${WITH_OPENCV})
rmvl_add_module(
  ml INTERFACE
  DEPENDS algorithm
)

if(WITH_ONNXRUNTIME AND BUILD_rmvl_ml_INIT)
  set(BUILD_rmvl_ort_INIT ON)
else()
  set(BUILD_rmvl_ort_INIT OFF)
endif()

rmvl_add_module(
  ort
  DEPENDS ml
  EXTRA_HEADER ${Ort_INCLUDE_DIRS}
  EXTERNAL ${Ort_LIBS}
)

# test
if(BUILD_TESTS)

endif(BUILD_TESTS)
