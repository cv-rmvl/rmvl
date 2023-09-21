/**
 * @file gyro_detector.h
 * @author RoboMaster Vision Community
 * @brief 装甲板识别派生类头文件
 * @version 0.1
 * @date 2022-12-10
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <unordered_map>

#include "detector.h"

#ifdef HAVE_RMVL_ORT
#include "rmvl/ml/ort.h"
#endif

#include "rmvl/tracker/gyro_tracker.h"

namespace rm
{

//! @addtogroup gyro_detector
//! @{

//! 整车状态识别模块
class GyroDetector final : public detector
{
    int _armor_num; //!< 默认装甲板数目

#ifdef HAVE_RMVL_ORT
    std::unique_ptr<OnnxRT> _ort;
    std::unordered_map<int, RobotType> _robot_t;
#else
    const int *_ort = nullptr;
#endif // HAVE_RMVL_ORT

public:
    GyroDetector(int armor_num) : _armor_num(armor_num) {}
    ~GyroDetector() = default;

#ifdef HAVE_RMVL_ORT
    explicit GyroDetector(const std::string &model, int armor_num) : _armor_num(armor_num)
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
    DetectInfo detect(std::vector<group::ptr> &groups, cv::Mat &src, rm::PixChannel color,
                      const GyroData &gyro_data, int64 record_time) override;

    //! 构建 GyroDetector
    static inline std::unique_ptr<GyroDetector> make_detector(int armor_num = 0)
    {
        return std::make_unique<GyroDetector>(armor_num);
    }

#ifdef HAVE_RMVL_ORT
    /**
     * @brief 构建 GyroDetector
     *
     * @param[in] model ONNX-Runtime 数字识别模型
     */
    static inline std::unique_ptr<GyroDetector> make_detector(const std::string &model, int armor_num = 0)
    {
        return std::make_unique<GyroDetector>(model, armor_num);
    }
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
    void find(cv::Mat &src, std::vector<feature::ptr> &features, std::vector<combo::ptr> &combos, std::vector<cv::Mat> &rois);

    /**
     * @brief 匹配、更新时间序列
     *
     * @param[in] groups 所有序列组
     * @param[in] combos 每一帧的所有目标
     */
    void match(std::vector<group::ptr> &groups, std::vector<combo::ptr> &combos);

    /**
     * @brief 寻找灯条
     *
     * @param[in] bin 二值图
     * @return 找到的灯条
     */
    std::vector<LightBlob::ptr> findLightBlobs(cv::Mat &bin);

    /**
     * @brief 匹配装甲板
     *
     * @param[in out] light_blobs 灯条列表，内部进行从左到右排序
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
     * @brief 删除假装甲板
     *
     * @param[in out] armors 所有装甲板
     */
    void eraseFakeArmors(std::vector<Armor::ptr> &armors);

    /**
     * @brief 将指定装甲板匹配至单一 GyroGroup 的 tracker 中
     * @note
     * - 将 `combos` 逐一与所有的 `trackers` 按照空间位置进行匹配
     *
     * @param[in] group 单一 GyroGroup 序列组
     * @param[in] combos 单一序列组所对应的装甲板
     */
    void matchOneGroup(group::ptr group, const std::vector<combo::ptr> &combos);

    /**
     * @brief 删除因数字识别判断出的伪装甲板序列
     *
     * @param[in] trackers 所有追踪器序列
     */
    void eraseFakeTracker(std::vector<tracker::ptr> &trackers);

    /**
     * @brief 通过熵权法推理装甲板的匹配对象
     *
     * @param[in] group 序列组
     * @param[in] combos 匹配到序列组中的装甲板
     * @return 推理得到的数据指标
     */
    std::unordered_map<size_t, size_t> ewTopsisInference(group::ptr group, const std::vector<combo::ptr> &combos);
};

//! @} gyro_detector

} // namespace rm
