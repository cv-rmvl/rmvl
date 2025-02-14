/**
 * @file armor.h
 * @author RoboMaster Vision Community
 * @brief 装甲板类头文件
 * @version 1.0
 * @date 2021-08-13
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <opencv2/ml.hpp>

#include "combo.h"
#include "rmvl/feature/light_blob.h"

namespace rm
{

//! @addtogroup combo_armor
//! @{

//! @example samples/detector/mv/sample_mv_armor_size_classify.cpp 大小装甲板分类例程
//! @example samples/detector/hik/sample_hik_armor_size_classify.cpp 大小装甲板分类例程

//! 装甲板大小类型
enum class ArmorSizeType : uint8_t
{
    UNKNOWN, //!< 未知
    SMALL,   //!< 小装甲板
    BIG      //!< 大装甲板
};

//! 机器人类型
enum class RobotType : uint8_t
{
    UNKNOWN,    //!< 未知
    HERO,       //!< 英雄机器人
    ENGINEER,   //!< 工程机器人
    INFANTRY_3, //!< 3 号步兵机器人
    INFANTRY_4, //!< 4 号步兵机器人
    INFANTRY_5, //!< 5 号步兵机器人
    OUTPOST,    //!< 前哨站
    BASE,       //!< 基地
    SENTRY      //!< 哨兵机器人
};

/**
 * @brief 装甲模块组合特征
 * @note
 * - 特征包括 `[0]`: 左灯条，`[1]`: 右灯条
 * - 角点为 `[0]`: 左灯条下顶点，`[1]`: 左灯条上顶点，`[2]`: 右灯条上顶点，`[3]`: 右灯条下顶点
 */
class RMVL_EXPORTS_W_DES Armor final : public combo
{
public:
    using ptr = std::shared_ptr<Armor>;
    using const_ptr = std::shared_ptr<const Armor>;

    //! @cond
    Armor() = default;
    Armor(float, float, float, float);
    //! @endcond

    /**
     * @brief Armor 构造接口
     * @note 若提供具体的 `armor_size_type` 参数，则不进行可行性验证，直接强制构造
     *
     * @param[in] p_left 左灯条共享指针
     * @param[in] p_right 右灯条共享指针
     * @param[in] imu_data 当前时刻组合特征对应的 IMU 数据
     * @param[in] tick 捕获组合特征的时间点
     * @param[in] armor_size_type 需要指定的大小装甲板类型，默认为 `ArmorSizeType::UNKNOWN`
     * @return 若成功，返回 Armor 的共享指针，否则返回空
     */
    RMVL_W static ptr make_combo(LightBlob::ptr p_left, LightBlob::ptr p_right, const ImuData &imu_data, double tick, ArmorSizeType armor_size_type = ArmorSizeType::UNKNOWN);

    /**
     * @brief 从另一个组合体进行构造
     *
     * @param[in] tick 当前时间点，可用 `rm::Timer::now()` 获取
     * @return 指向新组合体的共享指针
     */
    RMVL_W combo::ptr clone(double tick) override;

    RMVL_COMBO_CAST(Armor)

    /**
     * @brief 加载 SVM 装甲板大小分类的 *.xml 文件
     *
     * @param[in] path *.xml 文件路径
     */
    RMVL_W static inline void loadSVM(const std::string &path) { _svm = cv::ml::SVM::load(path); }

    /**
     * @brief 装甲板相机外参从 IMU 坐标系转化为相机坐标系
     *
     * @param[in] gyro_rmat IMU 坐标系下的装甲板旋转矩阵
     * @param[in] gyro_tvec IMU 坐标系下的装甲板平移向量
     * @param[in] imu_data IMU 数据信息
     * @param[out] cam_rmat 相机坐标系下的装甲板旋转矩阵
     * @param[out] cam_tvec 相机坐标系下的装甲板平移向量
     */
    static void imuConvertToCamera(const cv::Matx33f &gyro_rmat, const cv::Vec3f &gyro_tvec,
                                   const ImuData &imu_data, cv::Matx33f &cam_rmat, cv::Vec3f &cam_tvec);

    /**
     * @brief 装甲板相机外参从相机坐标系转化为 IMU 坐标系
     *
     * @param[in] cam_rmat 相机坐标系下的装甲板旋转矩阵
     * @param[in] cam_tvec 相机坐标系下的装甲板平移向量
     * @param[in] imu_data IMU 数据信息
     * @param[out] gyro_rmat IMU 坐标系下的装甲板旋转矩阵
     * @param[out] gyro_tvec IMU 坐标系下的装甲板平移向量
     */
    static void cameraConvertToImu(const cv::Matx33f &cam_rmat, const cv::Vec3f &cam_tvec,
                                   const ImuData &imu_data, cv::Matx33f &gyro_rmat, cv::Vec3f &gyro_tvec);

    /**
     * @brief 判断单个装甲板所在区域内是否包含指定的灯条
     *
     * @param[in] blob 指定的灯条
     * @param[in] armor 单一装甲板
     * @return 是否包含灯条中心
     */
    static bool isContainBlob(LightBlob::ptr blob, Armor::ptr armor);

    /**
     * @brief 根据图像中指定装甲板的信息，截取仅包含数字的 ROI
     *
     * @param[in] src 输入图像
     * @param[in] p_armor 指定的参考装甲板
     * @return ROI
     */
    static cv::Mat getNumberROI(cv::Mat src, const_ptr p_armor);

    //! 获取组合特征宽高比
    RMVL_W float getComboRatio() { return _combo_ratio; }
    //! 获取左右灯条宽度的比值
    RMVL_W float getWidthRatio() { return _width_ratio; }
    //! 获取左右灯条长度的比值
    RMVL_W float getLengthRatio() { return _length_ratio; }
    //! 获取左右灯条错位角
    RMVL_W float getCornerAngle() { return _corner_angle; }
    //! 获取匹配误差
    RMVL_W float getError() { return _match_error; }
    //! 设置机器人类型 RobotType
    RMVL_W void setType(RobotType stat);
    //! 获取装甲板姿态法向量
    RMVL_W const cv::Vec2f &getPose() const { return _pose; }

private:
    float _combo_ratio{};  //!< 组合特征宽高比
    float _width_ratio{};  //!< 左右灯条宽度比
    float _length_ratio{}; //!< 左右灯条长度比
    float _corner_angle{}; //!< 左右灯条错位角
    float _match_error{};  //!< 匹配误差

    cv::Vec2f _pose; //!< 装甲板姿态法向量

    static cv::Ptr<cv::ml::SVM> _svm; //!< 大小装甲板二分类 SVM
};

inline cv::Ptr<cv::ml::SVM> Armor::_svm = nullptr;

/**
 * @brief 装甲板大小类型转为字符串
 *
 * @param[in] armor_size 装甲板大小类型
 */
const char *to_string(ArmorSizeType armor_size);

/**
 * @brief StateType 转为装甲板大小类型
 *
 * @param[in] tp StateType 类型
 */
ArmorSizeType to_armor_size_type(const StateType &tp);

/**
 * @brief 机器人类型转为字符串
 *
 * @param[in] robot 机器人类型
 */
const char *to_string(RobotType robot);

/**
 * @brief StateType 转为机器人类型
 *
 * @param[in] tp StateType 类型
 */
RobotType to_robot_type(const StateType &tp);

//! @} combo_armor

} // namespace rm
