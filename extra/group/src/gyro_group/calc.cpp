/**
 * @file calc.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲序列组计算源文件
 * @version 0.1
 * @date 2023-02-10
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/calib3d.hpp>

#include "rmvl/group/gyro_group.h"
#include "rmvl/algorithm/transform.hpp"
#include "rmvl/tracker/gyro_tracker.h"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/combo/armor.h"
#include "rmvlpara/group/gyro_group.h"

namespace rm
{

using namespace numeric_literals;

int GyroGroup::calcArmorNum(const std::vector<combo::ptr> &ref_combos)
{
    // 机器人类型集合
    std::unordered_set<RobotType> robot_set;
    // 装甲板大小集合
    std::unordered_set<ArmorSizeType> armor_size_set;
    for (auto ref_combo : ref_combos)
    {
        robot_set.insert(ref_combo->type().RobotTypeID);
        armor_size_set.insert(ref_combo->type().ArmorSizeTypeID);
    }
    // 其中一个是前哨站，则 armor_num = 3
    if (robot_set.find(RobotType::OUTPOST) != robot_set.end())
        return 3;
    // 均不是英雄，且均为大装甲板：armor_num = 2
    if (armor_size_set.find(ArmorSizeType::SMALL) == armor_size_set.end() &&
        armor_size_set.find(ArmorSizeType::BIG) != armor_size_set.end() &&
        robot_set.find(RobotType::HERO) == robot_set.end())
        return 2;
    // 其余情况：armor_num = 4
    return 4;
}

void GyroGroup::calcGroupFrom3DMessage(const std::vector<cv::Vec2f> &gyro_poses, const std::vector<cv::Vec3f> &gyro_ts,
                                       const std::vector<float> &rs, cv::Vec3f &gyro_center)
{
    // --------------------- 保证三个集合元素个数相同 ---------------------
    if (gyro_ts.size() != gyro_poses.size() || gyro_ts.size() != rs.size())
        RMVL_Error(RMVL_StsBadArg, "Size of the arguments are not equal.");
    size_t combos_num = gyro_ts.size(); // 装甲板组数量
    if (combos_num != 1 && combos_num != 2)
        RMVL_Error_(RMVL_StsBadSize, "Bad number of armor plates in a combo_set: %zu", combos_num);
    if (combos_num == 1)
        gyro_center = gyro_ts[0] + rs[0] * cv::Vec3f(gyro_poses[0](0), 0, gyro_poses[0](1));
    else // combos_num == 2
    {
        gyro_center = (gyro_ts[0] + gyro_ts[1]) / 2.f;
        gyro_center(2) = (gyro_ts[0] + rs[0] * cv::Vec3f(gyro_poses[0](0), 0, gyro_poses[0](1)) +
                          gyro_ts[1] + rs[1] * cv::Vec3f(gyro_poses[1](0), 0, gyro_poses[1](1)))(2) /
                         2.f;
    }
}

combo::ptr GyroGroup::constructComboForced(combo::ptr p_combo, const ImuData &imu_data, const cv::Matx33f &gyro_rmat, const cv::Vec3f &gyro_tvec, double tick)
{
    // IMU 坐标系转化为相机坐标系
    cv::Matx33f cam_rmat;
    cv::Vec3f cam_tvec;
    Armor::imuConvertToCamera(gyro_rmat, gyro_tvec, p_combo->imu(), cam_rmat, cam_tvec);

    if (cam_tvec(2) <= 0)
        RMVL_Error_(RMVL_StsError, "Bad value of \"cam_tvec\", cam_tvec(2) = %.3f", cam_tvec(2));
    // rmat -> rvec
    cv::Vec3f cam_rvec;
    Rodrigues(cam_rmat, cam_rvec);
    std::vector<cv::Point2f> new_corners;
    projectPoints(p_combo->type().ArmorSizeTypeID == ArmorSizeType::SMALL ? para::armor_param.SMALL_ARMOR : para::armor_param.BIG_ARMOR,
                  cam_rvec, cam_tvec, para::camera_param.cameraMatrix, para::camera_param.distCoeffs, new_corners);
    if (new_corners.size() != 4)
        RMVL_Error_(RMVL_StsBadSize, "Size of the \"new_corners\" are not equal to 4. (size = %zu)", new_corners.size());
    // 强制构造灯条与装甲板
    auto left = LightBlob::make_feature(new_corners[1], new_corners[0], p_combo->at(0)->width());
    auto right = LightBlob::make_feature(new_corners[2], new_corners[3], p_combo->at(1)->width());
    return Armor::make_combo(left, right, imu_data, tick, p_combo->type().ArmorSizeTypeID);
}

void GyroGroup::getGroupInfo(const std::vector<combo::ptr> &visible_combos, std::vector<TrackerState> &state_vec,
                             std::vector<combo::ptr> &combo_vec, cv::Vec3f &group_center3d, cv::Point2f &group_center2d)
{
    size_t visible_num = visible_combos.size();
    if (visible_num != 1 && visible_num != 2)
        RMVL_Error_(RMVL_StsBadArg, "Bad size of the \"visible_combos\". (size = %zu)", visible_num);
    state_vec.resize(_armor_num);
    combo_vec.resize(_armor_num);

    std::vector<cv::Matx33f> Rs(visible_num); // 旋转矩阵
    std::vector<cv::Vec2f> Ps(visible_num);   // 姿态法向量
    std::vector<cv::Vec3f> ts(visible_num);   // 平移向量
    std::vector<float> rs(visible_num);       // 旋转半径
    // 排序，从右到左
    auto operate_combos = visible_combos;
    sort(operate_combos.begin(), operate_combos.end(), [](combo::const_ptr lhs, combo::const_ptr rhs) {
        return lhs->center().x > rhs->center().x;
    });
    for (size_t i = 0; i < visible_num; ++i)
    {
        Rs[i] = operate_combos[i]->extrinsic().R();
        Ps[i] = Armor::cast(operate_combos[i])->getPose();
        ts[i] = operate_combos[i]->extrinsic().tvec();
        rs[i] = para::gyro_group_param.INIT_RADIUS;
    }

    if (visible_num == 1)
    {
        // 唯一一个追踪器
        auto p_combo = operate_combos.front();
        calcGroupFrom3DMessage(Ps, ts, rs, group_center3d);
        float r = rs.front();
        cv::Vec3f center_tvec = group_center3d; // 车组旋转中心位置平移向量
        // 追踪器信息
        state_vec.front() = TrackerState(0, r, 0.f);
        combo_vec.front() = p_combo;
        // 旋转中心点像素坐标系坐标
        auto I = cv::Matx33f::eye();
        cv::Vec3f cam_tvec;
        Armor::imuConvertToCamera(I, center_tvec, _imu_data, I, cam_tvec);
        group_center2d = cameraConvertToPixel(para::camera_param.cameraMatrix, para::camera_param.distCoeffs, cam_tvec);

        // 2、3、4 号追踪器
        for (int i = 0; i < _armor_num - 1; i++)
        {
            // 绕 y 轴旋转 90 * (i + 1) 度 (俯视图顺时针)
            auto y_rotate = euler2Mat(static_cast<float>(2_PI / _armor_num) * (i + 1), EulerAxis::Y);
            cv::Matx33f new_R = y_rotate * Rs.front(); // IMU 坐标系下的新旋转矩阵
            // IMU 坐标系下的新装甲板到旋转中心点的方向向量
            cv::Vec3f new_pose = normalize(cv::Vec3f(new_R(0, 2), 0, new_R(2, 2)));
            cv::Vec3f new_gyro_tvec = center_tvec - new_pose * r; // IMU 坐标系下的新装甲板相机坐标系平移向量
            combo::ptr armor = constructComboForced(p_combo, _imu_data, new_R, new_gyro_tvec, _tick);
            // 追踪器信息
            combo_vec[i + 1] = armor;
            state_vec[i + 1] = TrackerState(i + 1, r, 0.f);
        }
    }
    else
    {
        calcGroupFrom3DMessage(Ps, ts, rs, group_center3d);
        for (size_t i = 0; i < 2; i++)
        {
            // 计算高度差：当前与下一块装甲板 y 轴高度差
            float delta_y = ts[i](1) - group_center3d(1);
            // 中心点的 y 定义为 2 个 combos 中间
            combo_vec[i] = operate_combos[i];
            state_vec[i] = TrackerState(i, rs[i], delta_y);
        }
        const cv::Vec3f &center_tvec = group_center3d;
        // 旋转中心点像素坐标系坐标
        auto I = cv::Matx33f::eye();
        cv::Vec3f cam_tvec;
        Armor::imuConvertToCamera(I, center_tvec, _imu_data, I, cam_tvec);
        group_center2d = cameraConvertToPixel(para::camera_param.cameraMatrix, para::camera_param.distCoeffs, center_tvec);
        // 第三、四块装甲板
        for (int i = 0; i < _armor_num - 2; i++)
        {
            // 绕 y 轴旋转 180 * (i + 1) 度 (俯视图顺时针)
            cv::Matx33f y_rotate = cv::Matx33f::diag({-1, 0, -1});
            cv::Matx33f new_R = y_rotate * Rs[i];
            // 新装甲板到旋转中心点的方向向量
            cv::Vec3f new_pose = normalize(cv::Vec3f(new_R(0, 2), 0, new_R(2, 2)));
            cv::Vec3f new_tvec = center_tvec - new_pose * rs[i]; // 新装甲板平移向量
            combo::ptr armor = constructComboForced(operate_combos[i], _imu_data, new_R, new_tvec, _tick);
            // 组合体信息
            combo_vec[i + 2] = armor;
            state_vec[i + 2] = TrackerState(i + 2, rs[i], state_vec[i].delta_y());
        }
    }
}

void GyroGroup::updateRotStatus()
{
    // 当前为低速状态
    if (_rot_status == RotStatus::LOW_ROT_SPEED)
    {
        if (abs(_rotspeed) > para::gyro_group_param.MIN_HIGH_ROT_SPEED)
            _rot_status = RotStatus::HIGH_ROT_SPEED;
    }
    // 当前为高速状态
    else
    {
        if (abs(_rotspeed) < para::gyro_group_param.MAX_LOW_ROT_SPEED)
            _rot_status = RotStatus::LOW_ROT_SPEED;
    }
}

} // namespace rm
