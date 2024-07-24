/**
 * @file ew_topsis.hpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-01-12
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace rm
{

//! @addtogroup algorithm
//! @{

/**
 * @brief 熵权 TOPSIS 算法
 *
 * @tparam Tp 元素类型
 */
template <typename Tp>
class EwTopsis
{
public:
    typedef Tp value_type;
    typedef Tp &reference;
    typedef const Tp &const_reference;
    typedef std::size_t size_type;

private:
    typedef std::vector<Tp> idx_type;
    typedef std::vector<Tp> sample_type;
    typedef std::vector<std::vector<Tp>> mat_type;

    mat_type R_;   //!< 指标数据
    sample_type S; //!< 最终的指标数据

    const size_type _sample_size; //!< 行数，样本数
    const size_type _index_size;  //!< 列数，指标数

public:
    EwTopsis(const EwTopsis &) = delete;
    EwTopsis(EwTopsis &&) = delete;

    /**
     * @brief 构造熵权 TOPSIS 算法类
     *
     * @param[in] samples 样本指标
     */
    EwTopsis(const mat_type &samples) : R_(samples), _sample_size(samples.size()), _index_size(samples[0].size()) {}

    //! 基于权熵 TOPSIS 推理出最终的指标
    void inference()
    {
        mat_type R;
        calcR(R_, R);
        mat_type P;
        calcP(R, P);
        idx_type H;
        calcH(P, H);
        idx_type w;
        calcw(H, w);
        calcS(w, R_, S);
    }

    //! 获取样本的综合指标
    inline idx_type finalIndex() { return S; }

private:
    /**
     * @brief 获取标准化指标
     *
     * @param[in] _R 原始指标矩阵
     * @param[out] R 标准化指标矩阵
     */
    inline void calcR(const mat_type &_R, mat_type &R)
    {
        R = _R;
        idx_type min_indexs(_index_size);
        idx_type max_indexs(_index_size);
        for (size_type j = 0; j < _index_size; ++j)
        {
            min_indexs[j] = _R[0][j];
            max_indexs[j] = _R[0][j];
            for (size_type i = 1; i < _sample_size; ++i)
            {
                if (_R[i][j] < min_indexs[j])
                    min_indexs[j] = _R[i][j];
                if (_R[i][j] > max_indexs[j])
                    max_indexs[j] = _R[i][j];
            }
        }
        for (size_type i = 0; i < _sample_size; ++i)
            for (size_type j = 0; j < _index_size; ++j)
                R[i][j] = (R_[i][j] - min_indexs[j]) / (max_indexs[j] - min_indexs[j]);
    }

    /**
     * @brief 获取归一化指标
     *
     * @param[in] R 标准化指标矩阵
     * @param[out] P 归一化指标矩阵
     */
    inline void calcP(const mat_type &R, mat_type &P)
    {
        P = R;
        idx_type sums(_index_size);
        for (size_type j = 0; j < _index_size; ++j)
            sums[j] = std::accumulate(R.begin(), R.end(), 0,
                                      [&j](int a, const idx_type &b) {
                                          return a + b[j];
                                      });
        for (size_type i = 0; i < _sample_size; ++i)
            for (size_type j = 0; j < _index_size; ++j)
                P[i][j] = R[i][j] / sums[j];
    }

    /**
     * @brief 获取指标熵值向量
     *
     * @param[in] P 归一化指标矩阵
     * @param[out] H 指标熵值向量
     */
    inline void calcH(const mat_type &P, idx_type &H)
    {
        H.resize(_index_size);
        for (size_type j = 0; j < _index_size; ++j)
        {
            H[j] = 0;
            for (size_type i = 0; i < _sample_size; ++i)
                if (P[i][j] != 0)
                    H[j] -= P[i][j] * std::log(P[i][j]);
            H[j] /= std::log(_sample_size);
        }
    }

    /**
     * @brief 获取指标熵权向量
     *
     * @param[in] H 指标熵值向量
     * @param[out] w 指标熵权向量
     */
    inline void calcw(const idx_type &H, idx_type &w)
    {
        w.resize(_index_size);
        value_type tmp = _index_size - std::accumulate(H.begin(), H.end(), 0);
        for (size_type j = 0; j < _index_size; ++j)
            w[j] = (1 - H[j]) / tmp;
    }

    /**
     * @brief 获取样本综合指标
     * @todo Declaration shadows a fileld of EwTopsis<Tp> R_ and S
     * @param[in] w  指标熵权向量
     * @param[in] R_ 原始指标矩阵
     * @param[out] S 综合指标
     */
    inline void calcS(const idx_type &w, const mat_type &R_, sample_type &S)
    {
        S.resize(_sample_size);
        for (size_type i = 0; i < _sample_size; ++i)
        {
            S[i] = 0;
            for (size_type j = 0; j < _index_size; ++j)
                S[i] += w[j] * R_[i][j];
        }
    }
};

//! @} algorithm

} // namespace rm
