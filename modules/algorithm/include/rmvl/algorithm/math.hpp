/**
 * @file math.hpp
 * @author RoboMaster Vision Community
 * @brief 基础数学库
 * @version 1.0
 * @date 2023-01-12
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <cmath>
#include <numeric>
#include <unordered_map>
#include <vector>

#if defined HAVE_OPENCV
#include <opencv2/core/matx.hpp>
#else
#include <algorithm>
#endif // HAVE_OPENCV

#include "rmvl/core/util.hpp"

namespace rm
{

//! @addtogroup algorithm
//! @{

// --------------------【结构、类型、常量定义】--------------------
#ifdef MAXFLOAT
constexpr double FLOAT_MAX{MAXFLOAT};
#elif defined HUGE
constexpr double FLOAT_MAX{HUGE};
#endif

constexpr double PI = 3.14159265358979323; //!< 圆周率: \f$\pi\f$
constexpr double e = 2.7182818459045;      //!< 自然对数底数: \f$e\f$
constexpr double SQRT_2 = 1.4142135623731; //!< 根号 2: \f$\sqrt2\f$

constexpr double PI_2 = PI / 2.; //!< PI / 2: \f$\frac\pi2\f$
constexpr double PI_4 = PI / 4.; //!< PI / 4: \f$\frac\pi4\f$

namespace numeric_literals
{

constexpr double operator""_PI(long double num) { return num * PI; }
constexpr double operator""_PI(long long unsigned num) { return num * PI; }

constexpr double operator""_to_rad(long double num) { return num * PI / 180.; }
constexpr double operator""_to_rad(long long unsigned num) { return num * PI / 180.; }

constexpr double operator""_to_deg(long double num) { return num * 180. / PI; }
constexpr double operator""_to_deg(long long unsigned num) { return num * 180. / PI; }

} // namespace numeric_literals

} // namespace rm

#ifdef HAVE_OPENCV

// 定义部分 Matx
namespace cv
{

using Matx11f = Matx<float, 1, 1>;
using Matx11d = Matx<double, 1, 1>;
using Matx51f = Matx<float, 5, 1>;
using Matx15f = Matx<float, 1, 5>;
using Matx51d = Matx<double, 5, 1>;
using Matx15d = Matx<double, 1, 5>;
using Matx55f = Matx<float, 5, 5>;
using Matx55d = Matx<double, 5, 5>;

} // namespace cv

#endif // HAVE_OPENCV

namespace rm
{

#ifdef HAVE_OPENCV

template <typename Tp>
constexpr Tp operator+(Tp val, const cv::Matx<Tp, 1, 1> &mat) { return val + mat(0, 0); }
template <typename Tp>
constexpr cv::Matx<Tp, 1, 1> operator+(const cv::Matx<Tp, 1, 1> &mat, Tp val) { return cv::Matx<Tp, 1, 1>(mat(0, 0) + val); }
template <typename Tp>
constexpr Tp operator-(Tp val, const cv::Matx<Tp, 1, 1> &mat) { return val - mat(0, 0); }
template <typename Tp>
constexpr cv::Matx<Tp, 1, 1> operator-(const cv::Matx<Tp, 1, 1> &mat, Tp val) { return cv::Matx<Tp, 1, 1>(mat(0, 0) - val); }

#endif // HAVE_OPENCV

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

// ------------------------【广义位移计算】------------------------

#ifdef HAVE_OPENCV

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

#endif // HAVE_OPENCV

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
constexpr Tp sec(Tp x) { return 1 / cos(x); }

/**
 * @brief 余割 \f$\csc(x)\f$
 *
 * @tparam Tp 变量类型
 * @param[in] x 自变量
 * @return csc(x)
 */
template <typename Tp>
constexpr Tp csc(Tp x) { return 1 / sin(x); }

/**
 * @brief 余切 \f$\cot(x)\f$
 *
 * @tparam Tp 变量类型
 * @param[in] x 自变量
 * @return cot(x)
 */
template <typename Tp>
constexpr Tp cot(Tp x) { return 1 / tan(x); }

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
constexpr Tp sgn(Tp x) { return (x > 0) ? 1 : ((x < 0) ? -1 : 0); }

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

#ifdef HAVE_OPENCV

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

#endif // HAVE_OPENCV

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

/**
 * @brief 使用 `std::vector` 表示的向量加法
 *
 * @tparam T 数据类型
 * @param[in] vec1 向量 1
 * @param[in] vec2 向量 2
 * @return 和向量
 */
