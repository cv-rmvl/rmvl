/**
 * @file mathmodel_impl.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 数学模型算法实现
 * @version 1.0
 * @date 2024-09-07
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include "rmvl/algorithm/math.hpp"

namespace rm
{

class EwTopsis::Impl
{
    typedef std::vector<double> IndexType;
    typedef std::vector<double> SampleType;
    typedef std::vector<std::vector<double>> MatType;

public:
    /**
     * @brief 构造熵权 TOPSIS 算法类
     *
     * @param[in] samples 样本指标
     */
    Impl(const MatType &samples) : R_(samples), _sample_size(samples.size()), _index_size(samples[0].size()) {}

    //! 基于权熵 TOPSIS 推理出最终的指标
    IndexType inference();

private:
    /**
     * @brief 获取标准化指标
     *
     * @param[in] _R 原始指标矩阵
     * @param[out] R 标准化指标矩阵
     */
    void calcR(const MatType &_R, MatType &R);

    /**
     * @brief 获取归一化指标
     *
     * @param[in] R 标准化指标矩阵
     * @param[out] P 归一化指标矩阵
     */
    void calcP(const MatType &R, MatType &P);

    /**
     * @brief 获取指标熵值向量
     *
     * @param[in] P 归一化指标矩阵
     * @param[out] H 指标熵值向量
     */
    void calcH(const MatType &P, IndexType &H);

    /**
     * @brief 获取指标熵权向量
     *
     * @param[in] H 指标熵值向量
     * @param[out] w 指标熵权向量
     */
    void calcw(const IndexType &H, IndexType &w);

    /**
     * @brief 获取样本综合指标
     * @todo Declaration shadows a fileld of EwTopsis<Tp> R_ and S
     * @param[in] w  指标熵权向量
     * @param[in] R_ 原始指标矩阵
     * @param[out] S_ 综合指标
     */
    void calcS(const IndexType &w, const MatType &R_, SampleType &S_);

    MatType R_;   //!< 指标数据
    SampleType S; //!< 最终的指标数据

    const std::size_t _sample_size; //!< 行数，样本数
    const std::size_t _index_size;  //!< 列数，指标数
};

class Munkres::Impl
{
public:
    //! 创建 KM 算法求解器
    Impl(const std::vector<std::vector<double>> &cost_matrix);

    //! 求解
    std::vector<std::size_t> solve() noexcept;

private:
    enum class Step
    {
        Step1, //!< 步骤 1，对每行减去最小值
        Step2, //!< 步骤 2，标记 `0` 元素的行和列
        Step3, //!< 步骤 3，覆盖所有标记了 `0` 元素的列
        Step4, //!< 步骤 4，找出未覆盖的 `0` 元素
        Step5, //!< 步骤 5，构造增广路径
        Step6, //!< 步骤 6，调整代价矩阵，以出现新的 `0` 元素
        End,   //!< 结束，后处理
    };

    /**
     * @brief 查找矩阵中的未覆盖 `0` 元素
     *
     * @param[in] row_covered 行是否覆盖
     * @param[in] col_covered 列是否覆盖
     * @return 第一个找到的未覆盖 `0` 元素的坐标，不存在返回 `{-1, -1}`
     */
    std::tuple<int, int> findOneZero(const std::vector<bool> &row_covered, const std::vector<bool> &col_covered);

    /**
     * @brief 判断指定行是否有 `*`
     *
     * @param[in] row 指定行
     * @return 是否有 `*`
     */
    bool isStarInRow(int row);

    /**
     * @brief 查找指定行的 `*` 所在的列
     *
     * @param[in] row 指定行
     * @return `*` 所在列，不存在返回 `-1`
     */
    int findStarInRow(int row);

    /**
     * @brief 查找指定列的 `*` 所在的行
     *
     * @param[in] col 指定列
     * @return `*` 所在行，不存在返回 `-1`
     */
    int findStarInCol(int col);

    /**
     * @brief 查找指定行的 `'` 所在的列
     *
     * @param[in] row 指定行
     * @return `'` 所在列，不存在返回 `-1`
     */
    int findPrimeInRow(int row);

    void step1();
    void step2();
    void step3();
    void step4();
    void step5();
    void step6();

    void clearCovers();
    void erasePrimes();

    const std::size_t _m{};
    const std::size_t _n{};

    std::vector<std::vector<double>> _matrix{};
    std::vector<std::vector<int>> _mask{};
    std::vector<bool> _row_covered{};
    std::vector<bool> _col_covered{};
    Step _step{Step::Step1};
    std::vector<std::pair<int, int>> _path{};
    std::pair<int, int> _first_zero{};
};

} // namespace rm
