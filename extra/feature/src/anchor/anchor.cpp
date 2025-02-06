/**
 * @file anchor.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2025-02-06
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include "anchor_def.hpp"

namespace rm
{

Anchor::ptr Anchor::make_feature(const std::vector<cv::Point> &contour, AnchorType type)
{
    if (type == AnchorType::Circle)
    {
        auto circle_info = createAnchorFromCircleContour(contour);
        if (circle_info.has_value())
        {
            auto value = circle_info.value();
            auto retval = std::make_shared<Anchor>();
            auto &state = retval->_state;
            state["anchor"] = "circle";
            state["e"] = value.eccentricity;
            retval->_center = value.center;
            retval->_width = retval->_height = value.radius * 2;
            return retval;
        }
        else
            return nullptr;
    }
    else if (type == AnchorType::Square)
    {
        auto square_info = createAnchorFromSqaureContour(contour);
        if (square_info.has_value())
        {
            auto value = std::move(square_info.value());
            auto retval = std::make_shared<Anchor>();
            auto &state = retval->_state;
            state["anchor"] = "square";
            retval->_center = value.center;
            retval->_width = retval->_height = value.length;
            retval->_corners = value.corners;
            retval->_angle = value.angle;
            return retval;
        }
        else
            return nullptr;
    }
    else if (type == AnchorType::Cross)
    {
        auto cross_info = createAnchorFromCrossContour(contour);
        if (cross_info.has_value())
        {
            auto value = cross_info.value();
            auto retval = std::make_shared<Anchor>();
            auto &state = retval->_state;
            state["anchor"] = "cross";
            retval->_center = value.center;
            retval->_width = retval->_height = value.length;
            retval->_corners = value.corners;
            retval->_angle = value.angle;
            return retval;
        }
        else
            return nullptr;
    }
    return nullptr;
}

std::string_view Anchor::to_string(AnchorType type)
{
    switch (type)
    {
    case AnchorType::Circle:
        return "circle";
    case AnchorType::Square:
        return "square";
    case AnchorType::Cross:
        return "cross";
    default:
        return "unknown";
    }
}

AnchorType Anchor::from_string(std::string_view type)
{
    if (type == "circle")
        return AnchorType::Circle;
    if (type == "square")
        return AnchorType::Square;
    if (type == "cross")
        return AnchorType::Cross;
    return AnchorType::Unknown;
}

} // namespace rm
