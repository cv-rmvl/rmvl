/**
 * @file cv.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2026-01-28
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <unordered_set>

#include "rmvl/lpss/cv.hpp"

#ifdef HAVE_OPENCV

namespace rm {

cv::Mat from_msg(const msg::Image &img_msg) {
    if (img_msg.data.empty()) {
        return cv::Mat{};
    }
    if (img_msg.encoding == "mono8")
        return cv::Mat(img_msg.height, img_msg.width, CV_8UC1, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == "bgr8" || img_msg.encoding == "rgb8")
        return cv::Mat(img_msg.height, img_msg.width, CV_8UC3, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == "rgba8" || img_msg.encoding == "bgra8")
        return cv::Mat(img_msg.height, img_msg.width, CV_8UC4, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == "mono16")
        return cv::Mat(img_msg.height, img_msg.width, CV_16UC1, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == "bayer_rggb8")
        return cv::Mat(img_msg.height, img_msg.width, CV_8UC1, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == "bayer_bggr8")
        return cv::Mat(img_msg.height, img_msg.width, CV_8UC1, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else
        return cv::Mat{};
}

msg::Image to_msg(cv::Mat img, std::string_view encoding) {
    static std::unordered_set<std::string> valid_encodings = {"rgb8", "bgr8", "mono8", "mono16", "rgba8", "bgra8", "bayer_rggb8", "bayer_bggr8"};
    if (valid_encodings.find(std::string(encoding)) == valid_encodings.end())
        return msg::Image{};

    msg::Image img_msg{};
    img_msg.height = img.rows;
    img_msg.width = img.cols;
    img_msg.encoding = encoding;
    size_t data_size = img.total() * img.elemSize();
    img_msg.data.resize(data_size);
    std::memcpy(img_msg.data.data(), img.data, data_size);
    return img_msg;
}

} // namespace rm

#endif
