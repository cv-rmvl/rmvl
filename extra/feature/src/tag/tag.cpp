/**
 * @file tag.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2023-09-18
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include "rmvl/feature/tag.h"

#include "rmvl/algorithm/math.hpp"
#include "rmvl/core/util.hpp"

namespace rm
{

Tag::ptr Tag::make_feature(const std::vector<cv::Point2f> &corners, char type)
{
    if (type <= '0' || (type >= '9' && type <= 'A') || (type >= 'Z' && type <= 'a') || type >= 'z')
        return nullptr;
    if (corners.size() != 4)
        return nullptr;
    auto retval = std::make_shared<Tag>();

    retval->_corners = std::vector<cv::Point2f>(corners.begin(), corners.end());
    retval->_state["tag"] = std::string(1, type);
    cv::Point2f center;
    retval->_center = std::accumulate(corners.begin(), corners.end(), cv::Point2f(0, 0)) / 4.f;

    double length1 = getDistance(corners[0], corners[1]);
    double length2 = getDistance(corners[1], corners[2]);
    retval->_width = std::max(length1, length2);
    retval->_height = std::min(length1, length2);

    return retval;
}

} // namespace rm
