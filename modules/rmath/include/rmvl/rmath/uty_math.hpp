/**
 * @file uty_math.hpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-01-12
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <cmath>
#include <unordered_map>

#if __cplusplus > 201703L
#include <numbers>
#endif

#include <opencv2/core/matx.hpp>

#include "rmvl/core/util.hpp"

namespace rm
{

//! @addtogroup rmath
//! @{

// --------------------【结构、类型、常量定义】--------------------
#ifdef MAXFLOAT
constexpr double FLOAT_MAX{MAXFLOAT};
#elif defined HUGE
constexpr double FLOAT_MAX{HUGE};
#endif

#if __cplusplus > 201703L
constexpr double PI{std::numbers::pi};        //!< 圆周率: \f$\pi\f$
constexpr double e{std::numbers::e};          //!< 自然对数底数: \f$e\f$
constexpr double SQRT_2{std::numbers::sqrt2}; //!< 根号 2: \f$\sqrt2\f$
#else
constexpr double PI{3.14159265358979323}; //!< 圆周率: \f$\pi\f$
constexpr double e{2.7182818459045};      //!< 自然对数底数: \f$e\f$
constexpr double SQRT_2{1.4142135623731}; //!< 根号 2: \f$\sqrt2\f$
#endif

constexpr double PI_2{PI / 2.}; //!< PI / 2: \f$\frac\pi2\f$
constexpr double PI_4{PI / 4.}; //!< PI / 4: \f$\frac\pi4\f$
constexpr double g{9.788};      //!< 重力加速度: \f$g\f$

constexpr double operator""_PI(long double num) { return num * PI; }
constexpr double operator""_PI(long long unsigned num) { return num * PI; }

constexpr double operator""_to_rad(long double num) { return num * PI / 180.; }
constexpr double operator""_to_rad(long long unsigned num) { return num * PI / 180.; }

constexpr double operator""_to_deg(long double num) { return num * 180. / PI; }
constexpr double operator""_to_deg(long long unsigned num) { return num * 180. / PI; }

} // namespace rm

// 定义部分 Matx
namespace cv
{

using Matx11f = Matx<float, 1, 1>;
using Matx11d = Matx<double, 1, 1>;
using Matx51f = Matx<float, 5, 1>;
using Matx15f = Matx<float, 1, 5>;

} // namespace cv

namespace rm
{

template <typename Tp>
constexpr Tp operator+(Tp val, const cv::Matx<Tp, 1, 1> &mat) { return val + mat(0, 0); }
template <typename Tp>
constexpr cv::Matx<Tp, 1, 1> operator+(const cv::Matx<Tp, 1, 1> &mat, Tp val) { return cv::Matx<Tp, 1, 1>(mat(0, 0) + val); }
template <typename Tp>
constexpr Tp operator-(Tp val, const cv::Matx<Tp, 1, 1> &mat) { return val - mat(0, 0); }
template <typename Tp>
constexpr cv::Matx<Tp, 1, 1> operator-(const cv::Matx<Tp, 1, 1> &mat, Tp val) { return cv::Matx<Tp, 1, 1>(mat(0, 0) - val); }

//! 角度制式
enum AngleMode : bool
{
    RAD = true, //!< 弧度制
    DEG = false //!< 角度制
};

// ------------------------【常用变换公式】------------------------

/**
 * @brief 角度转换为弧度
 *
 * @tparam Tp 变量类型
 * @param[in] deg 角度
 * @return 弧度
 */
template <typename Tp>
constexpr Tp deg2rad(Tp deg) { return deg * static_cast<Tp>(PI) / static_cast<Tp>(180); }

/**
 * @brief 弧度转换为角度
 *
 * @tparam Tp 变量类型
 * @param[in] rad 弧度
 * @return 角度
 */
template <typename Tp>
constexpr Tp rad2deg(Tp rad) { return rad * static_cast<Tp>(180) / static_cast<Tp>(PI); }

/**
 * @brief Point类型转换为Matx类型
 *
 * @tparam Tp 数据类型
 * @param[in] point Point类型变量
 * @return Matx类型变量
 */
