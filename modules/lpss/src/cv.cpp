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

#include "rmvl/lpss/cv.hpp"

#ifdef HAVE_OPENCV

namespace rm::cvmsg {

cv::Point3d from_msg(const msg::Point &pt_msg) { return {pt_msg.x, pt_msg.y, pt_msg.z}; }

msg::Point to_msg(const cv::Point3d &pt) { return {pt.x, pt.y, pt.z}; }

cv::Point3f from_msg(const msg::Point32 &pt_msg) { return {pt_msg.x, pt_msg.y, pt_msg.z}; }

msg::Point32 to_msg(const cv::Point3f &pt) { return {pt.x, pt.y, pt.z}; }

cv::Vec3d from_msg(const msg::Vector3 &vec_msg) { return {vec_msg.x, vec_msg.y, vec_msg.z}; }

msg::Vector3 to_msg(const cv::Vec3d &vec) { return {vec[0], vec[1], vec[2]}; }

#if LPSS_CV_VERSION >= 40501

cv::Quatd from_msg(const msg::Quaternion &quat_msg) { return {quat_msg.w, quat_msg.x, quat_msg.y, quat_msg.z}; }

msg::Quaternion to_msg(const cv::Quatd &quat) { return {quat.w, quat.x, quat.y, quat.z}; }

cv::Affine3d from_msg(const msg::Transform &tf_msg) {
    cv::Quatd quat = from_msg(tf_msg.rotation);
    cv::Vec3d trans = from_msg(tf_msg.translation);
    return cv::Affine3d(quat.toRotMat3x3(), trans);
}

msg::Transform to_msg(const cv::Affine3d &tf) {
    cv::Matx33d rot_mat = tf.rotation();
    msg::Transform tf_msg{};
    tf_msg.rotation = to_msg(cv::Quatd::createFromRotMat(rot_mat));
    tf_msg.translation = to_msg(tf.translation());
    return tf_msg;
}

#endif

cv::Mat from_msg(const msg::Image &img_msg) {
    if (img_msg.data.empty()) {
        return cv::Mat{};
    }
    if (img_msg.encoding == msg::Image::encoding_mono8)
        return cv::Mat(img_msg.height, img_msg.width, CV_8UC1, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == msg::Image::encoding_bgr8 || img_msg.encoding == msg::Image::encoding_rgb8)
        return cv::Mat(img_msg.height, img_msg.width, CV_8UC3, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == msg::Image::encoding_rgba8 || img_msg.encoding == msg::Image::encoding_bgra8)
        return cv::Mat(img_msg.height, img_msg.width, CV_8UC4, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == msg::Image::encoding_mono16)
        return cv::Mat(img_msg.height, img_msg.width, CV_16UC1, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == msg::Image::encoding_bayer_rggb8)
        return cv::Mat(img_msg.height, img_msg.width, CV_8UC1, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == msg::Image::encoding_bayer_bggr8)
        return cv::Mat(img_msg.height, img_msg.width, CV_8UC1, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == msg::Image::encoding_bayer_rggb16)
        return cv::Mat(img_msg.height, img_msg.width, CV_16UC1, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == msg::Image::encoding_bayer_bggbr16)
        return cv::Mat(img_msg.height, img_msg.width, CV_16UC1, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == msg::Image::encoding_yuv422)
        return cv::Mat(img_msg.height, img_msg.width, CV_8UC2, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else if (img_msg.encoding == msg::Image::encoding_yuv420)
        return cv::Mat(img_msg.height * 3 / 2, img_msg.width, CV_8UC1, const_cast<uint8_t *>(img_msg.data.data())).clone();
    else
        return cv::Mat{};
}

msg::Image to_msg(cv::Mat img, uint8_t encoding) {
    if (encoding > msg::Image::encoding_yuv420 || img.empty())
        return msg::Image{};

    msg::Image img_msg{};
    img_msg.height = img.rows;
    img_msg.width = img.cols;
    img_msg.encoding = encoding;

    size_t row_bytes = img.cols * img.elemSize();
    img_msg.data.resize(img.rows * row_bytes);
    for (int i = 0; i < img.rows; ++i)
        std::memcpy(img_msg.data.data() + i * row_bytes, img.ptr(i), row_bytes);

    return img_msg;
}

} // namespace rm::cvmsg

#endif
