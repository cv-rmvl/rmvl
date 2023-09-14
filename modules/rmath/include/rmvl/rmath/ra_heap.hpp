/**
 * @file ra_heap.hpp
 * @author RoboMaster Vision Community
 * @brief 支持随机访问的堆
 * @version 1.0
 * @date 2023-01-12
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace rm
{

//! @addtogroup ra_heap
//! @{

/**
 * @brief 支持随机访问的堆
 *
 * @tparam Tp 元素类型
 * @tparam Sequence 容器类型，默认为 std::vector<Tp>
 * @tparam Compare 比较器可调用对象，默认为 std::less<Tp>，即默认为大根堆
 */
template <typename Tp, typename Sequence = std::vector<Tp>, typename Compare = std::less<Tp>>
class RaHeap
{
#if __cplusplus >= 201703L
    static_assert(std::is_same_v<Tp, typename Sequence::value_type>,
                  "value_type must be the same as the underlying container");
#endif // C++17

public:
    typedef typename Sequence::value_type value_type;
    typedef typename Sequence::reference reference;
    typedef typename Sequence::const_reference const_reference;
    typedef typename Sequence::size_type size_type;
    typedef Sequence container_type;
    typedef Compare value_compare;

private:
    Sequence _c;   //!< 元素数组
    Compare _comp; //!< 可调用对象

    std::unordered_map<value_type, size_type> _indexs; //!< 下标哈希表（存放数组的下标）

public:
    RaHeap() = default;

    /**
     * @brief 在堆顶添加元素
     *
     * @param x 待添加的元素
     */
    void push(const_reference x)
    {
        _c.push_back(x);
        _indexs[x] = _c.size() - 1;
        upHeapify(_c.size() - 1);
    }

    /**
     * @brief 在堆顶添加元素
     *
     * @param x 待添加的元素
     */
    void push(value_type &&x)
    {
        _c.push_back(std::move(x));
        _indexs[x] = _c.size() - 1;
        upHeapify(_c.size() - 1);
    }

    /**
     * @brief 在堆顶添加元素
     *
     * @param x 待添加的元素
     */
    template <typename ValueType>
    void emplace(ValueType &&x)
    {
        _c.emplace_back(std::forward<ValueType>(x));
        _indexs[x] = _c.size() - 1;
        upHeapify(_c.size() - 1);
    }

    /**
     * @brief 更新元素
     *
     * @param prev 之前的元素
     * @param value 改动后的元素
     */
    void update(const_reference prev, const_reference value)
    {
        if (_indexs.find(prev) == _indexs.end())
            return;
        size_type idx = _indexs[prev];
        _indexs[value] = idx;
        _indexs.erase(prev);
        _c[idx] = value;
        if (_comp(prev, value))
            upHeapify(idx);
        else
            downHeapify(idx);
    }

    /**
     * @brief 删除指定元素
     *
     * @param value 待删除的元素
     */
    void erase(const_reference value)
    {
        if (_indexs.find(value) == _indexs.end())
            return;
        size_type idx = _indexs[value];
        swapNode(idx, _c.size() - 1);
        _indexs.erase(_c.back());
        _c.pop_back();
        downHeapify(idx);
    }

    /**
     * @brief 弹出堆顶
     */
    void pop()
    {
        swapNode(0, _c.size() - 1);
        _indexs.erase(_c.back());
        _c.pop_back();
        downHeapify(0);
    }

    //! 堆是否为空
    inline bool empty() { return size() == 0; }
    //! 堆的大小
    inline size_type size() { return _c.size(); }
    //! 获取堆顶元素
    inline const_reference top() const { return _c.front(); }
    //! 导出容器
    inline const container_type &c() const { return _c; }

private:
    /**
     * @brief 交换 _c 数组中指定的两个下标的元素
     *
     * @param idx1 下标 1
     * @param idx2 小标 2
     */
    void swapNode(size_t idx1, size_t idx2)
    {
        _indexs[_c[idx1]] = idx2;
        _indexs[_c[idx2]] = idx1;
        std::swap(_c[idx1], _c[idx2]);
    }

    /**
     * @brief 从给定的节点开始往上生成堆
     *
     * @param idx 节点下标
     */
    void upHeapify(size_type idx)
    {
        while (idx != 0 && _comp(_c[(idx - 1) >> 1], _c[idx]))
        {
            swapNode((idx - 1) >> 1, idx);
            idx = (idx - 1) >> 1;
        }
    }

    /**
     * @brief 从给定的节点开始往下生成堆
     *
     * @param idx 节点下标
     */
    void downHeapify(size_type idx)
    {
        size_type left = (idx << 1) + 1;
        while (left < _c.size())
        {
            // 左节点与右节点比较出满足条件的
            size_type better = left + 1 < _c.size() && _comp(_c[left], _c[left + 1]) ? left + 1 : left;
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

//! @} ra_heap

} // namespace rm
