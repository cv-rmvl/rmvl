/**
 * @file find.cpp
 * @author RoboMaster Vision Community
 * @brief find features and combos
 * @version 0.1
 * @date 2022-12-10
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/imgproc.hpp>

#include "rmvl/detector/gyro_detector.h"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/detector/gyro_detector.h"

namespace rm
{

void GyroDetector::find(cv::Mat &src, std::vector<feature::ptr> &features, std::vector<combo::ptr> &combos, std::vector<cv::Mat> &rois)
{
    // ----------------------- light_blob -----------------------
    // 找到所有灯条
    std::vector<LightBlob::ptr> blobs = findLightBlobs(src);
    // 删除过亮灯条
    eraseBrightBlobs(src, blobs);
    // ------------------------- armor --------------------------
    if (blobs.size() >= 2)
    {
        // 找到所有装甲板
        std::vector<Armor::ptr> armors = findArmors(blobs);
        if (_ort)
        {
            rois.clear();
            rois.reserve(armors.size());
            for (const auto &armor : armors)
            {
                cv::Mat roi = Armor::getNumberROI(src, armor);
                PreprocessOptions preop;
                preop.means = {para::gyro_detector_param.MODEL_MEAN};
                preop.stds = {para::gyro_detector_param.MODEL_STD};
                int idx = ClassificationNet::cast(_ort->inference({roi}, preop, {})).first;
                armor->setType(_robot_t[idx]);
                rois.emplace_back(roi);
            }
            // eraseFakeArmors(armors);
        }
        else
            for (auto armor : armors)
                armor->setType(RobotType::UNKNOWN);

        // 根据匹配误差筛选
        eraseErrorArmors(armors);
        // 更新至特征容器
        for (const auto &blob : blobs)
            features.emplace_back(blob);
        // 更新至组合体容器
        for (const auto &armor : armors)
            combos.emplace_back(armor);
    }
}

std::vector<LightBlob::ptr> GyroDetector::findLightBlobs(cv::Mat &bin)
{
    // 储存找到的灯条
    std::vector<LightBlob::ptr> light_blobs;
    // 储存查找出的轮廓
    std::vector<std::vector<cv::Point>> contours;
    // 查找最外围轮廓
    cv::findContours(bin, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    for (auto &contour : contours)
    {
        // 排除面积过小的误识别
        if (cv::contourArea(contour) < para::gyro_detector_param.MIN_CONTOUR_AREA)
            continue;
        // 构造灯条对象
        LightBlob::ptr p_light = LightBlob::make_feature(contour);
        // 将识别出的灯条 push 到 light_blobs 中
        if (p_light != nullptr)
            light_blobs.push_back(p_light);
    }
    return light_blobs;
}

std::vector<Armor::ptr> GyroDetector::findArmors(std::vector<LightBlob::ptr> &light_blobs)
{
    // 灯条从左到右排序
    sort(light_blobs.begin(), light_blobs.end(), [](LightBlob::const_ptr lhs, LightBlob::const_ptr rhs) {
        return lhs->center().x < rhs->center().x;
    });
    // 储存所有匹配到的装甲板
    std::vector<Armor::ptr> current_armors;
    if (light_blobs.size() < 2)
        return current_armors;
    // -------------------------------------【匹配】-------------------------------------
    for (size_t i = 0; i + 1 < light_blobs.size(); ++i)
    {
        for (size_t j = i + 1; j < light_blobs.size(); j++)
        {
            // 构造装甲板
            Armor::ptr armor = Armor::make_combo(light_blobs[i], light_blobs[j], _imu_data, _tick);
            // 是否找到目标（未找到则单次跳出）
            if (armor == nullptr)
                continue;
            /**
             * @brief 装甲板区域内是否包含灯条中心点
             * @note 分支限界，复杂度不会过高
             */
            bool contain = false;
            for (size_t k = i + 1; k < j; ++k)
            {
                if (Armor::isContainBlob(light_blobs[k], armor))
                {
                    contain = true;
                    break;
                }
            }
            // 是否包含灯条中心点（包含则单次跳出）
            if (contain)
                continue;
            current_armors.emplace_back(armor);
        }
    }
    return current_armors;
}

void GyroDetector::eraseErrorArmors(std::vector<Armor::ptr> &armors)
{
    // 判断大小是否允许被删除
    if (armors.size() < 2)
        return;
    std::unordered_map<Armor::const_ptr, bool> armor_map; // [装甲板 : 能否删除]
    for (const auto &armor : armors)
        armor_map.at(armor) = false;
    // 设置是否删除的标志位
    for (size_t i = 0; i + 1 < armors.size(); i++)
    {
        for (size_t j = i + 1; j < armors.size(); j++)
        {
            // 共享左灯条 or 右灯条，优先匹配宽度小的
            if (armors[i]->at(0) == armors[j]->at(0) || armors[i]->at(1) == armors[j]->at(1))
                armor_map[armors[i]->width() > armors[j]->width() ? armors[i] : armors[j]] = true;
            else if (armors[i]->at(0) == armors[j]->at(1) || armors[i]->at(1) == armors[j]->at(0))
                armor_map[armors[i]->getError() > armors[j]->getError() ? armors[i] : armors[j]] = true;
        }
    }
    // 删除
    armors.erase(std::remove_if(armors.begin(), armors.end(), [&](Armor::const_ptr val) {
                     return armor_map.at(val);
                 }),
                 armors.end());
}

void GyroDetector::eraseFakeArmors(std::vector<Armor::ptr> &armors)
{
    armors.erase(std::remove_if(armors.begin(), armors.end(), [](Armor::const_ptr it) {
                     return it->type().RobotTypeID == RobotType::UNKNOWN;
                 }),
                 armors.end());
}

void GyroDetector::eraseBrightBlobs(cv::Mat src, std::vector<LightBlob::ptr> &blobs)
{
    blobs.erase(std::remove_if(blobs.begin(), blobs.end(), [&](LightBlob::const_ptr blob) {
                    int total_brightness = 0;
                    for (int i = -5; i <= 5; i++)
                    {
                        if (i == 0)
                            continue;
                        int x = blob->center().x - blob->height() * i / 5;
                        x = (x > src.cols) ? src.cols - 1 : x;
                        x = (x < 0) ? 1 : x;
                        int y = blob->center().y;
                        y = y < 0 ? 1 : y;
                        y = (y > src.rows) ? src.rows - 1 : y;
                        auto colors = src.at<cv::Vec3b>(y, x);
                        int brightness = 0.1 * colors[0] + 0.6 * colors[1] + 0.3 * colors[2];
                        total_brightness += brightness;
                    }
                    return (total_brightness > 1100);
                }),
                blobs.end());
}

} // namespace rm
