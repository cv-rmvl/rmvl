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

Tag::Tag(const std::vector<cv::Point2f> &corners, char type)
{
    RMVL_DbgAssert((type >= 'A' && type <= 'Z') || (type >= '0' && type <= '9'));
    size_t corners_size = corners.size();
    if (corners_size != 4)
        RMVL_Error_(RMVL_StsBadArg, "the size of the argument \"corners\" should be 4, but now it is %zu.", corners_size);
    _corners = std::vector<cv::Point2f>(corners.begin(), corners.end());
    _state["tag"] = std::string(type, 1);
    cv::Point2f center;
    for (const auto &corner : corners)
        center += corner;
    center /= static_cast<float>(corners_size);
    _center = center;

    double length1 = getDistance(corners[0], corners[1]);
    double length2 = getDistance(corners[1], corners[2]);
    _width = length1 > length2 ? length1 : length2;
    _height = _width == length1 ? length2 : length1;
}

} // namespace rm
