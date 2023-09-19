/**
 * @file combo.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2023-09-18
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include "rmvl/combo/combo.h"

using namespace rm;

DefaultCombo::DefaultCombo(const feature_ptr &p_feature, int64_t tick) : combo()
{
    _features = {p_feature};
    _height = p_feature->getHeight();
    _width = p_feature->getWidth();
    _center = p_feature->getCenter();
    _angle = p_feature->getAngle();
    _corners = p_feature->getCorners();
    _type = p_feature->getType();
    _tick = tick;
}
