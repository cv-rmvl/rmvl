# ----------------------------------------------------------------------------
#  Generate para and add module for ml
# ----------------------------------------------------------------------------
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

# --------------------------------------------------------------------------
#  Install the necessary library for OS
# --------------------------------------------------------------------------
if(Ort_FOUND AND DEFINED Ort_LIB)
  rmvl_install_sdk(Ort)
endif()

# ----------------------------------------------------------------------------
#  Build Python bindings
# ----------------------------------------------------------------------------
if(BUILD_PYTHON)
  foreach(m ort)
    if(BUILD_rmvl_${m})
      list(APPEND ml_inc rmvl/ml/${m}.h) 
      list(APPEND ml_dep ${m})
    endif()
  endforeach()
  
  rmvl_generate_python(ml
    FILES ${ml_inc}
    DEPENDS ${ml_dep}
    USE_PREBIND
  )
endif()

# ----------------------------------------------------------------------------
#  Build the test program
# --------------------------------------------------------------------------
if(BUILD_TESTS)

endif(BUILD_TESTS)
