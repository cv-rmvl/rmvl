/**
 * @file datastruct.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 数据结构
 * @version 1.0
 * @date 2024-04-27
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace rm
{

//! @addtogroup algorithm
//! @{
//! @defgroup algorithm_datastruct 数据结构
//! @{
//! @brief 包含自定义的容器适配器以及其余数据结构
//! @} algorithm_datastruct
//! @} algorithm

//! @addtogroup algorithm_datastruct
//! @{

/**
 * @brief 支持随机访问的堆（与 `std::priority_queue` 具备相同的基础容器筛选条件）
 *
 * @tparam Tp 元素类型
 * @tparam Sequence 容器类型，默认为 `std::vector<Tp>`
 * @tparam Compare 比较器可调用对象，默认为 `std::less<Tp>`，即默认为大根堆
 */
template <typename Tp, typename Sequence = std::vector<Tp>, typename Compare = std::less<Tp>>
class RaHeap
{
    static_assert(std::is_same_v<Tp, typename Sequence::value_type>,
                  "value_type must be the same as the underlying container");

public:
    typedef Tp value_type;
    typedef Tp &reference;
    typedef const Tp &const_reference;
    typedef std::size_t size_type;
    typedef Sequence container_type;
    typedef Compare value_compare;

private:
    Sequence _c;   //!< 元素数组
    Compare _comp; //!< 可调用对象

    std::unordered_map<Tp, std::size_t> _indexs; //!< 下标哈希表（存放数组的下标）

public:
    RaHeap() = default;

    /**
     * @brief 在堆顶添加元素
     *
     * @param[in] x 待添加的元素
     */
    inline void push(const Tp &x)
    {
        _c.push_back(x);
        _indexs[x] = _c.size() - 1;
        upHeapify(_c.size() - 1);
    }

    /**
     * @brief 在堆顶添加元素
     *
     * @param[in] x 待添加的元素
     */
    inline void push(Tp &&x)
    {
        _c.push_back(std::move(x));
        _indexs[x] = _c.size() - 1;
        upHeapify(_c.size() - 1);
    }

    /**
     * @brief 在堆顶添加元素
     *
     * @param[in] x 待添加的元素
     */
    template <typename ValueType>
    inline void emplace(ValueType &&x)
    {
        _c.emplace_back(std::forward<ValueType>(x));
        _indexs[x] = _c.size() - 1;
        upHeapify(_c.size() - 1);
    }

    /**
     * @brief 更新元素
     *
     * @param[in] prev 之前的元素
     * @param[in] value 改动后的元素
     */
    inline void update(const Tp &prev, const Tp &value)
    {
        if (_indexs.find(prev) == _indexs.end())
            return;
        std::size_t idx = _indexs[prev];
        _indexs[value] = idx;
        _indexs.erase(prev);
        _c[idx] = value;
        (_comp(prev, value)) ? upHeapify(idx) : downHeapify(idx);
    }

    /**
     * @brief 删除指定元素
     *
     * @param[in] value 待删除的元素
     */
    inline void erase(const Tp &value)
    {
        if (_indexs.find(value) == _indexs.end())
            return;
        std::size_t idx = _indexs[value];
        swapNode(idx, _c.size() - 1);
        _indexs.erase(_c.back());
        _c.pop_back();
        downHeapify(idx);
    }

    //! 弹出堆顶
    inline void pop()
    {
        swapNode(0, _c.size() - 1);
        _indexs.erase(_c.back());
        _c.pop_back();
        downHeapify(0);
    }

    //! 堆是否为空
    inline bool empty() { return _c.empty(); }
    //! 堆的大小
    inline std::size_t size() { return _c.size(); }
    //! 获取堆顶元素
    inline const Tp &top() const { return _c.front(); }
    //! 导出容器
    inline const Sequence &c() const { return _c; }

private:
    //! 交换 `_c` 数组中指定的两个下标的元素
    void swapNode(std::size_t idx1, std::size_t idx2)
    {
        _indexs[_c[idx1]] = idx2;
        _indexs[_c[idx2]] = idx1;
        std::swap(_c[idx1], _c[idx2]);
    }

    //! 从给定的节点开始往上生成堆
    void upHeapify(std::size_t idx)
    {
        while (idx != 0 && _comp(_c[(idx - 1) >> 1], _c[idx]))
        {
            swapNode((idx - 1) >> 1, idx);
            idx = (idx - 1) >> 1;
        }
    }

    //! 从给定的节点开始往下生成堆
    void downHeapify(std::size_t idx)
    {
        std::size_t left = (idx << 1) + 1;
        while (left < _c.size())
        {
            // 左节点与右节点比较出满足条件的
            auto better = left + 1 < _c.size() && _comp(_c[left], _c[left + 1]) ? left + 1 : left;
            // 满足条件的子节点与自身比较
            better = _comp(_c[better], _c[idx]) ? idx : better;
            if (better == idx)
                break;
            swapNode(idx, better);
            // update
            idx = better;
            left = (idx << 1) + 1;
        }
    }
};

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
    typedef Tp &reference;
    typedef const Tp &const_reference;
    typedef std::size_t size_type;

private:
    std::unordered_set<Tp> _element_set;           //!< 元素集合
    std::unordered_map<Tp, Tp> _parent_map;        //!< 父元素哈希表
    std::unordered_map<Tp, std::size_t> _size_map; //!< 集合大小
    int _connected_component{};                    //!< 连通分量

public:
    /**
     * @brief 构造并查集
     *
     * @tparam InputIterator 老式前向迭代器类型
     * @param[in] first 起始迭代器
     * @param[in] last 终止迭代器
     */
    template <typename InputIterator>
    UnionFind(InputIterator first, InputIterator last)
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
    inline bool isSameSet(const Tp &val_a, const Tp &val_b)
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
    inline void unionSet(const Tp &val_a, const Tp &val_b)
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
    inline std::unordered_map<Tp, std::vector<Tp>> exportData()
    {
        std::unordered_map<Tp, std::vector<Tp>> datas;
        for (const auto &map_pair : _size_map)
            datas[map_pair.first].reserve(map_pair.second);
        for (const auto &element : _element_set)
            datas[findRep(element)].emplace_back(element);
        return datas;
    }

    //! 获取连通分量
    inline int getConnectedComponent() { return _connected_component; }

private:
    //! 寻找代表元素
    Tp findRep(Tp element)
    {
        std::stack<Tp> path;
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

//! @} algorithm_datastruct

} // namespace rm
