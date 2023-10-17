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

/**
 * @brief 装甲模块组合特征
 * @note 特征包括 [0]: 左灯条，[1]: 右灯条
 * @note 角点为 [0]: 左灯条下顶点，[1]: 左灯条上顶点，[2]: 右灯条上顶点，[3]: 右灯条下顶点
 */
class Armor : public combo
{
    float _combo_ratio = 0.f;  //!< 组合特征宽高比
    float _width_ratio = 0.f;  //!< 左右灯条宽度比
    float _length_ratio = 0.f; //!< 左右灯条长度比
    float _corner_angle = 0.f; //!< 左右灯条错位角
    float _match_error = 0.f;  //!< 匹配误差

    cv::Vec2f _pose; //!< 装甲板姿态法向量

    static cv::Ptr<cv::ml::SVM> _svm; //!< 大小装甲板二分类 SVM

public:
    using ptr = std::shared_ptr<Armor>;
    using const_ptr = std::shared_ptr<const Armor>;

    Armor() = delete;
    Armor(const Armor &) = delete;
    Armor(Armor &&) = delete;

    //! @warning 禁止直接使用构造函数
    Armor(LightBlob::ptr, LightBlob::ptr, const GyroData &, double, float, float, float, float, float, float, float, ArmorSizeType);

    /**
     * @brief Armor 构造接口
     * @note 若提供参数 `armor_size_type`，则会强制构造 Armor
     *
     * @param[in] p_left 左灯条共享指针
     * @param[in] p_right 右灯条共享指针
     * @param[in] gyro_data 当前时刻组合特征对应的陀螺仪数据
     * @param[in] tick 捕获组合特征的时间点
     * @param[in] armor_size_type 需要指定的大小装甲板类型，默认为 `ArmorSizeType::UNKNOWN`
     * @return 若成功，返回 Armor 的共享指针，否则返回空
     */
    static Armor::ptr make_combo(LightBlob::ptr p_left, LightBlob::ptr p_right, const GyroData &gyro_data,
                                 double tick, ArmorSizeType armor_size_type = ArmorSizeType::UNKNOWN);

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_combo combo::ptr 抽象指针
     * @return 派生对象指针
     */
    static inline Armor::ptr cast(combo::ptr p_combo) { return std::dynamic_pointer_cast<Armor>(p_combo); }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_combo combo::const_ptr 抽象指针
     * @return 派生对象指针
     */
    static inline Armor::const_ptr cast(combo::const_ptr p_combo) { return std::dynamic_pointer_cast<const Armor>(p_combo); }

    /**
     * @brief 加载 SVM 装甲板大小分类的 *.xml 文件
     *
     * @param[in] path *.xml 文件路径
     */
    static inline void loadSVM(const std::string &path) { _svm = cv::ml::SVM::load(path); }

    /**
     * @brief 装甲板相机外参从陀螺仪坐标系转化为相机坐标系
     *
     * @param[in] gyro_rmat 陀螺仪坐标系下的装甲板旋转矩阵
     * @param[in] gyro_tvec 陀螺仪坐标系下的装甲板平移向量
     * @param[in] gyro_data 陀螺仪数据信息
     * @param[out] cam_rmat 相机坐标系下的装甲板旋转矩阵
     * @param[out] cam_tvec 相机坐标系下的装甲板平移向量
     */
    static void gyroConvertToCamera(const cv::Matx33f &gyro_rmat, const cv::Vec3f &gyro_tvec,
                                    const GyroData &gyro_data, cv::Matx33f &cam_rmat, cv::Vec3f &cam_tvec);

    /**
     * @brief 装甲板相机外参从相机坐标系转化为陀螺仪坐标系
     *
     * @param[in] cam_rmat 相机坐标系下的装甲板旋转矩阵
     * @param[in] cam_tvec 相机坐标系下的装甲板平移向量
     * @param[in] gyro_data 陀螺仪数据信息
     * @param[out] gyro_rmat 陀螺仪坐标系下的装甲板旋转矩阵
     * @param[out] gyro_tvec 陀螺仪坐标系下的装甲板平移向量
     */
    static void cameraConvertToGyro(const cv::Matx33f &cam_rmat, const cv::Vec3f &cam_tvec,
                                    const GyroData &gyro_data, cv::Matx33f &gyro_rmat, cv::Vec3f &gyro_tvec);

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
     * @param[in] p_combo 指定的参考装甲板
     * @return ROI
     */
    static cv::Mat getNumberROI(cv::Mat src, combo::ptr p_combo);

    //! 获取组合特征宽高比
    inline float getComboRatio() { return _combo_ratio; }
    //! 获取左右灯条宽度的比值
    inline float getWidthRatio() { return _width_ratio; }
    //! 获取左右灯条长度的比值
    inline float getLengthRatio() { return _length_ratio; }
    //! 获取左右灯条错位角
    inline float getCornerAngle() { return _corner_angle; }
    //! 获取匹配误差
    inline float getError() { return _match_error; }
    //! 获取装甲板大小类型
    inline ArmorSizeType getArmorType() { return _type.ArmorSizeTypeID; }
    //! 设置机器人类型 RobotType
    inline void setType(RobotType stat) { _type.RobotTypeID = stat; }
    //! 设置相机外参
    inline void setExtrinsic(const CameraExtrinsics<float> &extrinsic) { _extrinsic = extrinsic; }
    //! 获取装甲板姿态法向量
    inline const cv::Vec2f &getPose() const { return _pose; }

private:
    /**
     * @brief 获取装甲板的位姿
     *
     * @param[in] cam_matrix 相机内参，用于解算相机外参
     * @param[in] distcoeff 相机畸变参数，用于解算相机外参
     * @param[in] gyro_data 陀螺仪数据
     * @return CameraExtrinsics - 相机外参
     */
    CameraExtrinsics<float> calculateExtrinsic(const cv::Matx33f &cam_matrix, const cv::Matx51f &distcoeff, const GyroData &gyro_data);

    /**
     * @brief 用来确定装甲板的种类 (大装甲或者小装甲)
     *
     * @return Armor::ArmorType
     */
    ArmorSizeType matchArmorType();
};

inline cv::Ptr<cv::ml::SVM> Armor::_svm = nullptr;

//! @} combo_armor

} // namespace rm
