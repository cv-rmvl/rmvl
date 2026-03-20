/**
 * @file msgio.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief LPSS 消息格式化输出工具
 * @version 1.0
 * @date 2026-03-20
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <cctype>

#include "fmt/format.h"

#include "rmvlmsg/geometry/point32.hpp"
#include "rmvlmsg/geometry/pose.hpp"
#include "rmvlmsg/geometry/transform.hpp"
#include "rmvlmsg/geometry/twist.hpp"
#include "rmvlmsg/geometry/vector3.hpp"
#include "rmvlmsg/geometry/wrench.hpp"

namespace rm::lpss::helper {

struct DecimalPrecisionSpec {
    int precision{-1};

    static constexpr bool is_digit(char ch) noexcept { return ch >= '0' && ch <= '9'; }

    constexpr auto parse(fmt::format_parse_context &ctx) {
        auto it = ctx.begin();
        const auto end = ctx.end();
        if (it == end || *it == '}')
            return it;
        if (*it != '.')
            throw fmt::format_error("invalid format, expected '{:.N}'");
        ++it;
        if (it == end || !is_digit(*it))
            throw fmt::format_error("invalid precision, expected digits after '.'");
        precision = 0;
        while (it != end && is_digit(*it)) {
            precision = precision * 10 + (*it - '0');
            ++it;
        }
        if (it != end && *it == 'f')
            ++it;
        if (it != end && *it != '}')
            throw fmt::format_error("invalid format suffix");
        return it;
    }
};

} // namespace rm::lpss::helper

template <>
struct fmt::formatter<rm::msg::Point> : rm::lpss::helper::DecimalPrecisionSpec {
    template <typename FormatContext>
    auto format(const rm::msg::Point &p, FormatContext &ctx) const {
        auto out = fmt::format_to(ctx.out(), "[");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.x, precision) : fmt::format_to(out, "{}", p.x);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.y, precision) : fmt::format_to(out, "{}", p.y);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.z, precision) : fmt::format_to(out, "{}", p.z);
        return fmt::format_to(out, "]");
    }
};

template <>
struct fmt::formatter<rm::msg::Point32> : rm::lpss::helper::DecimalPrecisionSpec {
    template <typename FormatContext>
    auto format(const rm::msg::Point32 &p, FormatContext &ctx) const {
        auto out = fmt::format_to(ctx.out(), "[");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.x, precision) : fmt::format_to(out, "{}", p.x);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.y, precision) : fmt::format_to(out, "{}", p.y);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.z, precision) : fmt::format_to(out, "{}", p.z);
        return fmt::format_to(out, "]");
    }
};

template <>
struct fmt::formatter<rm::msg::Quaternion> : rm::lpss::helper::DecimalPrecisionSpec {
    template <typename FormatContext>
    auto format(const rm::msg::Quaternion &q, FormatContext &ctx) const {
        auto out = fmt::format_to(ctx.out(), "[");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", q.x, precision) : fmt::format_to(out, "{}", q.x);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", q.y, precision) : fmt::format_to(out, "{}", q.y);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", q.z, precision) : fmt::format_to(out, "{}", q.z);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", q.w, precision) : fmt::format_to(out, "{}", q.w);
        return fmt::format_to(out, "]");
    }
};

template <>
struct fmt::formatter<rm::msg::Vector3> : rm::lpss::helper::DecimalPrecisionSpec {
    template <typename FormatContext>
    auto format(const rm::msg::Vector3 &v, FormatContext &ctx) const {
        auto out = fmt::format_to(ctx.out(), "[");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", v.x, precision) : fmt::format_to(out, "{}", v.x);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", v.y, precision) : fmt::format_to(out, "{}", v.y);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", v.z, precision) : fmt::format_to(out, "{}", v.z);
        return fmt::format_to(out, "]");
    }
};

template <>
struct fmt::formatter<rm::msg::Pose> : rm::lpss::helper::DecimalPrecisionSpec {
    template <typename FormatContext>
    auto format(const rm::msg::Pose &p, FormatContext &ctx) const {
        auto out = fmt::format_to(ctx.out(), "[[");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.position.x, precision) : fmt::format_to(out, "{}", p.position.x);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.position.y, precision) : fmt::format_to(out, "{}", p.position.y);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.position.z, precision) : fmt::format_to(out, "{}", p.position.z);
        out = fmt::format_to(out, "], [");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.orientation.x, precision) : fmt::format_to(out, "{}", p.orientation.x);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.orientation.y, precision) : fmt::format_to(out, "{}", p.orientation.y);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.orientation.z, precision) : fmt::format_to(out, "{}", p.orientation.z);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", p.orientation.w, precision) : fmt::format_to(out, "{}", p.orientation.w);
        return fmt::format_to(out, "]]");
    }
};

template <>
struct fmt::formatter<rm::msg::Transform> : rm::lpss::helper::DecimalPrecisionSpec {
    template <typename FormatContext>
    auto format(const rm::msg::Transform &t, FormatContext &ctx) const {
        auto out = fmt::format_to(ctx.out(), "[[");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.translation.x, precision) : fmt::format_to(out, "{}", t.translation.x);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.translation.y, precision) : fmt::format_to(out, "{}", t.translation.y);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.translation.z, precision) : fmt::format_to(out, "{}", t.translation.z);
        out = fmt::format_to(out, "], [");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.rotation.x, precision) : fmt::format_to(out, "{}", t.rotation.x);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.rotation.y, precision) : fmt::format_to(out, "{}", t.rotation.y);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.rotation.z, precision) : fmt::format_to(out, "{}", t.rotation.z);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.rotation.w, precision) : fmt::format_to(out, "{}", t.rotation.w);
        return fmt::format_to(out, "]]");
    }
};

template <>
struct fmt::formatter<rm::msg::Twist> : rm::lpss::helper::DecimalPrecisionSpec {
    template <typename FormatContext>
    auto format(const rm::msg::Twist &t, FormatContext &ctx) const {
        auto out = fmt::format_to(ctx.out(), "[[");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.linear.x, precision) : fmt::format_to(out, "{}", t.linear.x);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.linear.y, precision) : fmt::format_to(out, "{}", t.linear.y);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.linear.z, precision) : fmt::format_to(out, "{}", t.linear.z);
        out = fmt::format_to(out, "], [");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.angular.x, precision) : fmt::format_to(out, "{}", t.angular.x);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.angular.y, precision) : fmt::format_to(out, "{}", t.angular.y);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", t.angular.z, precision) : fmt::format_to(out, "{}", t.angular.z);
        return fmt::format_to(out, "]]");
    }
};

template <>
struct fmt::formatter<rm::msg::Wrench> : rm::lpss::helper::DecimalPrecisionSpec {
    template <typename FormatContext>
    auto format(const rm::msg::Wrench &w, FormatContext &ctx) const {
        auto out = fmt::format_to(ctx.out(), "[[");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", w.force.x, precision) : fmt::format_to(out, "{}", w.force.x);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", w.force.y, precision) : fmt::format_to(out, "{}", w.force.y);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", w.force.z, precision) : fmt::format_to(out, "{}", w.force.z);
        out = fmt::format_to(out, "], [");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", w.torque.x, precision) : fmt::format_to(out, "{}", w.torque.x);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", w.torque.y, precision) : fmt::format_to(out, "{}", w.torque.y);
        out = fmt::format_to(out, ", ");
        out = (precision >= 0) ? fmt::format_to(out, "{:.{}f}", w.torque.z, precision) : fmt::format_to(out, "{}", w.torque.z);
        return fmt::format_to(out, "]]");
    }
};