template <typename T>
inline std::vector<T> operator+(const std::vector<T> &vec1, const std::vector<T> &vec2)
{
    std::vector<T> retval(vec1.size());
    std::transform(vec1.cbegin(), vec1.cend(), vec2.cbegin(), retval.begin(), std::plus<T>());
    return retval;
}

/**
 * @brief 使用 `std::vector` 表示的向量减法
 *
 * @tparam T 数据类型
 * @param[in] vec1 向量 1
 * @param[in] vec2 向量 2
 * @return 差向量
 */
template <typename T>
inline std::vector<T> operator-(const std::vector<T> &vec1, const std::vector<T> &vec2)
{
    std::vector<T> retval(vec1.size());
    std::transform(vec1.cbegin(), vec1.cend(), vec2.cbegin(), retval.begin(), std::minus<T>());
    return retval;
}

/**
 * @brief 使用 `std::vector` 表示的向量自加
 *
 * @tparam T 数据类型
 * @param[in] vec1 向量 1
 * @param[in] vec2 向量 2
 * @return 和向量
 */
template <typename T>
inline std::vector<T> &operator+=(std::vector<T> &vec1, const std::vector<T> &vec2)
{
    std::transform(vec1.cbegin(), vec1.cend(), vec2.cbegin(), vec1.begin(), std::plus<T>());
    return vec1;
}

/**
 * @brief 使用 `std::vector` 表示的向量自减
 *
 * @tparam T 数据类型
 * @param[in] vec1 向量 1
 * @param[in] vec2 向量 2
 * @return 差向量
 */
template <typename T>
inline std::vector<T> &operator-=(std::vector<T> &vec1, const std::vector<T> &vec2)
{
    std::transform(vec1.cbegin(), vec1.cend(), vec2.cbegin(), vec1.begin(), std::minus<T>());
    return vec1;
}

/**
 * @brief 使用 `std::vector` 表示的向量取反
 *
 * @tparam T 数据类型
 * @param[in] vec 向量
 * @return 向量取反后的结果
 */
template <typename T>
inline std::vector<T> operator-(const std::vector<T> &vec)
{
    std::vector<T> retval(vec.size());
    std::transform(vec.cbegin(), vec.cend(), retval.begin(), std::negate<T>());
    return retval;
}

/**
 * @brief 使用 `std::vector` 表示的向量乘法（数乘）
 *
 * @tparam T 数据类型
 * @param[in] vec 向量
 * @param[in] val 数乘因子
 * @return 向量乘法（数乘）
 */
template <typename T>
inline std::vector<T> operator*(const std::vector<T> &vec, T val)
{
    std::vector<T> retval(vec.size());
    std::transform(vec.cbegin(), vec.cend(), retval.begin(), [val](const T &x) { return x * val; });
    return retval;
}

/**
 * @brief 使用 `std::vector` 表示的向量乘法（数乘）
 *
 * @tparam T 数据类型
 * @param[in] val 数乘因子
 * @param[in] vec 向量
 * @return 向量乘法（数乘）
 */
template <typename T>
inline std::vector<T> operator*(T val, const std::vector<T> &vec) { return vec * val; }

/**
 * @brief 使用 `std::vector` 表示的向量乘法（数乘）
 *
 * @tparam T 数据类型
 * @param[in] vec 向量
 * @param[in] val 数乘因子
 * @return 向量乘法（数乘）
 */
template <typename T>
inline std::vector<T> &operator*=(std::vector<T> &vec, T val)
{
    std::transform(vec.cbegin(), vec.cend(), vec.begin(), [val](const T &x) { return x * val; });
    return vec;
}

/**
 * @brief 使用 `std::vector` 表示的向量除法（数乘）
 *
 * @tparam T 数据类型
 * @param[in] vec 向量
 * @param[in] val 除数
 * @return 向量除法（数乘）
 */
template <typename T>
inline std::vector<T> operator/(const std::vector<T> &vec, T val)
{
    std::vector<T> retval(vec.size());
    std::transform(vec.cbegin(), vec.cend(), retval.begin(), [val](const T &x) { return x / val; });
    return retval;
}

/**
 * @brief 使用 `std::vector` 表示的向量除法（数乘）
 *
 * @tparam T 数据类型
 * @param[in] val 除数
 * @param[in] vec 向量
 * @return 向量除法（数乘）
 */
template <typename T>
inline std::vector<T> &operator/=(std::vector<T> &vec, T val)
{
    std::transform(vec.cbegin(), vec.cend(), vec.begin(), [val](const T &x) { return x / val; });
    return vec;
}

// ------------------------【数学模型算法】------------------------

/**
 * @brief 熵权 TOPSIS 算法
 *
 * @tparam Tp 元素类型
 */
