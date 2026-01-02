/**
 * @file dsp.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Digital Signal Processing Module 数字信号处理模块
 * @version 1.0
 * @date 2024-04-28
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#ifdef HAVE_OPENCV
#include <opencv2/imgproc.hpp>
#else
#include "rmvl/algorithm/math.hpp"
#endif

#include "rmvl/algorithm/dsp.hpp"

namespace rm {

RealSignal Gx(const ComplexSignal &x, GxType type) {
    const std::size_t N = x.size();
    RealSignal retval(N);
    switch (type) {
    case GxType::Amp:
        for (std::size_t i = 0; i < N; ++i)
            retval[i] = std::abs(x[i]) / N;
        return retval;
    case GxType::Phase:
        for (std::size_t i = 0; i < N; ++i)
            retval[i] = std::arg(x[i]);
        return retval;
    case GxType::Power:
        for (std::size_t i = 0; i < N; ++i)
            retval[i] = std::norm(x[i]) * 2. / N;
        return retval;
    case GxType::LogPower:
        for (std::size_t i = 0; i < N; ++i)
            retval[i] = 10. * std::log(std::norm(x[i]) * 2. / N);
        return retval;
    default:
        return retval;
    }
}

#ifdef HAVE_OPENCV

cv::Mat draw(const RealSignal &datas, const cv::Scalar &color) {
    cv::Mat img(cv::Size(int(datas.size() * 2.5), int(datas.size() * 1.5)), CV_8UC3, cv::Scalar(40, 40, 40));
    int cx{img.cols / 2}, cy{img.rows / 2};

    double max_val = *std::max_element(datas.begin(), datas.end(), [](double lhs, double rhs) { return std::abs(lhs) < std::abs(rhs); });
    double height_ratio{cy / max_val};

    cv::line(img, cv::Point(0, cy), cv::Point(img.cols, cy), cv::Scalar(255, 255, 255), 1);
    cv::line(img, cv::Point(0.2 * cx, 0), cv::Point(0.2 * cx, img.rows), cv::Scalar(255, 255, 255), 1);
    cv::line(img, cv::Point(0.2 * cx, 0.2 * cy), cv::Point(0.22 * cx, 0.2 * cy), cv::Scalar(255, 255, 255));
    cv::line(img, cv::Point(0.2 * cx, 1.8 * cy), cv::Point(0.22 * cx, 1.8 * cy), cv::Scalar(255, 255, 255));
    cv::putText(img, std::to_string(max_val), cv::Point(0.22 * cx, 0.2 * cy), cv::FONT_HERSHEY_COMPLEX, 0.002 * cy, cv::Scalar(255, 255, 255));
    cv::putText(img, std::to_string(-max_val), cv::Point(0.22 * cx, 1.8 * cy), cv::FONT_HERSHEY_COMPLEX, 0.002 * cy, cv::Scalar(255, 255, 255));

    for (std::size_t i = 0; i + 1 < datas.size(); ++i)
        cv::line(img, cv::Point(cx + 2 * (i - datas.size() / 2), height_ratio * 0.8 * -datas[i] + cy),
                 cv::Point(cx + 2 * (i + 1 - datas.size() / 2), height_ratio * 0.8 * -datas[i + 1] + cy),
                 color, 2);

    return img;
}

ComplexSignal dft(const ComplexSignal &xt) {
    const int N = static_cast<int>(xt.size());
    // std::deque -> cv::Mat
    cv::Mat input(1, N, CV_64FC2);
    for (int i = 0; i < N; ++i)
        input.at<cv::Vec2d>(0, i) = {xt[i].real(), xt[i].imag()};
    cv::Mat output;
    // process
    cv::dft(input, output, cv::DFT_COMPLEX_OUTPUT);
    ComplexSignal res(N);
    for (int i = 0; i < N; ++i)
        res[i] = {output.at<cv::Vec2d>(0, i)[0], output.at<cv::Vec2d>(0, i)[1]};

    return res;
}

ComplexSignal idft(const ComplexSignal &Xf) {
    const int N = static_cast<int>(Xf.size());
    // std::deque -> cv::Mat
    cv::Mat input(1, N, CV_64FC2);
    for (int i = 0; i < N; ++i)
        input.at<cv::Vec2d>(0, i) = {Xf[i].real(), Xf[i].imag()};
    cv::Mat output;
    // process
    cv::dft(input, output, cv::DFT_INVERSE | cv::DFT_COMPLEX_OUTPUT);
    ComplexSignal res(N);
    for (int i = 0; i < N; ++i)
        res[i] = {output.at<cv::Vec2d>(0, i)[0] / N, output.at<cv::Vec2d>(0, i)[1] / N};

    return res;
}

#else

using namespace numeric_literals;

/**
 * @brief 快速傅里叶变换递归函数
 *
 * @param[in] xt 离散时域信号
 * @param[in] N 信号长度
 * @return 离散频域信号
 */
static inline ComplexSignal fftprocess(const ComplexSignal &xt) {
    std::size_t N = xt.size();
    if (N == 1)
        return xt;
    ComplexSignal pe(N / 2), po(N / 2);
    for (std::size_t i = 0; i < N / 2; ++i)
        pe[i] = xt[2 * i], po[i] = xt[2 * i + 1];
    ComplexSignal y(N);
    auto ye = fftprocess(pe), yo = fftprocess(po);
    for (std::size_t k = 0; k < N / 2; ++k) {
        auto wk_yo = std::polar(1.0, -2_PI / N * k) * yo[k];
        y[k] = ye[k] + wk_yo, y[k + N / 2] = ye[k] - wk_yo;
    }

    return y;
}

static inline ComplexSignal ifftprocess(const ComplexSignal &Xf) {
    std::size_t N = Xf.size();
    if (N == 1)
        return Xf;
    ComplexSignal pe(N / 2), po(N / 2);
    for (std::size_t i = 0; i < N / 2; ++i)
        pe[i] = Xf[2 * i], po[i] = Xf[2 * i + 1];
    ComplexSignal y(N);
    auto ye = ifftprocess(pe), yo = ifftprocess(po);
    for (std::size_t k = 0; k < N / 2; ++k) {
        auto wk_yo = std::polar(1.0, 2_PI / N * k) * yo[k];
        y[k] = ye[k] + wk_yo, y[k + N / 2] = ye[k] - wk_yo;
    }

    return y;
}

ComplexSignal dft(const ComplexSignal &xt) {
    std::size_t N = xt.size();
    if (std::log2(N) != std::floor(std::log2(N)))
        RMVL_Error(RMVL_StsBadArg, "The size of the signal must be a power of 2.");
    return fftprocess(xt);
}

ComplexSignal idft(const ComplexSignal &Xf) {
    std::size_t N = Xf.size();
    if (std::log2(N) != std::floor(std::log2(N)))
        RMVL_Error(RMVL_StsBadArg, "The size of the signal must be a power of 2.");
    auto x = ifftprocess(Xf);
    for (auto &xi : x)
        xi /= static_cast<double>(N);
    return x;
}

#endif // HAVE_OPENCV

} // namespace rm
