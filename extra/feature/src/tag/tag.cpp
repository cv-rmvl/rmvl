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

#include "rmvl/core/util.hpp"
#include "rmvl/core/math.hpp"

using namespace std;
using namespace cv;
using namespace rm;

Tag::Tag(const vector<Point2f> &corners, TagType type)
{
    size_t corners_size = corners.size();
    if (corners_size != 4)
        RMVL_Error_(RMVL_StsBadArg, "the size of the argument \"corners\" should be 4, but now it is %zu.", corners_size);
    _corners = corners;
    _type.TagTypeID = type;
    Point2f center;
    for (const auto &corner : corners)
        center += corner;
    center /= static_cast<float>(corners_size);
    _center = center;

    double length1 = getDistance(corners[0], corners[1]);
    double length2 = getDistance(corners[1], corners[2]);
    _width = length1 > length2 ? length1 : length2;
    _height = _width == length1 ? length2 : length1;
}
