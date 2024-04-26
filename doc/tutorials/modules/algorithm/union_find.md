并查集 {#tutorial_modules_union_find}
============

@author RoboMaster Vision Community
@date 2023/01/11

@prev_tutorial{tutorial_modules_fft}

@next_tutorial{tutorial_modules_ra_heap}

@tableofcontents

------

相关类 rm::UnionFind

## 1. 什么是并查集

并查集是一种树型的数据结构，用于处理一些不相交集合的 **合并** 及 **查询** 问题，常常在使用中用森林来表示。

在一些有 `N` 个元素的集合应用问题中，我们通常是在开始时让每个元素构成一个单元素的集合，然后按一定顺序将属于同一组的元素所在的集合合并，其间要反复查找一个元素在哪个集合中。

+ 如果使用链表代替，查找的方法时间复杂度达到了 \f$\text O(N)\f$，合并的方法时间复杂度为 \f$\text O(1)\f$。
+ 如果使用哈希表代替，查找的方法时间复杂度为 \f$\text O(1)\f$，合并的方法时间复杂度达到了 \f$\text O(N)\f$。

使用森林来表示的并查集，查询、合并操作时间复杂度均接近 \f$\text O(1)\f$。

## 2. 使用方法

rm::UnionFind 支持集合的快速合并、查找的结构，提供三种典型操作

|              成员方法             |           含义           |
| :-------------------------------: | :----------------------: |
|       bool inSameSet(a, b);       | 两个元素是否在一个集合中 |
|        void unionSet(a, b);       | 两个元素所在集合进行合并 |
| void getConnectedComponent(a, b); |   获取并查集的连通分量   |

## 3. 例题

**【LeetCode 200】岛屿数量**

**问题：**

给你一个由 `'1'`（陆地）和 `'0'`（水）组成的的二维网格，请你计算网格中岛屿的数量。

岛屿总是被水包围，并且每座岛屿只能由水平方向和/或竖直方向上相邻的陆地连接形成。

此外，你可以假设该网格的四条边均被水包围。 

**示例 1：**

```
输入：grid = [
  ['1','1','1','1','0'],
  ['1','1','0','1','0'],
  ['1','1','0','0','0'],
  ['0','0','0','0','0']
]
输出：1
```

**示例 2：**

```
输入：grid = [
  ['1','1','0','0','0'],
  ['1','1','0','0','0'],
  ['0','0','1','0','0'],
  ['0','0','0','1','1']
]
输出：3
```

**提示：**

- `m == grid.length`
- `n == grid[i].length`
- `1 <= m, n <= 300`
- `grid[i][j]` 的值为 `'0'` 或 `'1'`

**代码：**

使用 UnionFind 进行解答：

```cpp
class Solution
{
public:
    int numIslands(vector<vector<char>>& grid)
    {
        const int rows = grid.size();
        if (!rows)
            return 0;
        const int cols = grid[0].size();

        vector<char> data;
        data.reserve(rows * cols);
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                if (grid[i][j] == '1')
                    data.emplace_back(i * cols + j);
        UnionFind<char> uf(data.begin(), data.end());

        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
                if (grid[r][c] == '1')
                {
                    grid[r][c] = '0';
                    if (r - 1 >= 0 && grid[r - 1][c] == '1')
                        uf.unionSet(r * cols + c, (r - 1) * cols + c);
                    if (r + 1 < rows && grid[r + 1][c] == '1')
                        uf.unionSet(r * cols + c, (r + 1) * cols + c);
                    if (c - 1 >= 0 && grid[r][c - 1] == '1')
                        uf.unionSet(r * cols + c, r * cols + c - 1);
                    if (c + 1 < cols && grid[r][c + 1] == '1')
                        uf.unionSet(r * cols + c, r * cols + c + 1);
                }
        return uf.getConnectedComponent();
    }
};
```
