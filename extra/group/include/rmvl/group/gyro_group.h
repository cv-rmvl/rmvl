/**
 * @file gyro_group.h
 * @author RoboMaster Vision Community
 * @brief 装甲序列组头文件
 * @version 0.1
 * @date 2022-12-10
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <unordered_set>

#include "rmvl/algorithm/kalman.hpp"
#include "rmvl/group/group.h"

namespace rm
{

//! 序列组中的追踪器状态
class TrackerState
{
    std::size_t _index = 0; //!< 在车组中追踪器的序号
    float _radius = 0.f;    //!< 旋转半径
    float _delta_y = 0.f;   //!< 与旋转中心的高度差（向下为正）

public:
    TrackerState() = default;

    /**
     * @brief 追踪器信息结构体直接初始化
     *
     * @param[in] index 在车组中追踪器的序号
     * @param[in] radius 旋转半径
     * @param[in] delta_y 与旋转中心的高度差（向下为正）
     */
    TrackerState(int index, float radius, float delta_y) : _index(index), _radius(radius), _delta_y(delta_y) {}

    //! 获取在车组中追踪器的序号
    inline size_t index() const { return _index; }
    //! 获取旋转半径
    inline float radius() const { return _radius; }
    //! 与旋转中心的高度差（向下为正）
    inline float delta_y() const { return _delta_y; }
    //! 更新在车组中追踪器的序号
    inline void index(int idx) { _index = idx; }
    //! 更新旋转半径
    inline void radius(float r) { _radius = r; }
    //! 更新与旋转中心的高度差（向下为正）
    inline void delta_y(float dy) { _delta_y = dy; }
};

//! 旋转状态类型
enum class RotStatus : uint8_t
{
    LOW_ROT_SPEED,  //!< 低转速
    HIGH_ROT_SPEED, //!< 高转速
};

//! @addtogroup gyro_group
//! @{

//! 整车状态序列组
class RMVL_EXPORTS_W_DES GyroGroup final : public group
{
public:
    using ptr = std::shared_ptr<GyroGroup>;
    using const_ptr = std::shared_ptr<const GyroGroup>;

    //! @cond
    GyroGroup(const std::vector<combo::ptr> &combos, int armor_num);
    //! @endcond

    /**
     * @brief 根据第一帧识别到的组合体构建 GyroGroup
     * @note 组合体列表大小仅允许 1 或 2
     *
     * @param[in] first_combos 组合体列表
     * @param[in] armor_num 强制指定装甲板个数（小于 1 表示自动判断）
     */
    RMVL_W static inline ptr make_group(const std::vector<combo::ptr> &first_combos, int armor_num)
    {
        if (first_combos.size() != 1 && first_combos.size() != 2)
            return nullptr;
        return std::make_shared<GyroGroup>(first_combos, armor_num);
    }

    /**
     * @brief 从另一个序列组进行构造
     *
     * @return 指向新序列组的共享指针
     */
    RMVL_W group::ptr clone() override;

    //! 判断是否为无效序列组
    RMVL_W bool invalid() const override;

    RMVL_GROUP_CAST(GyroGroup)

    /**
     * @brief 根据 IMU 坐标系下装甲板外参信息和默认初始半径集合，得到修正的半径集合、序列组中心坐标、装甲板法向量集合
     *
     * @param[in] gyro_poses 装甲板在相机坐标系下的姿态法向量集合 pose
     * @param[in] gyro_ts 装甲板在相机坐标系下的平移向量集合 t
     * @param[in] rs 装甲板旋转半径集合 r（传入初始的旋转半径）
     * @param[out] gyro_center 装甲板序列组在 IMU 坐标系下中心坐标
     */
    static void calcGroupFrom3DMessage(const std::vector<cv::Vec2f> &gyro_poses, const std::vector<cv::Vec3f> &gyro_ts,
                                       const std::vector<float> &rs, cv::Vec3f &gyro_center);

    /**
     * @brief 强制构建 combo
     *
     * @param[in] p_combo 指定参考的组合体
     * @param[in] imu_data 最新 IMU 数据
     * @param[in] gyro_rmat IMU 坐标系下装甲板的旋转矩阵
     * @param[in] gyro_tvec IMU 坐标系下装甲板的平移向量
     * @param[in] tick 当前时间点数据
     * @return 强制构建的 combo
     */
    static combo::ptr constructComboForced(combo::ptr p_combo, const ImuData &imu_data,
                                           const cv::Matx33f &gyro_rmat, const cv::Vec3f &gyro_tvec, double tick);

