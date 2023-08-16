/**
 * @file union_find.hpp
 * @author RoboMaster Vision Community
 * @brief 并查集
 * @version 1.0
 * @date 2023-01-11
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace rm
{

//! @addtogroup union_find
//! @{

/**
 * @brief 并查集
 *
 * @tparam Tp 元素类型
 */
template <typename Tp>
class UnionFind
{
public:
    typedef Tp value_type;
    typedef Tp *pointer;
    typedef Tp &reference;
    typedef const Tp *const_pointer;
    typedef const Tp &const_reference;
    typedef std::size_t size_type;

private:
    std::unordered_set<value_type> _element_set;            //!< 元素集合
    std::unordered_map<value_type, value_type> _parent_map; //!< 父元素哈希表
    std::unordered_map<value_type, size_type> _size_map;    //!< 集合大小
    int _connected_component = 0;                           //!< 连通分量

public:
    UnionFind(const UnionFind &) = delete;
    UnionFind(UnionFind &&) = delete;

    /**
     * @brief Construct a new Union Find Set object
     *
     * @tparam _InputIterator 迭代器类型
     * @param[in] first 起始迭代器
     * @param[in] last 终止迭代器
     */
    template <typename _InputIterator>
    UnionFind(_InputIterator first, _InputIterator last)
    {
        try
        {
            for (; first != last; ++first)
            {
                _element_set.insert(*first);
                _parent_map[*first] = *first;
                _size_map[*first] = 1;
                _connected_component++;
            }
        }
        catch (...)
        {
            _element_set.clear();
            throw;
        }
    }

    /**
     * @brief 两个元素是否在同一个集合
     *
     * @param[in] val_a 元素 A
     * @param[in] val_b 元素 B
     * @return 是否在同一个集合
     */
    inline bool isSameSet(const_reference val_a, const_reference val_b)
    {
        // Sign up?
        if (_element_set.find(val_a) == _element_set.end() ||
            _element_set.find(val_b) == _element_set.end())
            return false;
        // Common representation element?
        return findRep(val_a) == findRep(val_b);
    }

    /**
     * @brief 将两个元素所在集合合并
     *
     * @param[in] val_a 元素 A
     * @param[in] val_b 元素 B
     */
    inline void unionSet(const_reference val_a, const_reference val_b)
    {
        // Sign up?
        if (_element_set.find(val_a) == _element_set.end() ||
            _element_set.find(val_b) == _element_set.end())
            return;
        // Common representation element?
        auto a_rep = findRep(val_a);
        auto b_rep = findRep(val_b);
        if (a_rep == b_rep)
            return;
        // Mount samll set to large set.
        auto large_set = _size_map[a_rep] >= _size_map[b_rep] ? a_rep : b_rep;
        auto small_set = large_set == a_rep ? b_rep : a_rep;
        _parent_map[small_set] = large_set;
        _size_map[large_set] = _size_map[a_rep] + _size_map[b_rep];
        _size_map.erase(small_set);
        _connected_component--;
    }

    /**
     * @brief 导出数据
     *
     * @return Key: 集合代表元素，Value: 集合
     */
    inline std::unordered_map<value_type, std::vector<value_type>> exportData()
    {
        std::unordered_map<value_type, std::vector<value_type>> datas;
        for (const auto &map_pair : _size_map)
            datas[map_pair.first].reserve(map_pair.second);
        for (const auto &element : _element_set)
            datas[findRep(element)].emplace_back(element);
        return datas;
    }

    //! 获取连通分量
    inline int getConnectedComponent() { return _connected_component; }

private:
    /**
     * @brief 寻找代表元素
     *
     * @param[in] element 指定的元素
     * @return 集合的代表元素
     */
    value_type findRep(value_type element)
    {
        std::stack<value_type> path;
        while (element != _parent_map[element])
        {
            path.push(element);
            element = _parent_map[element];
        }
        while (!path.empty())
        {
            _parent_map[path.top()] = element;
            path.pop();
        }
        return element;
    }
};

//! @} union_find

} // namespace rm
