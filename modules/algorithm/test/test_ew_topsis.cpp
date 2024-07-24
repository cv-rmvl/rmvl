/**
 * @file test_ew_topsis.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-01-13
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <gtest/gtest.h>

#include "rmvl/algorithm/ew_topsis.hpp"
#include "rmvl/algorithm/math.hpp"

using namespace rm;

namespace rm_test
{

TEST(EwTopsisTest, BasicFunction)
{
    std::vector<std::vector<float>> samples = {{1.f, 1.f}, {3.f, 3.f}, {2.f, 4.f}, {1.f, 5.f}};

    EwTopsis ew(samples);
    ew.inference();
    auto ret = ew.finalIndex();
    auto it = min_element(ret.begin(), ret.end());
    EXPECT_EQ(it, ret.begin());
}

class EwA;
using Ew_ptr = std::shared_ptr<EwA>;

struct MyPoint2f
{
    float x{};
    float y{};
    MyPoint2f(float x_, float y_) : x(x_), y(y_) {}
};

class EwA
{
    MyPoint2f center;
    float angle;

public:
    EwA(MyPoint2f c, float a) : center(c), angle(a) {}

    inline MyPoint2f getCenter() { return center; }
    inline float getAngle() { return angle; }

    static Ew_ptr make(float x, float y, float a) { return std::make_shared<EwA>(MyPoint2f(x, y), a); }
};

static inline float dis(const MyPoint2f &a, const MyPoint2f &b)
{
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

TEST(EwTopsisTest, CustomData)
{
    // 第一帧有四个 EwA，各自属于一个 standard 序列
    auto standard0 = EwA::make(-10, 5, 6);
    auto standard1 = EwA::make(10, 5, -6);
    auto standard2 = EwA::make(10, -4, 4);
    auto standard3 = EwA::make(-10, -4, -4);
    std::vector<Ew_ptr> standards = {standard0,
                                     standard1,
                                     standard2,
                                     standard3};

    // 第二帧有 2 个新的 EwA
    auto current0 = EwA::make(9, 4.5f, -5);
    auto current1 = EwA::make(-8, -4.2f, -3.5f);
    std::vector<Ew_ptr> currents = {current0,
                                    current1};

    // 问: current 各自属于哪一个 standard 序列?
    // (a) 生成样本指标矩阵，（指标：距离，角度差）
    std::vector<std::vector<float>> samples(standards.size() * currents.size());
    for (size_t c = 0; c < currents.size(); ++c)
    {
        for (size_t s = 0; s < standards.size(); ++s)
        {
            // 2 表示两个指标
            samples[c * standards.size() + s].resize(2);
            samples[c * standards.size() + s][0] = -dis(currents[c]->getCenter(), standards[s]->getCenter());
            samples[c * standards.size() + s][1] = -abs(currents[c]->getAngle() - standards[s]->getAngle());
        }
    }
    // (b) 运用熵权法推理
    EwTopsis<float> ew(samples);
    ew.inference();
    auto arr = ew.finalIndex();
    // (c) 数据导出并提取出指定的下标
    std::unordered_map<size_t, size_t> target;
    for (size_t i = 0; i < currents.size(); ++i)
        // 每个 current 都在 standards 找到熵权最小的一个作为目标
        target[i] = (max_element(arr.begin() + standards.size() * i, arr.begin() + standards.size() * (i + 1)) -
                     arr.begin()) %
                    standards.size();

    EXPECT_EQ(target[0], 1);
    EXPECT_EQ(target[1], 3);
}

} // namespace rm_test
