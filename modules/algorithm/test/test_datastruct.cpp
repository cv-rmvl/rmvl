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

#include <algorithm>
#include <deque>

#include <gtest/gtest.h>

#include "rmvl/algorithm/datastruct.hpp"

using namespace rm;

namespace rm_test {

TEST(Algorithm_data, raheap_basic_method_int) {
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

TEST(Algorithm_data, raheap_update_erase_export_int) {
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

class RHA {
    int _x;
    int _y;

public:
    RHA(int x, int y) : _x(x), _y(y) {}

    static RHA_ptr make(int x, int y) { return std::make_shared<RHA>(x, y); }

    inline int x() { return _x; }
    inline int y() { return _y; }
};

struct Comp {
    bool operator()(const RHA_ptr &a1, const RHA_ptr &a2) {
        return a1->x() * a1->x() + a1->y() * a1->y() <
               a2->x() * a2->x() + a2->y() * a2->y();
    }
};

TEST(Algorithm_data, raheap_custom_data) {
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

template <typename Tp>
struct UnionFindPublic : public UnionFind<Tp> {
    UnionFindPublic(const std::vector<Tp> &elements) : UnionFind<Tp>(elements.begin(), elements.end()) {}
    inline Tp findRepPublic(Tp element) { return this->findRep(element); }
};

TEST(Algorithm_data, unionfind_merge_connected_findRep) {
    std::vector arr = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    UnionFindPublic uf(arr);
    uf.merge(1, 5);
    uf.merge(5, 6);
    uf.merge(2, 3);
    // {{1, 5, 6}, {4}, {2, 3}, {7}, {8}, {9}}
    EXPECT_TRUE(uf.connected(3, 2));
    EXPECT_TRUE(uf.connected(1, 6));
    EXPECT_EQ(uf.findRepPublic(1), uf.findRepPublic(6));
    EXPECT_FALSE(uf.connected(2, 6));
    EXPECT_FALSE(uf.connected(4, 7));
    // wrong number
    EXPECT_FALSE(uf.connected(4, 10));

    EXPECT_EQ(uf.components(), 6);
}

struct A {
    int a;
    char b;
    float c;

    A(int _a, char _b, float _c) : a(_a), b(_b), c(_c) {}
};

using a_ptr = std::shared_ptr<A>;

TEST(Algorithm_data, unionfind_shared_pointer_extract) {
    std::vector<a_ptr> arr = {std::make_shared<A>(0, 'a', 1.1f),
                              std::make_shared<A>(1, 'b', 2.2f),
                              std::make_shared<A>(2, 'c', 3.3f),
                              std::make_shared<A>(3, 'd', 4.4f),
                              std::make_shared<A>(4, 'e', 5.5f),
                              std::make_shared<A>(5, 'f', 6.6f)};

    UnionFindPublic uf(arr);
    uf.merge(arr[0], arr[1]);
    uf.merge(arr[1], arr[2]);
    uf.merge(arr[3], arr[4]);
    uf.merge(arr[4], arr[5]);

    EXPECT_TRUE(uf.connected(arr[0], arr[2]));
    EXPECT_FALSE(uf.connected(arr[0], arr[3]));
    EXPECT_EQ(uf.components(), 2);
    auto datas = uf.extract();
    // export
    auto it = datas.find(uf.findRepPublic(arr[0]));
    EXPECT_NE(it, datas.end());
    EXPECT_EQ(it->second.size(), 3);
    auto vec = it->second;
    EXPECT_NE(std::find(vec.begin(), vec.end(), arr[0]), vec.end());
    EXPECT_NE(std::find(vec.begin(), vec.end(), arr[1]), vec.end());
    EXPECT_NE(std::find(vec.begin(), vec.end(), arr[2]), vec.end());
    EXPECT_EQ(std::find(vec.begin(), vec.end(), arr[3]), vec.end());
}

} // namespace rm_test