template <typename Tp>
constexpr cv::Matx<Tp, 3, 1> point2matx(cv::Point3_<Tp> point) { return cv::Matx<Tp, 3, 1>(point.x, point.y, point.z); }

/**
 * @brief Matx类型转换为Point类型
 *
 * @tparam Tp 数据类型
 * @param[in] matx Matx类型变量
 * @return Point类型变量
 */
template <typename Tp>
constexpr cv::Point3_<Tp> matx2point(cv::Matx<Tp, 3, 1> matx) { return cv::Point3_<Tp>(matx(0), matx(1), matx(2)); }

/**
 * @brief Matx类型转换为Vec类型
 *
 * @tparam Tp 数据类型
 * @param[in] matx Matx类型变量
 * @return Vec类型变量
 */
template <typename Tp>
constexpr cv::Vec<Tp, 3> matx2vec(cv::Matx<Tp, 3, 1> matx) { return cv::Vec<Tp, 3>(matx(0), matx(1), matx(2)); }

// ------------------------【广义位移计算】------------------------

/**
 * @brief 获取距离
 *
 * @tparam Tp1 平面点 1 的数据类型
 * @tparam Tp2 平面点 2 的数据类型
 * @param[in] pt_1 起始点
 * @param[in] pt_2 终止点
 * @return 平面欧式距离
 */
template <typename Tp1, typename Tp2>
constexpr auto getDistance(const cv::Point_<Tp1> &pt_1, const cv::Point_<Tp2> &pt_2)
{
    return std::sqrt(std::pow(pt_1.x - pt_2.x, 2) + std::pow(pt_1.y - pt_2.y, 2));
}

/**
 * @brief 获取距离
 *
 * @tparam Tp1 平面向量 1 的数据类型
 * @tparam Tp2 平面向量 2 的数据类型
 * @param[in] vec_1 起始向量
 * @param[in] vec_2 终止向量
 * @return 平面欧式距离
 */
template <typename Tp1, typename Tp2>
constexpr auto getDistance(const cv::Vec<Tp1, 2> &vec_1, const cv::Vec<Tp2, 2> &vec_2)
{
    return std::sqrt(std::pow(vec_1(0) - vec_2(0), 2) + std::pow(vec_1(1) - vec_2(1), 2));
}

//! 计算所在平面
enum class CalPlane : uint8_t
{
    xyz = 0, //!< 三维空间
    xOy = 1, //!< xOy平面
    xOz = 2, //!< xOz平面
    yOz = 3  //!< yOz平面
};

/**
 * @brief 获取距离
 *
 * @tparam Tp1 空间点 1 的数据类型
 * @tparam Tp2 空间点 2 的数据类型
 * @param[in] pt_1 起始点
 * @param[in] pt_2 终止点
 * @param[in] calplane 要计算的距离所在的平面
 * @return 空间欧式距离
 */
template <typename Tp1, typename Tp2>
constexpr auto getDistance(const cv::Point3_<Tp1> &pt_1, const cv::Point3_<Tp2> &pt_2, CalPlane calplane = CalPlane::xyz)
{
    switch (calplane)
    {
    case CalPlane::xOy:
        return std::sqrt(std::pow(pt_1.x - pt_2.x, 2) + std::pow(pt_1.y - pt_2.y, 2));
    case CalPlane::xOz:
        return std::sqrt(std::pow(pt_1.x - pt_2.x, 2) + std::pow(pt_1.z - pt_2.z, 2));
    case CalPlane::yOz:
        return std::sqrt(std::pow(pt_1.y - pt_2.y, 2) + std::pow(pt_1.z - pt_2.z, 2));
    default:
        return std::sqrt(std::pow(pt_1.x - pt_2.x, 2) + std::pow(pt_1.y - pt_2.y, 2) + std::pow(pt_1.z - pt_2.z, 2));
    }
}

/**
 * @brief 获取距离
 *
 * @tparam Tp1 空间向量 1 的数据类型
 * @tparam Tp2 空间向量 2 的数据类型
 * @param[in] vec_1 起始向量
 * @param[in] vec_2 终止向量
 * @param[in] calplane 要计算的距离所在的平面
 * @return 空间欧式距离
 */
