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

#ifdef HAVE_RMVL_ORT
#include "rmvl/ml/ort.h"
#endif

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
#ifdef HAVE_RMVL_ORT
    std::unique_ptr<OnnxRT> _ort;
    std::unordered_map<int, RobotType> _robot_t;
#else
    const int *_ort = nullptr;
#endif // HAVE_RMVL_ORT

public:
    ArmorDetector() = default;
    ~ArmorDetector() = default;

#ifdef HAVE_RMVL_ORT
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
#endif // HAVE_RMVL_ORT

    /**
     * @brief 装甲板识别核心函数
     *
     * @param[in out] groups 序列组容器
     * @param[in] src 原图像
     * @param[in] color 待识别的颜色
     * @param[in] gyro_data 陀螺仪数据
     * @param[in] record_time 时间戳
     * @return 识别信息结构体
     */
    DetectInfo detect(std::vector<group_ptr> &groups, cv::Mat &src, PixChannel color,
                      const GyroData &gyro_data, int64 record_time) override;

    //! 构建 ArmorDetector
    static inline std::unique_ptr<ArmorDetector> make_detector() { return std::make_unique<ArmorDetector>(); }

#ifdef HAVE_RMVL_ORT
    /**
     * @brief 构建 ArmorDetector
     *
     * @param[in] model ONNX_Runtime 数字识别模型
     */
    static inline std::unique_ptr<ArmorDetector> make_detector(const std::string &model) { return std::make_unique<ArmorDetector>(model); }
#endif // HAVE_RMVL_ORT

private:
    /**
     * @brief 找出所有目标
     *
     * @param[in] src 预处理之后的图像
     * @param[out] features 找到的特征列表
     * @param[out] combos 找到的组合体列表
     * @param[out] rois 找到的组合体对应的 ROI 列表
     */
    void find(cv::Mat &src, std::vector<feature_ptr> &features, std::vector<combo_ptr> &combos, std::vector<cv::Mat> &rois);

    /**
     * @brief 匹配、更新时间序列
     *
     * @param[in] groups 所有序列组
     * @param[in] combos 每一帧的所有目标
     */
    void match(std::vector<group_ptr> &groups, std::vector<combo_ptr> &combos);

    /**
     * @brief 寻找灯条
     *
     * @param bin 二值图
     *
     * @return 找到的灯条
     */
    std::vector<light_blob_ptr> findLightBlobs(cv::Mat &bin);

    /**
     * @brief 匹配装甲板
     *
     * @param[in] light_blobs 利用找到的灯条匹配装甲板
     * @return 当前帧找到的所有装甲板
     */
    std::vector<armor_ptr> findArmors(std::vector<light_blob_ptr> &light_blobs);

    /**
     * @brief 在多个装甲板共享同一个灯条时，根据匹配误差移除装甲板
     *
     * @param[in out] armors 待筛选的所有装甲板
     */
    void eraseErrorArmors(std::vector<armor_ptr> &armors);

    /**
     * @brief 删除强光误识别的灯条
     *
     * @param[in] src 原图像
     * @param[in out] blobs 所有灯条
     */
    void eraseBrightBlobs(cv::Mat src, std::vector<light_blob_ptr> &blobs);

    /**
     * @brief 删除因数字识别未正确识别导致的假装甲板
     *
     * @param[in out] armors 所有装甲板
     */
    void eraseFakeArmors(std::vector<armor_ptr> &armors);

    /**
     * @brief 装甲板匹配至时间序列
     *
     * @param[in] trackers 所有追踪器序列
     * @param[in] combos 每一帧的所有目标
     */
    void matchArmors(std::vector<tracker_ptr> &trackers, std::vector<combo_ptr> &combos);

    /**
     * @brief 及时删除多帧为空的序列
     *
     * @param[in out] trackers 所有追踪器序列
     */
    void eraseNullTracker(std::vector<tracker_ptr> &trackers);

    /**
     * @brief 删除因数字识别判断出的伪装甲板序列
     *
     * @param[in out] trackers 所有追踪器序列
     */
    void eraseFakeTracker(std::vector<tracker_ptr> &trackers);
};

//! @} armor_detector

} // namespace rm
