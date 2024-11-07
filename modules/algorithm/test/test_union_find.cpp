/**
 * @file test_union_find.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-01-11
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <gtest/gtest.h>

#define private public

#include "rmvl/algorithm/datastruct.hpp"

#undef private

using namespace rm;

namespace rm_test
{

TEST(UnionFindTest, int_merge_connected_findRep)
{
    std::vector arr = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    UnionFind<int> uf(arr.begin(), arr.end());
    uf.merge(1, 5);
    uf.merge(5, 6);
    uf.merge(2, 3);
    // {{1, 5, 6}, {4}, {2, 3}, {7}, {8}, {9}}
    EXPECT_TRUE(uf.connected(3, 2));
    EXPECT_TRUE(uf.connected(1, 6));
    EXPECT_EQ(uf.findRep(1), uf.findRep(6));
    EXPECT_FALSE(uf.connected(2, 6));
    EXPECT_FALSE(uf.connected(4, 7));
    // wrong number
    EXPECT_FALSE(uf.connected(4, 10));

    EXPECT_EQ(uf.components(), 6);
}

struct A
{
    int a;
    char b;
    float c;

    A(int _a, char _b, float _c) : a(_a), b(_b), c(_c) {}
};

using a_ptr = std::shared_ptr<A>;

TEST(UnionFindTest, shared_pointer_extract)
{
    std::vector<a_ptr> arr = {std::make_shared<A>(0, 'a', 1.1f),
                              std::make_shared<A>(1, 'b', 2.2f),
                              std::make_shared<A>(2, 'c', 3.3f),
                              std::make_shared<A>(3, 'd', 4.4f),
                              std::make_shared<A>(4, 'e', 5.5f),
                              std::make_shared<A>(5, 'f', 6.6f)};

    UnionFind<a_ptr> uf(arr.begin(), arr.end());
    uf.merge(arr[0], arr[1]);
    uf.merge(arr[1], arr[2]);
    uf.merge(arr[3], arr[4]);
    uf.merge(arr[4], arr[5]);

    EXPECT_TRUE(uf.connected(arr[0], arr[2]));
    EXPECT_FALSE(uf.connected(arr[0], arr[3]));
    EXPECT_EQ(uf.components(), 2);
    auto datas = uf.extract();
    // export
    auto it = datas.find((uf.findRep(arr[0])));
    EXPECT_NE(it, datas.end());
    EXPECT_EQ(it->second.size(), 3);
    auto vec = it->second;
    EXPECT_NE(find(vec.begin(), vec.end(), arr[0]), vec.end());
    EXPECT_NE(find(vec.begin(), vec.end(), arr[1]), vec.end());
    EXPECT_NE(find(vec.begin(), vec.end(), arr[2]), vec.end());
    EXPECT_EQ(find(vec.begin(), vec.end(), arr[3]), vec.end());
}

} // namespace rm_test
