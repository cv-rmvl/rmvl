/**
 * @file rmvlpara.hpp
 * @author RoboMaster Vision Community
 * @brief 
 * @version 1.0
 * @date 2022-12-05
 * 
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 * 
 */

#ifndef RMVLPARA_ALL_HPP
#define RMVLPARA_ALL_HPP

//! modules definitions
#include <rmvl/rmvl_modules.hpp>

#ifdef HAVE_RMVL_CAMERA
#include "rmvlpara/camera.hpp"
#endif //! HAVE_RMVL_CAMERA

#ifdef HAVE_RMVL_ML
#include "rmvlpara/ml.hpp"
#endif //! HAVE_RMVL_ML

/////////////////////////////////////
///////    autoaim modules    ///////
/////////////////////////////////////

#ifdef HAVE_RMVL_FEATURE
#include "rmvlpara/feature.hpp"
#endif //! HAVE_RMVL_FEATURE

#ifdef HAVE_RMVL_COMBO
#include "rmvlpara/combo.hpp"
#endif //! HAVE_RMVL_COMBO

#ifdef HAVE_RMVL_TRACKER
#include "rmvlpara/tracker.hpp"
#endif //! HAVE_RMVL_TRACKER

#ifdef HAVE_RMVL_GROUP
#include "rmvlpara/group.hpp"
#endif //! HAVE_RMVL_GROUP

#ifdef HAVE_RMVL_DETECTOR
#include "rmvlpara/detector.hpp"
#endif //! HAVE_RMVL_DETECTOR

#ifdef HAVE_RMVL_COMPENSATOR
#include "rmvlpara/compensator.hpp"
#endif //! HAVE_RMVL_COMPENSATOR

#ifdef HAVE_RMVL_PREDICTOR
#include "rmvlpara/predictor.hpp"
#endif //! HAVE_RMVL_PREDICTOR

#ifdef HAVE_RMVL_DECIDER
#include "rmvlpara/decider.hpp"
#endif //! HAVE_RMVL_DECIDER

#endif // !RMVLPARA_ALL_HPP