template <typename Tp>
class EwTopsis
{
    typedef std::vector<Tp> idx_type;
    typedef std::vector<Tp> SampleType;
    typedef std::vector<std::vector<Tp>> MatType;

public:
    typedef Tp value_type;
    typedef Tp &reference;
    typedef const Tp &const_reference;
    typedef std::size_t size_type;

    EwTopsis(const EwTopsis &) = delete;
    EwTopsis(EwTopsis &&) = delete;

    /**
     * @brief 构造熵权 TOPSIS 算法类
     *
     * @param[in] samples 样本指标
     */
    EwTopsis(const MatType &samples) : R_(samples), _sample_size(samples.size()), _index_size(samples[0].size()) {}

    //! 基于权熵 TOPSIS 推理出最终的指标
    void inference()
    {
        MatType R;
        calcR(R_, R);
        MatType P;
        calcP(R, P);
        idx_type H;
        calcH(P, H);
        idx_type w;
        calcw(H, w);
        calcS(w, R_, S);
    }

    //! 获取样本的综合指标
    inline idx_type finalIndex() { return S; }

private:
    /**
     * @brief 获取标准化指标
     *
     * @param[in] _R 原始指标矩阵
     * @param[out] R 标准化指标矩阵
     */
    inline void calcR(const MatType &_R, MatType &R)
    {
        R = _R;
        idx_type min_indexs(_index_size);
        idx_type max_indexs(_index_size);
        for (size_type j = 0; j < _index_size; ++j)
        {
            min_indexs[j] = _R[0][j];
            max_indexs[j] = _R[0][j];
            for (size_type i = 1; i < _sample_size; ++i)
            {
                if (_R[i][j] < min_indexs[j])
                    min_indexs[j] = _R[i][j];
                if (_R[i][j] > max_indexs[j])
                    max_indexs[j] = _R[i][j];
            }
        }
        for (size_type i = 0; i < _sample_size; ++i)
            for (size_type j = 0; j < _index_size; ++j)
                R[i][j] = (R_[i][j] - min_indexs[j]) / (max_indexs[j] - min_indexs[j]);
    }

    /**
     * @brief 获取归一化指标
     *
     * @param[in] R 标准化指标矩阵
     * @param[out] P 归一化指标矩阵
     */
    inline void calcP(const MatType &R, MatType &P)
    {
        P = R;
        idx_type sums(_index_size);
        for (size_type j = 0; j < _index_size; ++j)
            sums[j] = std::accumulate(R.begin(), R.end(), 0,
                                      [&j](int a, const idx_type &b) {
                                          return a + b[j];
                                      });
        for (size_type i = 0; i < _sample_size; ++i)
            for (size_type j = 0; j < _index_size; ++j)
                P[i][j] = R[i][j] / sums[j];
    }

    /**
     * @brief 获取指标熵值向量
     *
     * @param[in] P 归一化指标矩阵
     * @param[out] H 指标熵值向量
     */
    inline void calcH(const MatType &P, idx_type &H)
    {
        H.resize(_index_size);
        for (size_type j = 0; j < _index_size; ++j)
        {
            H[j] = 0;
            for (size_type i = 0; i < _sample_size; ++i)
                if (P[i][j] != 0)
                    H[j] -= P[i][j] * std::log(P[i][j]);
            H[j] /= std::log(_sample_size);
        }
    }

    /**
     * @brief 获取指标熵权向量
     *
     * @param[in] H 指标熵值向量
     * @param[out] w 指标熵权向量
     */
    inline void calcw(const idx_type &H, idx_type &w)
    {
        w.resize(_index_size);
        value_type tmp = _index_size - std::accumulate(H.begin(), H.end(), 0);
        for (size_type j = 0; j < _index_size; ++j)
            w[j] = (1 - H[j]) / tmp;
    }

    /**
     * @brief 获取样本综合指标
     * @todo Declaration shadows a fileld of EwTopsis<Tp> R_ and S
     * @param[in] w  指标熵权向量
     * @param[in] R_ 原始指标矩阵
     * @param[out] S_ 综合指标
     */
    inline void calcS(const idx_type &w, const MatType &R_, SampleType &S_)
    {
        S_.resize(_sample_size);
        for (size_type i = 0; i < _sample_size; ++i)
        {
            S_[i] = 0;
            for (size_type j = 0; j < _index_size; ++j)
                S_[i] += w[j] * R_[i][j];
        }
    }

    MatType R_;   //!< 指标数据
    SampleType S; //!< 最终的指标数据

    const size_type _sample_size; //!< 行数，样本数
    const size_type _index_size;  //!< 列数，指标数
};

//! @} algorithm

} // namespace rm
