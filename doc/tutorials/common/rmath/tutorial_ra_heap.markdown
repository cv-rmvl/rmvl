支持随机访问的堆 {#tutorial_common_ra_heap}
============

@author RoboMaster Vision Community
@date 2023/01/11

@prev_tutorial{tutorial_common_union_find}

@next_tutorial{tutorial_common_ew_topsis}

@tableofcontents

------

相关类 rm::RaHeap

## 1. 基本用法

rm::RaHeap 的基本用法与标准库中 `std::priority_queue` 的用法基本一致，都具有以下操作：

| 成员方法 |      含义      |
| :------: | :------------: |
| empty()  | 判断堆是否为空 |
|  size()  |  获取堆的大小  |
|  top()   |  获取堆顶元素  |
|  pop()   |  弹出堆顶元素  |
|  push()  |  压入堆顶元素  |

可参考以下文档：

- <a href="https://zh.cppreference.com/w/cpp/container/priority_queue" target="_blank">
      C++ 标准库——优先队列
  </a>

## 2. 扩展用法

除此之外，RaHeap 还提供了以下扩展用法：

| 成员方法 |                  含义                  |
| :------: | :------------------------------------: |
| update() | 更新某个元素为新值（不存在则直接返回） |
| erase()  |  删除某个指定元素（不存在则直接返回）  |
|   c()    |            返回堆的底层容器            |

## 3. 例题

**Dijkstra + 堆优化**

使用 RaHeap：

```cpp
using Pair = pair<Node *, int>;

struct Comp
{
    bool operator()(const Pair &p1, const Pair &p2)
    {
        return p1.second > p2.second;
    }
};

unordered_map<Node *, int> dijkstra(Node *head)
{
    RaHeap<Pair, vector<Pair>, Comp> node_heap;
    node_heap.push({head, 0});
    unordered_map<Node *, int> distanceMap;
    distanceMap[head] = 0;

    while (!node_heap.empty())
    {
        auto &[node, distance] = node_heap.top();
        node_heap.pop();
        for (auto edg : node->edges)
        {
            if (distanceMap.find(edg->to) == distanceMap.end())
            {
                node_heap.push({edg->to, edg.weight + distance});
                distanceMap[edg->to] = edg.weight + distance;
            }
            else if (edg.weight + distance < distanceMap[edg->to])
            {
                node_heap.update({edg->to, distanceMap[edg->to]},
                                 {edg->to, edg.weight + distance});
                distanceMap[edg->to] = edg.weight + distance;
            }
        }
    }
    return distanceMap;
}
```