template <typename Tp1, typename Tp2>
constexpr auto getDistance(const cv::Vec<Tp1, 3> &vec_1, const cv::Vec<Tp2, 3> &vec_2, CalPlane calplane = CalPlane::xyz)
{
    return getDistance(cv::Point3_<Tp1>(vec_1), cv::Point3_<Tp2>(vec_2), calplane);
}

/**
 * @brief 点到直线距离
 * @note 点 \f$P=(x_0,y_0)\f$ 到直线 \f$l:Ax+By+C=0\f$ 距离公式为
 *       \f[D(P,l)=\frac{Ax_0+By_0+C}{\sqrt{A^2+B^2}}\f]
 *
 * @tparam Tp1 直线方程数据类型
 * @tparam Tp2 平面点的数据类型
 * @param[in] line 用 `cv::Vec4_` 表示的直线方程 (vx, vy, x0, y0)
 * @param[in] pt 平面点
 * @param[in] direc 是否区分距离的方向
 * @note
 * - 计算结果有正负，即区分点在直线分布的方向，通过修改传入参数
 *   `direc = false` 来设置计算结果不区分正负（统一返回 \f$|D(P,l)|\f$）
 * - `line` 需要传入元素为 \f$(v_x, v_y, x_0, y_0)\f$ 的用 `cv::Vec4_`
 *   表示的向量，指代下列直线方程（与 `cv::fitLine` 传入的 `cv::Vec4_`
 *   参数一致）\f[l:y-y_0=\frac{v_y}{v_x}(x-x_0)\f]
 * @return 平面欧式距离
 */
template <typename Tp1, typename Tp2>
constexpr auto getDistance(const cv::Vec<Tp1, 4> &line, const cv::Point_<Tp2> &pt, bool direc = true)
{
    auto retval = (line(1) * pt.x - line(0) * pt.y + line(0) * line(3) - line(1) * line(2)) /
                  std::sqrt(line(0) * line(0) + line(1) * line(1));
    return direc ? retval : std::abs(retval);
}

/**
 * @brief 获取与水平方向的夹角，以平面直角坐标系 x 轴为分界线，
 *        逆时针为正方向，范围: (-180°, 180°]，默认返回弧度制
 *
 * @tparam Tp1 平面点 1 的数据类型
 * @tparam Tp2 平面点 2 的数据类型
 * @param[in] start 像素坐标系下的起点
 * @param[in] end 像素坐标系下的终点
 * @param[in] mode 返回角度模式，默认弧度制
 * @return 返回角度
 */
template <typename Tp1, typename Tp2>
constexpr auto getHAngle(const cv::Point_<Tp1> &start, const cv::Point_<Tp2> &end, AngleMode mode = RAD)
{
    auto rad = -std::atan2((end.y - start.y), (end.x - start.x));
    return mode ? rad : rad2deg(rad);
}

/**
 * @brief 获取与垂直方向的夹角，以平面直角坐标系 y 轴为分界线，
 *        顺时针为正方向，范围: (-180°, 180°]，默认返回弧度制
 *
 * @tparam Tp1 平面点 1 的数据类型
 * @tparam Tp2 平面点 2 的数据类型
 * @param[in] start 起点
 * @param[in] end 终点
 * @param[in] mode 返回角度模式，默认弧度制
 * @return 返回角度
 */
template <typename Tp1, typename Tp2>
constexpr auto getVAngle(const cv::Point_<Tp1> &start, const cv::Point_<Tp2> &end, AngleMode mode = RAD)
{
    auto rad = std::atan2((end.x - start.x), (start.y - end.y));
    return mode ? rad : rad2deg(rad);
}

/**
 * @brief 求两个角之间的夹角
 *
 * @tparam Tp 角度的数据类型
 * @param[in] angle_1 第 1 个角度
 * @param[in] angle_2 第 2 个角度
 * @return 夹角，角度的范围是 (-180°, 180°]
 */
template <typename Tp>
constexpr Tp getDeltaAngle(Tp angle_1, Tp angle_2)
{
    // 角度范围统一化
    while (std::abs(angle_1) > 180)
        angle_1 -= (angle_1 > 0) ? 360 : -360;
    while (std::abs(angle_2) > 180)
        angle_2 -= (angle_2 > 0) ? 360 : -360;
    // 计算差值
    Tp delta_angle = angle_1 - angle_2;
    if (angle_1 > 150 && angle_2 < -150)
        delta_angle -= 360;
    else if (angle_1 < -150 && angle_2 > 150)
        delta_angle += 360;
    return std::abs(delta_angle);
}

