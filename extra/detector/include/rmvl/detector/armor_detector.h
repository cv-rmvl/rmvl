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

#include "rmvl/combo/armor.h"
#include "rmvl/ml/ort.h"

namespace rm {

//! @addtogroup armor_detector
//! @{

//! @example samples/detector/mv/sample_mv_armor_collection.cpp 装甲板收集例程 Armor collection demo
//! @example samples/detector/hik/sample_hik_armor_collection.cpp 装甲板收集例程 Armor collection demo

//! 装甲板识别模块
class RMVL_EXPORTS_W_DEU ArmorDetector final : public detector {
    std::unique_ptr<OnnxNet> _ort;
    std::unordered_map<int, RobotType> _robot_t;

public:
    //! @cond
    ArmorDetector() = default;
    ~ArmorDetector() = default;

    explicit ArmorDetector(std::string_view model);
    //! @endcond

    /**
     * @brief 装甲板识别核心函数
     *
     * @param[in out] groups 序列组容器
     * @param[in] src 原图像
     * @param[in] color 待识别的颜色
     * @param[in] imu_data IMU 数据
     * @param[in] tick 当前时间点
     * @return 识别信息结构体
     */
    RMVL_W DetectInfo detect(std::vector<group::ptr> &groups, const cv::Mat &src, uint8_t color, const ImuData &imu_data, double tick) override;

    //! 构建 ArmorDetector
    RMVL_W static inline auto make_detector() { return std::make_unique<ArmorDetector>(); }

    /**
     * @brief 构建 ArmorDetector
     *
     * @param[in] model ONNX_Runtime 数字识别模型
     */
    RMVL_W static inline auto make_detector(std::string_view model) { return std::make_unique<ArmorDetector>(model); }

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
};

//! @} armor_detector

} // namespace rm
