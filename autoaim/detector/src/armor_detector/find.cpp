/**
 * @file find.cpp
 * @author RoboMaster Vision Community
 * @brief find features and combos
 * @version 1.0
 * @date 2021-08-18
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/imgproc.hpp>

#include "rmvl/detector/armor_detector.h"

#include "rmvlpara/camera.hpp"
#include "rmvlpara/detector/armor_detector.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

void ArmorDetector::find(Mat &src, vector<feature_ptr> &features, vector<combo_ptr> &combos)
{
    // ----------------------- light_blob -----------------------
    // 找到所有灯条
    vector<light_blob_ptr> blobs = findLightBlobs(src);
    // 删除过亮灯条
    eraseBrightBlobs(src, blobs);
    // ------------------------- armor --------------------------
    if (blobs.size() >= 2)
    {
        // 找到所有装甲板
        vector<armor_ptr> armors = this->findArmors(blobs);
#ifdef HAVE_RMVL_ORT
        if (_ort)
        {
            // 对装甲板从中间向两边排序
            sort(armors.begin(), armors.end(),
                 [&](const armor_ptr &lhs, const armor_ptr &rhs) -> bool
                 {
                     return abs(lhs->getCenter().x - src.cols / 2.f) <
                            abs(rhs->getCenter().x - src.cols / 2.f);
                 });
            for (auto armor : armors)
            {
                Mat roi = Armor::getNumberROI(src, armor);
                auto type = _ort->inference({roi})[0];
                armor->setType(_robot_t[type]);
            }
            // eraseFakeArmors(armors);
        }
        else
        {
#endif // HAVE_RMVL_ORT
            for (auto armor : armors)
                armor->setType(RobotType::UNKNOWN);
#ifdef HAVE_RMVL_ORT
        }
#endif // HAVE_RMVL_ORT

        // 根据匹配误差筛选
        this->eraseErrorArmors(armors);
        // 更新至特征容器
        for (const auto &blob : blobs)
            features.emplace_back(blob);
        // 更新至组合体容器
        for (const auto &armor : armors)
            combos.emplace_back(armor);
    }
}

vector<light_blob_ptr> ArmorDetector::findLightBlobs(Mat &bin)
{
    // 储存找到的灯条
    vector<light_blob_ptr> light_blobs;
    // 储存查找出的轮廓
    vector<vector<Point>> contours;
    // 查找最外围轮廓
    findContours(bin, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    for (auto &contour : contours)
    {
        // 排除面积过小的误识别
        if (contourArea(contour) < armor_detector_param.MIN_CONTOUR_AREA)
            continue;
        // 构造灯条对象
        light_blob_ptr p_light = LightBlob::make_feature(contour);
        // 将识别出的灯条 push 到 light_blobs 中
        if (p_light != nullptr)
            light_blobs.push_back(p_light);
    }
    return light_blobs;
}

vector<armor_ptr> ArmorDetector::findArmors(vector<light_blob_ptr> &light_blobs)
{
    // 灯条从左到右排序
    sort(light_blobs.begin(), light_blobs.end(),
         [&](const light_blob_ptr &p_left, const light_blob_ptr &p_right) -> bool
         {
             return p_left->getCenter().x < p_right->getCenter().x;
         });
    // 储存所有匹配到的装甲板
    vector<armor_ptr> current_armors;
    if (light_blobs.size() < 2)
        return current_armors;
    // -------------------------------------【匹配】-------------------------------------
    for (size_t i = 0; i + 1 < light_blobs.size(); ++i)
    {
        for (size_t j = i + 1; j < light_blobs.size(); j++)
        {
            // 构造装甲板
            armor_ptr armor = Armor::make_combo(light_blobs[i], light_blobs[j], _gyro_data, _tick);
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

void ArmorDetector::eraseErrorArmors(std::vector<armor_ptr> &armors)
{
    // 判断大小是否允许被删除
    if (armors.size() < 2)
        return;
    unordered_map<armor_ptr, bool> armor_map; // [装甲板 : 能否删除]
    for (const auto &armor : armors)
        armor_map[armor] = false;
    // 设置是否删除的标志位
    for (size_t i = 0; i + 1 < armors.size(); i++)
    {
        for (size_t j = i + 1; j < armors.size(); j++)
        {
            // 共享左灯条 or 右灯条，优先匹配宽度小的
            if (armors[i]->at(0) == armors[j]->at(0) || armors[i]->at(1) == armors[j]->at(1))
                armor_map[armors[i]->getWidth() > armors[j]->getWidth() ? armors[i] : armors[j]] = true;
            else if (armors[i]->at(0) == armors[j]->at(1) || armors[i]->at(1) == armors[j]->at(0))
                armor_map[armors[i]->getError() > armors[j]->getError() ? armors[i] : armors[j]] = true;
        }
    }
    // 删除
    armors.erase(remove_if(armors.begin(), armors.end(),
                           [&armor_map](const armor_ptr &val)
                           {
                               return armor_map[val];
                           }),
                 armors.end());
}

void ArmorDetector::eraseFakeArmors(vector<armor_ptr> &armors)
{
    armors.erase(remove_if(armors.begin(), armors.end(),
                           [&](armor_ptr &it)
                           {
                               return it->getType().RobotTypeID == RobotType::UNKNOWN;
                           }),
                 armors.end());
}

void ArmorDetector::eraseBrightBlobs(Mat src, vector<light_blob_ptr> &blobs)
{
    blobs.erase(remove_if(blobs.begin(), blobs.end(),
                          [&](light_blob_ptr &blob) -> bool
                          {
                              int total_brightness = 0;
                              for (int i = -5; i <= 5; i++)
                              {
                                  if (i == 0)
                                      continue;
                                  int x = blob->getCenter().x - blob->getHeight() * i / 5;
                                  x = (x > src.cols) ? src.cols - 1 : x;
                                  x = (x < 0) ? 1 : x;
                                  int y = blob->getCenter().y;
                                  y = y < 0 ? 1 : y;
                                  y = (y > src.rows) ? src.rows - 1 : y;
                                  auto colors = src.at<Vec3b>(y, x);
                                  int brightness = 0.1 * colors[0] + 0.6 * colors[1] + 0.3 * colors[2];
                                  total_brightness += brightness;
                              }
                              return (total_brightness > 1100);
                          }),
                blobs.end());
}
