/**
 * @file armor_detector.h
 * @author RoboMaster Vision Community
 * @brief 装甲板识别派生类头文件
 * @version 1.0
 * @date 2021-08-18
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "detector.h"

#include "rmvl/ml/ort.h"
#include "rmvl/tracker/planar_tracker.h"

namespace rm
{

//! @addtogroup armor_detector
//! @{

//! @example samples/detector/mv/sample_mv_armor_collection.cpp 装甲板收集例程 Armor collection demo
//! @example samples/detector/hik/sample_hik_armor_collection.cpp 装甲板收集例程 Armor collection demo

//! 装甲板识别模块
class ArmorDetector final : public detector
{
    std::unique_ptr<OnnxRT> _ort;
    std::unordered_map<int, RobotType> _robot_t;

public:
    ArmorDetector() = default;
    ~ArmorDetector() = default;

    explicit ArmorDetector(const std::string &model)
    {
        _ort = std::make_unique<OnnxRT>(model);
        _robot_t[0] = RobotType::UNKNOWN;
        _robot_t[1] = RobotType::HERO;
        _robot_t[2] = RobotType::ENGINEER;
        _robot_t[3] = RobotType::INFANTRY_3;
        _robot_t[4] = RobotType::INFANTRY_4;
        _robot_t[5] = RobotType::INFANTRY_5;
        _robot_t[6] = RobotType::OUTPOST;
        _robot_t[7] = RobotType::BASE;
        _robot_t[8] = RobotType::SENTRY;
    }

    /**
     * @brief 装甲板识别核心函数
     *
     * @param[in out] groups 序列组容器
     * @param[in] src 原图像
     * @param[in] color 待识别的颜色
     * @param[in] gyro_data 陀螺仪数据
     * @param[in] tick 当前时间点
     * @return 识别信息结构体
     */
    DetectInfo detect(std::vector<group::ptr> &groups, cv::Mat &src, PixChannel color,
                      const GyroData &gyro_data, double tick) override;

    //! 构建 ArmorDetector
    static inline std::unique_ptr<ArmorDetector> make_detector() { return std::make_unique<ArmorDetector>(); }

    /**
     * @brief 构建 ArmorDetector
     *
     * @param[in] model ONNX_Runtime 数字识别模型
     */
    static inline std::unique_ptr<ArmorDetector> make_detector(const std::string &model) { return std::make_unique<ArmorDetector>(model); }

private:
    /**
     * @brief 找出所有目标
     *
     * @param[in] src 预处理之后的图像
     * @param[out] features 找到的特征列表
     * @param[out] combos 找到的组合体列表
     * @param[out] rois 找到的组合体对应的 ROI 列表
     */
    void find(cv::Mat &src, std::vector<feature::ptr> &features, std::vector<combo::ptr> &combos, std::vector<cv::Mat> &rois);

    /**
     * @brief 匹配、更新时间序列
     *
     * @param[in] groups 所有序列组
     * @param[in] combos 每一帧的所有目标
     */
    void match(std::vector<group::ptr> &groups, const std::vector<combo::ptr> &combos);

    /**
     * @brief 寻找灯条
     *
     * @param bin 二值图
     *
     * @return 找到的灯条
     */
    std::vector<LightBlob::ptr> findLightBlobs(cv::Mat &bin);

    /**
     * @brief 匹配装甲板
     *
     * @param[in] light_blobs 找到的灯条（函数内部会对此进行排序）
     * @return 当前帧找到的所有装甲板
     */
    std::vector<Armor::ptr> findArmors(std::vector<LightBlob::ptr> &light_blobs);

    /**
     * @brief 在多个装甲板共享同一个灯条时，根据匹配误差移除装甲板
     *
     * @param[in out] armors 待筛选的所有装甲板
     */
    void eraseErrorArmors(std::vector<Armor::ptr> &armors);

    /**
     * @brief 删除强光误识别的灯条
     *
     * @param[in] src 原图像
     * @param[in out] blobs 所有灯条
     */
    void eraseBrightBlobs(cv::Mat src, std::vector<LightBlob::ptr> &blobs);

    /**
     * @brief 删除因数字识别未正确识别导致的假装甲板
     *
     * @param[in out] armors 所有装甲板
     */
    void eraseFakeArmors(std::vector<Armor::ptr> &armors);

    /**
     * @brief 装甲板匹配至时间序列
     *
     * @param[in out] trackers 所有追踪器序列
     * @param[in] combos 每一帧的所有目标
     */
    void matchArmors(std::vector<tracker::ptr> &trackers, const std::vector<combo::ptr> &combos);

    /**
     * @brief 及时删除多帧为空的序列
     *
     * @param[in out] trackers 所有追踪器序列
     */
    void eraseNullTracker(std::vector<tracker::ptr> &trackers);

    /**
     * @brief 删除因数字识别判断出的伪装甲板序列
     *
     * @param[in out] trackers 所有追踪器序列
     */
    void eraseFakeTracker(std::vector<tracker::ptr> &trackers);
};

//! @} armor_detector

} // namespace rm