// ------------------------【常用数学公式】------------------------
/**
 * @brief 正割 \f$\sec(x)\f$
 *
 * @tparam Tp 变量类型
 * @param[in] x 自变量
 * @return sec(x)
 */
template <typename Tp>
constexpr inline Tp sec(Tp x) { return 1 / cos(x); }

/**
 * @brief 余割 \f$\csc(x)\f$
 *
 * @tparam Tp 变量类型
 * @param[in] x 自变量
 * @return csc(x)
 */
template <typename Tp>
constexpr inline Tp csc(Tp x) { return 1 / sin(x); }

/**
 * @brief 余切 \f$\cot(x)\f$
 *
 * @tparam Tp 变量类型
 * @param[in] x 自变量
 * @return cot(x)
 */
template <typename Tp>
constexpr inline Tp cot(Tp x) { return 1 / tan(x); }

/**
 * @brief 符号函数
 *
 * @tparam Tp 变量类型
 * @param[in] x 自变量
 * @return \f[\text{sgn}(x)=\left\{\begin{matrix}
 *         1&&x>0\\0&&x=0\\-1&&x<0
 *         \end{matrix}\right.\f]
 */
template <typename Tp>
constexpr inline Tp sgn(Tp x) { return (x > 0) ? 1 : ((x < 0) ? -1 : 0); }

/**
 * @brief 计算 sigmoid(x) 在某一点的函数值
 * @note sigmoid 函数表达式为：\f[y=f_{sig}(x)=\frac{K_p}{1+e^{-kx+\mu}}\f]
 *
 * @tparam Tp 变量类型
 * @param[in] x 自变量 \f$x\f$
 * @param[in] k 缩放系数 \f$k\f$
 * @param[in] Kp 开环增益系数 \f$K_p\f$
 * @param[in] mu 偏移系数 \f$\mu\f$
 * @return 函数值 \f$f_{sig}(x)\f$
 */
template <typename Tp>
constexpr Tp sigmoid(Tp x, Tp k = 1, Tp Kp = 1, Tp mu = 0) { return Kp / (1 + std::pow(static_cast<Tp>(e), -k * x + mu)); }

/**
 * @brief 平面向量外积
 *
 * @tparam Tp 向量数据类型
 * @param[in] a 向量 A
 * @param[in] b 向量 B
 * @return 外积，若 retval = 0，则共线
 */
template <typename Tp>
constexpr Tp cross2D(const cv::Vec<Tp, 2> &a, const cv::Vec<Tp, 2> &b) { return a(0) * b(1) - a(1) * b(0); }

/**
 * @brief 平面向量外积
 *
 * @tparam Tp Point_数据类型
 * @param[in] a 向量 A
 * @param[in] b 向量 B
 * @return 外积，若 retval = 0，则共线
 */
template <typename Tp>
constexpr Tp cross2D(const cv::Point_<Tp> &a, const cv::Point_<Tp> &b) { return a.x * b.y - a.y * b.x; }

/**
 * @brief 在指定范围内寻找众数，时间复杂度 O(N)
 *
 * @tparam ForwardIterator 前向迭代器
 * @param[in] first 起始迭代器
 * @param[in] last 终止迭代器
 * @return 众数
 */
template <typename ForwardIterator>
typename ForwardIterator::value_type calculateModeNum(ForwardIterator first, ForwardIterator last)
{
    assert(first != last);
    using value_type = typename ForwardIterator::value_type;
    std::unordered_map<value_type, std::size_t, typename hash_traits<value_type>::hash_func> hash_map;
    for (ForwardIterator _it = first; _it != last; ++_it)
        ++hash_map[*_it];
    return std::max_element(hash_map.begin(), hash_map.end(), [](const auto &lhs, const auto &rhs) {
               return lhs.second < rhs.second;
           })
        ->first;
}

//! @} rmath

} // namespace rm
