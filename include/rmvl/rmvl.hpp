/**
 * @file rmvl.hpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2022-11-26
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#ifndef RMVL_ALL_HPP
#define RMVL_ALL_HPP

//! modules definitions
#include <rmvl/rmvl_modules.hpp>

//! core module
#include "rmvl/core.hpp"

#ifdef HAVE_RMVL_ML
#include "rmvl/ml.hpp"
#endif //! HAVE_RMVL_ML

#ifdef HAVE_RMVL_CAMERA
#include "rmvl/camera.hpp"
#endif //! HAVE_RMVL_CAMERA

#ifdef HAVE_RMVL_DATAIO
#include "rmvl/dataio.hpp"
#endif //! HAVE_RMVL_DATAIO

#ifdef HAVE_RMVL_FILTER
#include "rmvl/filter.hpp"
#endif //! HAVE_RMVL_FILTER

#ifdef HAVE_RMVL_IMGPROC
#include "rmvl/imgproc.hpp"
#endif //! HAVE_RMVL_IMGPROC

#ifdef HAVE_RMVL_RMATH
#include "rmvl/rmath.hpp"
#endif //! HAVE_RMVL_RMATH

/////////////////////////////////////
///////    autoaim modules    ///////
/////////////////////////////////////

#ifdef HAVE_RMVL_FEATURE
#include "rmvl/feature.hpp"
#endif //! HAVE_RMVL_FEATURE

#ifdef HAVE_RMVL_COMBO
#include "rmvl/combo.hpp"
#endif //! HAVE_RMVL_COMBO

#ifdef HAVE_RMVL_TRACKER
#include "rmvl/tracker.hpp"
#endif //! HAVE_RMVL_TRACKER

#ifdef HAVE_RMVL_GROUP
#include "rmvl/group.hpp"
#endif //! HAVE_RMVL_GROUP

#ifdef HAVE_RMVL_DETECTOR
#include "rmvl/detector.hpp"
#endif //! HAVE_RMVL_DETECTOR

#ifdef HAVE_RMVL_COMPENSATOR
#include "rmvl/compensator.hpp"
#endif //! HAVE_RMVL_COMPENSATOR

#ifdef HAVE_RMVL_PREDICTOR
#include "rmvl/predictor.hpp"
#endif //! HAVE_RMVL_PREDICTOR

#ifdef HAVE_RMVL_DECIDER
#include "rmvl/decider.hpp"
#endif //! HAVE_RMVL_DECIDER

#ifdef HAVE_RMVL_TYPES
#include "rmvl/types.hpp"
#endif //! HAVE_RMVL_TYPES

#endif //! RMVL_ALL_HPP