    /**
     * @brief 计算装甲板数目
     *
     * @param[in] ref_combos 指定参与计算的装甲板
     * @return 装甲板数目
     */
    static int calcArmorNum(const std::vector<combo::ptr> &ref_combos);

    /**
     * @brief GyroGroup 同步操作
     *
     * @param[in] imu_data 最新 IMU 数据
     * @param[in] tick 最新时间点
     */
    RMVL_W void sync(const ImuData &imu_data, double tick) override;

    /**
     * @brief 获取追踪器状态
     *
     * @param[in] p_tracker 追踪器指针
     * @return 追踪器信状态
     */
    TrackerState getTrackerState(tracker::ptr p_tracker);

    //! 获取最新的车组旋转中心点
    RMVL_W inline cv::Vec3f getCenter3D() const { return _center3d; }
    //! 获取旋转中心点移动速度
    RMVL_W inline cv::Vec3f getSpeed3D() const { return _speed3d; }
    //! 获取相机坐标系自转角速度（俯视顺时针为正，滤波数据，弧度）
    RMVL_W inline float getRotatedSpeed() const { return _rotspeed; }
    //! 获取 IMU 数据
    RMVL_W inline ImuData imu() const { return _imu_data; }
    //! 获取旋转状态
    RMVL_W inline RotStatus getRotStatus() const { return _rot_status; }

private:
    /**
     * @brief 初始化时获取序列组信息，并完成强制构造
     *
     * @param[in] visible_combos 可见的组合体列表
     * @param[out] state_vec 追踪器信息列表
     * @param[out] combo_vec 整车模型中的组合体列表，下标与 state_vec 的下标保持一致
     * @param[out] group_center3d 旋转中心点相机坐标
     * @param[out] group_center2d 旋转中心点像素坐标
     */
    void getGroupInfo(const std::vector<combo::ptr> &visible_combos, std::vector<TrackerState> &state_vec,
                      std::vector<combo::ptr> &combo_vec, cv::Vec3f &group_center3d, cv::Point2f &group_center2d);

    /**
     * @brief 初始化序列组
     *
     * @param[in] visible_combo_set 可见组合体集合
     * @param[in] state_vec 所有追踪器信息列表（包括构建出的组合体）
     * @param[in] combo_vec 所有组合体列表（包括构建出的组合体）
     * @param[in] group_center3d 旋转中心点相机坐标
     * @param[in] group_center2d 旋转中心点像素坐标
     */
    void initGroup(const std::unordered_set<combo::ptr> &visible_combo_set, const std::vector<TrackerState> &state_vec,
                   const std::vector<combo::ptr> &combo_vec, const cv::Vec3f &group_center3d,
                   const cv::Point2f &group_center2d);

    /**
     * @brief 更新旋转中心滤波器
     *
     * @param[in] center 旋转中心坐标（原始数据）
     * @param[in] t 帧差时间
     * @param[out] update_center 旋转中心坐标（滤波数据）
     * @param[out] update_speed 旋转中心速度（滤波数据）
     */
    void updateCenter3DFilter(cv::Vec3f center, float t, cv::Vec3f &update_center, cv::Vec3f &update_speed);

    /**
     * @brief 更新旋转速度滤波器
     *
     * @param[in] rotspeed 旋转速度（原始数据）
     * @param[in] update_rotspeed 旋转速度（滤波数据）
     */
    void updateRotationFilter(float rotspeed, float &update_rotspeed);

    //! 初始化滤波器
    void initFilter();

    //! 更新group的旋转状态
    void updateRotStatus();

    double _tick{};      //!< 时间点
    ImuData _imu_data{}; //!< 当前 IMU 数据
    bool _is_tracked{};  //!< 是否为目标序列组

    //! 追踪器状态哈希表 [追踪器 : 追踪器状态]
    std::unordered_map<tracker::ptr, TrackerState> _tracker_state{};

    KF63f _center3d_filter{}; //!< 旋转中心点位置滤波器

    std::deque<float> _rotspeed_deq{}; //!< 旋转速度时间队列（原始数据）

    cv::Vec3f _center3d{}; //!< IMU 坐标系下旋转中心点坐标（滤波数据）
    cv::Vec3f _speed3d{};  //!< 旋转中心点平移速度（滤波数据）
    float _rotspeed = 0.f; //!< 绕 y 轴自转角速度（俯视顺时针为正，滤波数据，弧度）

    RotStatus _rot_status = RotStatus::LOW_ROT_SPEED; //!< 当前旋转状态

    int _armor_num = 4; //!< 装甲板数量
};

//! @} gyro_group

} // namespace rm
