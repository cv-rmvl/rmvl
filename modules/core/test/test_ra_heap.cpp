/**
 * @file test_ra_heap.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-01-13
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <deque>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "rmvl/core/datastruct.hpp"

using namespace rm;

namespace rm_test
{

TEST(RaHeapTest, BasicMethod_int)
{
    // 大根堆
    RaHeap<int> heap;
    heap.push(3);
    heap.push(1);
    heap.push(2);
    EXPECT_EQ(heap.top(), 3);
    heap.pop();
    EXPECT_EQ(heap.top(), 2);
    heap.pop();
    EXPECT_EQ(heap.top(), 1);
    heap.pop();
    EXPECT_TRUE(heap.empty());
}

TEST(RaHeapTest, UpdateEraseExport_int)
{
    // 小根堆
    RaHeap<int, std::deque<int>, std::greater<int>> heap;
    heap.push(3);
    heap.push(2);
    heap.push(4);
    EXPECT_EQ(heap.top(), 2);
    heap.update(3, 1);
    EXPECT_EQ(heap.top(), 1);
    const auto &vec = heap.extract();
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec.front(), 1);
    heap.erase(2);
    EXPECT_EQ(vec.size(), 2);
}

class RHA;
using RHA_ptr = std::shared_ptr<RHA>;

class RHA
{
    int _x;
    int _y;

public:
    RHA(int x, int y) : _x(x), _y(y) {}

    static RHA_ptr make(int x, int y) { return std::make_shared<RHA>(x, y); }

    inline int x() { return _x; }
    inline int y() { return _y; }
};

struct Comp
{
    bool operator()(const RHA_ptr &a1, const RHA_ptr &a2)
    {
        return a1->x() * a1->x() + a1->y() * a1->y() <
               a2->x() * a2->x() + a2->y() * a2->y();
    }
};

TEST(RaHeapTest, CustomData)
{
    std::vector<RHA_ptr> arr;
    arr.emplace_back(RHA::make(1, 2));  // arr[0] = 5
    arr.emplace_back(RHA::make(3, 4));  // arr[1] = 25
    arr.emplace_back(RHA::make(-2, 3)); // arr[2] = 13
    arr.emplace_back(RHA::make(5, 4));  // arr[3] = 41
    arr.emplace_back(RHA::make(6, 1));  // arr[4] = 37

    RaHeap<RHA_ptr, std::vector<RHA_ptr>, Comp> heap;
    for (auto ele : arr)
        heap.push(ele);
    EXPECT_EQ(heap.top(), arr[3]);
}

} // namespace rm_test
