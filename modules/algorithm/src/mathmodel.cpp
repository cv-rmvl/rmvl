/**
 * @file mathmodel.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2024-09-07
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <algorithm>
#include <limits>
#include <stdexcept>

#include "mathmodel_impl.hpp"

namespace rm
{

////////////////////////////////// EwTopsis //////////////////////////////////

EwTopsis::EwTopsis(const std::vector<std::vector<double>> &samples) : _impl(new Impl(samples)) {}
EwTopsis::~EwTopsis() = default;
std::vector<double> EwTopsis::inference() { return _impl->inference(); }

std::vector<double> EwTopsis::Impl::inference()
{
    std::vector<std::vector<double>> R{};
    calcR(_R, R);
    std::vector<std::vector<double>> P{};
    calcP(R, P);
    std::vector<double> H;
    calcH(P, H);
    std::vector<double> w;
    calcw(H, w);
    calcS(w, _R, _S);
    return _S;
}

void EwTopsis::Impl::calcR(const std::vector<std::vector<double>> &R, std::vector<std::vector<double>> &Rstd)
{
    Rstd = R;
    std::vector<double> min_indexs(_index_size);
    std::vector<double> max_indexs(_index_size);
    for (std::size_t j = 0; j < _index_size; ++j)
    {
        min_indexs[j] = _R[0][j];
        max_indexs[j] = _R[0][j];
        for (std::size_t i = 1; i < _sample_size; ++i)
        {
            if (_R[i][j] < min_indexs[j])
                min_indexs[j] = _R[i][j];
            if (_R[i][j] > max_indexs[j])
                max_indexs[j] = _R[i][j];
        }
    }
    for (std::size_t i = 0; i < _sample_size; ++i)
        for (std::size_t j = 0; j < _index_size; ++j)
            Rstd[i][j] = (_R[i][j] - min_indexs[j]) / (max_indexs[j] - min_indexs[j]);
}

void EwTopsis::Impl::calcP(const std::vector<std::vector<double>> &R, std::vector<std::vector<double>> &P)
{
    P = R;
    std::vector<double> sums(_index_size);
    for (std::size_t j = 0; j < _index_size; ++j)
        sums[j] = std::accumulate(R.begin(), R.end(), 0.0, [&j](double a, const std::vector<double> &b) {
            return a + b[j];
        });
    for (std::size_t i = 0; i < _sample_size; ++i)
        for (std::size_t j = 0; j < _index_size; ++j)
            P[i][j] = R[i][j] / sums[j];
}

void EwTopsis::Impl::calcH(const std::vector<std::vector<double>> &P, std::vector<double> &H)
{
    H.resize(_index_size);
    for (std::size_t j = 0; j < _index_size; ++j)
    {
        H[j] = 0;
        for (std::size_t i = 0; i < _sample_size; ++i)
            if (P[i][j] != 0)
                H[j] -= P[i][j] * std::log(P[i][j]);
        H[j] /= std::log(_sample_size);
    }
}

inline void EwTopsis::Impl::calcw(const std::vector<double> &H, std::vector<double> &w)
{
    w.resize(_index_size);
    double tmp = static_cast<double>(_index_size) - std::accumulate(H.begin(), H.end(), 0.0);
    for (std::size_t j = 0; j < _index_size; ++j)
        w[j] = (1 - H[j]) / tmp;
}

inline void EwTopsis::Impl::calcS(const std::vector<double> &w, const std::vector<std::vector<double>> &R, std::vector<double> &S)
{
    S.resize(_sample_size);
    for (std::size_t i = 0; i < _sample_size; ++i)
    {
        S[i] = 0;
        for (std::size_t j = 0; j < _index_size; ++j)
            S[i] += w[j] * R[i][j];
    }
}

////////////////////////////////// Munkres //////////////////////////////////

Munkres::Munkres(const std::vector<std::vector<double>> &cost_matrix) : _impl(new Impl(cost_matrix)) {}
Munkres::~Munkres() = default;
std::vector<std::size_t> Munkres::solve() noexcept { return _impl->solve(); }

Munkres::Impl::Impl(const std::vector<std::vector<double>> &cost_matrix)
    : _m(cost_matrix.size()), _n(cost_matrix.front().size()), _matrix(cost_matrix),
      _mask(_m), _row_covered(_m, false), _col_covered(_n, false)
{
    for (const auto &row : _matrix)
        if (row.size() != _n)
            RMVL_Error(RMVL_StsBadArg, "Matrix: \"cost_matrix\" is not rectangular");
    for (auto &row : _mask)
        row.resize(_n);
}

std::vector<std::size_t> Munkres::Impl::solve() noexcept
{
    while (_step != Step::End)
    {
        switch (_step)
        {
        case Step::Step1:
            step1();
            break;
        case Step::Step2:
            step2();
            break;
        case Step::Step3:
            step3();
            break;
        case Step::Step4:
            step4();
            break;
        case Step::Step5:
            step5();
            break;
        case Step::Step6:
            step6();
            break;
        case Step::End:
            break;
        }
    }
    std::vector<std::size_t> retval(_m);
    for (std::size_t r = 0; r < _m; r++)
        for (std::size_t c = 0; c < _n; c++)
            if (_mask[r][c] == 1)
                retval[r] = c;
    return retval;
}

std::tuple<int, int> Munkres::Impl::findOneZero(const std::vector<bool> &row_covered, const std::vector<bool> &col_covered)
{
    for (std::size_t r = 0; r < _m; r++)
        for (std::size_t c = 0; c < _n; c++)
            if (_matrix[r][c] == 0 && !row_covered[r] && !col_covered[c])
                return {static_cast<int>(r),
                        static_cast<int>(c)};
    return {-1, -1};
}

bool Munkres::Impl::isStarInRow(int row)
{
    const std::size_t n = _mask.front().size();
    for (std::size_t c = 0; c < n; c++)
        if (_mask[row][c] == 1)
            return true;
    return false;
}

int Munkres::Impl::findStarInRow(int row)
{
    const std::size_t n = _mask.front().size();
    for (std::size_t c = 0; c < n; c++)
        if (_mask[row][c] == 1)
            return static_cast<int>(c);
    return -1;
}

int Munkres::Impl::findStarInCol(int col)
{
    const std::size_t m = _mask.size();
    for (std::size_t r = 0; r < m; r++)
        if (_mask[r][col] == 1)
            return static_cast<int>(r);
    return -1;
}

int Munkres::Impl::findPrimeInRow(int row)
{
    const std::size_t n = _mask.front().size();
    for (std::size_t c = 0; c < n; c++)
        if (_mask[row][c] == 2)
            return static_cast<int>(c);
    return -1;
}

void Munkres::Impl::step1()
{
    double min_in_row{};
    for (auto &row : _matrix)
    {
        min_in_row = *std::min_element(row.begin(), row.end());
        std::for_each(row.begin(), row.end(), [min_in_row](double &val) { val -= min_in_row; });
    }
    _step = Step::Step2;
}

void Munkres::Impl::step2()
{
    for (std::size_t r = 0; r < _m; r++)
    {
        for (std::size_t c = 0; c < _n; c++)
        {
            if (_matrix[r][c] == 0 && !_row_covered[r] && !_col_covered[c])
            {
                _mask[r][c] = 1;
                _row_covered[r] = true;
                _col_covered[c] = true;
            }
        }
    }
    clearCovers();
    _step = Step::Step3;
}

void Munkres::Impl::step3()
{
    std::size_t colcount = 0;
    for (std::size_t r = 0; r < _m; r++)
        for (std::size_t c = 0; c < _n; c++)
            if (_mask[r][c] == 1)
                _col_covered[c] = true;
    for (std::size_t c = 0; c < _n; c++)
        if (_col_covered[c])
            colcount++;
    if (colcount >= _n || colcount >= _m)
        _step = Step::End;
    else
        _step = Step::Step4;
}

void Munkres::Impl::step4()
{
    int row = -1, col = -1;
    bool done = false;
    while (!done)
    {
        std::tie(row, col) = findOneZero(_row_covered, _col_covered);
        if (row == -1)
        {
            done = true;
            _step = Step::Step6;
        }
        else
        {
            _mask[row][col] = 2;
            if (isStarInRow(row))
            {
                int sta_Rcol = findStarInRow(row);
                _row_covered[row] = true;
                _col_covered[sta_Rcol] = false;
            }
            else
            {
                done = true;
                _step = Step::Step5;
                _first_zero = std::make_pair(row, col);
            }
        }
    }
}

void Munkres::Impl::step5()
{
    _path.clear();
    _path.push_back(_first_zero);

    bool done = false;
    do
    {
        int r = findStarInCol(_path.back().second);
        if (r > -1)
            _path.emplace_back(r, _path.back().second);
        else
            done = true;

        if (!done)
        {
            int c = findPrimeInRow(_path.back().first);
            _path.emplace_back(_path.back().first, c);
        }
    } while (!done);

    // augment path
    for (const auto &[r, c] : _path)
        _mask[r][c] = (_mask[r][c] == 1 ? 0 : 1);
    clearCovers();
    erasePrimes();
    _step = Step::Step3;
}

void Munkres::Impl::step6()
{
    double minval = std::numeric_limits<double>::max();
    for (std::size_t r = 0; r < _m; r++)
        for (std::size_t c = 0; c < _n; c++)
            if (!_row_covered[r] && !_col_covered[c])
                if (minval > _matrix[r][c])
                    minval = _matrix[r][c];
    // 重叠区域 +min，非覆盖区域 -min
    for (std::size_t r = 0; r < _m; r++)
    {
        for (std::size_t c = 0; c < _n; c++)
        {
            _matrix[r][c] += (_row_covered[r] ? minval : 0);
            _matrix[r][c] -= (!_col_covered[c] ? minval : 0);
        }
    }
    _step = Step::Step4;
}

void Munkres::Impl::clearCovers()
{
    std::fill(_row_covered.begin(), _row_covered.end(), false);
    std::fill(_col_covered.begin(), _col_covered.end(), false);
}

void Munkres::Impl::erasePrimes()
{
    std::for_each(_mask.begin(), _mask.end(), [](std::vector<int> &row) {
        std::for_each(row.begin(), row.end(), [](int &val) { val = (val == 2 ? 0 : val); });
    });
}

} // namespace rm
