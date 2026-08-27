/**
 * @file localization.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 视觉里程计与 SLAM 接入
 * @version 1.0
 * @date 2026-08-24
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "rmvlmsg/geometry/transform_stamped.hpp"
#include "rmvlmsg/nav/occupancy_grid.hpp"
#include "rmvlmsg/nav/occupancy_grid_update.hpp"
#include "rmvlmsg/nav/odometry.hpp"
#include "rmvlmsg/nav/path.hpp"
#include "rmvlmsg/sensor/camera_info.hpp"
#include "rmvlmsg/sensor/image.hpp"
#include "rmvlmsg/sensor/imu.hpp"

namespace rm::nav {

//! @defgroup nav_localization 视觉里程计与 SLAM 接入
//! @ingroup nav
//! @{
//! @brief 提供通用传感器帧同步、可替换定位后端和标准导航输出

//! 相机输入组合
enum class CameraMode : uint8_t {
    Monocular, //!< 单目图像，只保证相对尺度或依赖其他米制观测
    Stereo,    //!< 左右目图像
    Rgbd,      //!< 彩色或灰度主图像与深度图像
};

//! 视觉定位数据流状态
enum class LocalizationStatus : uint8_t {
    Ok,                  //!< 已生成有效定位结果
    WaitingForPrimary,   //!< 正在等待主图像
    WaitingForSecondary, //!< 正在等待右目或深度图像
    MissingCalibration,  //!< 缺少相机标定信息
    InvalidOptions,      //!< 同步或定位配置无效
    InvalidSensorData,   //!< 图像、标定、IMU 或外部里程计数据无效
    FrameMismatch,       //!< 图像与标定信息的坐标系或尺寸不一致
    TimestampMismatch,   //!< 输入时间戳乱序或无法在容差内组成一帧
    Initializing,        //!< 后端正在初始化
    TrackingLost,        //!< 后端暂时丢失跟踪
    BackendError,        //!< 第三方后端处理失败
    InvalidEstimate,     //!< 后端返回的位姿、速度或坐标系无效
    Unsupported,         //!< 后端不支持请求的可选操作
};

/**
 * @brief 获取视觉定位状态的稳定文本描述
 *
 * @param[in] status 视觉定位状态
 * @return 指向静态文本的字符串指针
 */
const char *to_string(LocalizationStatus status) noexcept;

/**
 * @brief 传感器帧同步配置
 * @note @p tolerance 必须非负，三个队列容量必须大于 0，否则同步器返回
 * LocalizationStatus::InvalidOptions 。
 */
struct FrameSyncOptions {
    CameraMode mode{CameraMode::Rgbd};                                 //!< 相机输入组合
    std::chrono::nanoseconds tolerance{std::chrono::milliseconds(10)}; //!< 图像及外部里程计最大时间差
    std::size_t image_queue_size{10};                                  //!< 每路图像最大缓存数
    std::size_t imu_queue_size{400};                                   //!< IMU 最大缓存数
    std::size_t odometry_queue_size{50};                               //!< 外部里程计最大缓存数
};

/**
 * @brief 已同步的一帧视觉定位输入
 * @details
 * - 单目模式只填写 @p primary 和 @p primary_info
 * - 双目和 RGB-D 模式同时填写 @p secondary 和 @p secondary_info
 * - @p imu 覆盖上一成功视觉帧之后且不晚于当前主图像的样本
 * - @p auxiliary_odometry 为同步容差内与主图像采集时间最接近的外部里程计
 */
struct SensorFrame {
    CameraMode mode{CameraMode::Rgbd};                 //!< 相机输入组合
    std::shared_ptr<const msg::Image> primary{};       //!< 主图像，单目和 RGB-D 通常为彩色或灰度图
    std::shared_ptr<const msg::Image> secondary{};     //!< Stereo 为右目图，RGB-D 为深度图
    msg::CameraInfo primary_info{};                    //!< 主相机标定信息
    std::optional<msg::CameraInfo> secondary_info{};   //!< 右目或深度相机标定信息
    std::vector<msg::Imu> imu{};                       //!< 上一视觉帧之后且不晚于本帧的 IMU 样本
    std::optional<msg::Odometry> auxiliary_odometry{}; //!< 与本帧最接近的轮速计或其他外部里程计

    //! @return 本帧采集时间，以主图像时间戳为准，无主图像时返回零时间
    const msg::Time &stamp() const noexcept;
};

//! 帧同步结果
struct FrameSyncResult {
    LocalizationStatus status{LocalizationStatus::WaitingForPrimary}; //!< 同步状态
    SensorFrame frame{};                                              //!< 成功时得到的传感器帧

    //! @return 是否得到完整且有效的传感器帧
    explicit operator bool() const noexcept { return status == LocalizationStatus::Ok; }
};

//! 帧同步统计
struct FrameSyncStatistics {
    std::size_t received_primary{};   //!< 收到的有效主图像数
    std::size_t received_secondary{}; //!< 收到的有效辅助图像数
    std::size_t matched{};            //!< 成功组成的传感器帧数
    std::size_t dropped_primary{};    //!< 因过期、乱序或队列溢出丢弃的主图像数
    std::size_t dropped_secondary{};  //!< 因过期或队列溢出丢弃的辅助图像数
    std::size_t rejected{};           //!< 因消息或同步配置无效而拒绝的输入数
};

/**
 * @brief 多传感器采集时间同步器
 * @details
 * - 使用消息的 `header.stamp` 同步，不使用不可预测的到达时间
 * - 单目只等待主图像，双目和 RGB-D 在容差内选择时间最接近的辅助图像
 * - CameraInfo 作为低频标定状态缓存，IMU 按相邻视觉帧时间区间批量交给后端
 * - 外部 Odometry 为可选输入，可承载轮速计、机械里程计或融合结果
 * - 图像必须紧密排列，尺寸和坐标系必须与对应 CameraInfo 一致
 * - 队列溢出、时间戳乱序和已经无法匹配的图像会被丢弃并计入统计
 * @note 本类线程安全，可直接在不同 LPSS 订阅回调中写入各路消息。
 * @note CameraInfo 的时间戳不参与逐帧同步，更新后持续用于后续图像，直至被下一份有效标定替换。
 */
class FrameSynchronizer {
public:
    /**
     * @brief 创建传感器帧同步器
     *
     * @param[in] options 同步配置
     */
    explicit FrameSynchronizer(FrameSyncOptions options = {});

    //! @cond

    ~FrameSynchronizer();
    FrameSynchronizer(FrameSynchronizer &&) noexcept;
    FrameSynchronizer &operator=(FrameSynchronizer &&) noexcept;

    FrameSynchronizer(const FrameSynchronizer &) = delete;
    FrameSynchronizer &operator=(const FrameSynchronizer &) = delete;

    //! @endcond

    //! @return 创建同步器时传入且不会再修改的同步配置
    const FrameSyncOptions &options() const noexcept;

    /**
     * @brief 更新主相机标定信息
     *
     * @param[in] info 主相机 CameraInfo
     * @return 输入状态，失败时保留原标定信息
     */
    LocalizationStatus setPrimaryInfo(const msg::CameraInfo &info);

    /**
     * @brief 更新右目或深度相机标定信息
     *
     * @param[in] info 辅助相机 CameraInfo
     * @return 输入状态，失败时保留原标定信息
     */
    LocalizationStatus setSecondaryInfo(const msg::CameraInfo &info);

    /**
     * @brief 写入主图像
     *
     * @param[in] image 主图像消息，可使用移动语义避免复制像素数据
     * @return 输入状态，无效图像或无效同步配置会被拒绝并计入统计
     */
    LocalizationStatus pushPrimary(msg::Image image);

    /**
     * @brief 零拷贝写入由调用方共享所有权的主图像
     *
     * @param[in] image 主图像共享指针
     * @return 输入状态，空指针、无效图像或无效同步配置会被拒绝并计入统计
     * @note 本函数只复制共享指针，不复制图像数据，调用方不得在写入后修改消息内容。
     */
    LocalizationStatus pushPrimary(std::shared_ptr<const msg::Image> image);

    /**
     * @brief 写入右目或深度图像
     *
     * @param[in] image 辅助图像消息，可使用移动语义避免复制像素数据
     * @return 输入状态，无效图像或无效同步配置会被拒绝并计入统计
     */
    LocalizationStatus pushSecondary(msg::Image image);

    /**
     * @brief 零拷贝写入由调用方共享所有权的右目或深度图像
     *
     * @param[in] image 辅助图像共享指针
     * @return 输入状态，空指针、无效图像或无效同步配置会被拒绝并计入统计
     * @note 本函数只复制共享指针，不复制图像数据，调用方不得在写入后修改消息内容。
     */
    LocalizationStatus pushSecondary(std::shared_ptr<const msg::Image> image);

    /**
     * @brief 写入一个可选 IMU 样本
     *
     * @param[in] imu IMU 消息
     * @return 输入状态
     */
    LocalizationStatus pushImu(msg::Imu imu);

    /**
     * @brief 写入一个可选外部里程计样本
     *
     * @param[in] odometry 轮速计或其他外部 Odometry 消息
     * @return 输入状态
     */
    LocalizationStatus pushOdometry(msg::Odometry odometry);

    /**
     * @brief 尝试获取下一帧已同步输入
     *
     * @return 同步结果，尚未收齐输入时返回对应等待状态
     * @note 返回 TimestampMismatch 或 FrameMismatch 时可能已经丢弃一张无法组成有效帧的图像，
     * 调用方可继续调用本函数处理后续数据。
     */
    FrameSyncResult next();

    //! @return 当前累计统计的线程安全快照
    FrameSyncStatistics statistics() const noexcept;

    //! 清空消息队列与上一帧时间，保留标定信息和统计
    void clear() noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

/**
 * @brief 第三方定位后端返回的统一估计
 * @details
 * - @p pose 表示 @p child_frame 在 @p reference_frame 中的位姿
 * - @p twist 表示使用 @p child_frame 表达的速度
 * - 位姿、速度和协方差必须为有限值，位姿四元数会由 LocalizationPipeline 归一化
 * - 可选 @p map_to_odom 的子坐标系必须等于 @p reference_frame
 *
 * 当 @p status 不是 LocalizationStatus::Ok 时，Pipeline 只保留状态和 @p detail ，忽略其余字段。
 */
struct BackendEstimate {
    LocalizationStatus status{LocalizationStatus::BackendError}; //!< 跟踪状态
    std::string detail{};                                        //!< 可选诊断说明
    std::string reference_frame{"odom"};                         //!< 位姿参考坐标系，通常为 odom 或 map
    std::string child_frame{"base_link"};                        //!< 被跟踪坐标系，导航场景通常为 base_link
    msg::PoseWithCovariance pose{};                              //!< child_frame 在 reference_frame 中的位姿
    msg::TwistWithCovariance twist{};                            //!< child_frame 表达的速度
    std::optional<msg::TransformStamped> map_to_odom{};          //!< SLAM 回环或重定位产生的可选 map -> odom 修正
};

/**
 * @brief 可替换的视觉里程计后端
 * @details 具体实现可封装相机 SDK、公开数据集读取器或第三方 VO/SLAM 库。后端负责图像处理、
 * 相机到机器人本体的外参换算以及可选的 IMU、轮速融合。基础 Nav 不提供内置 VO/SLAM 算法。
 */
class OdometryBackend {
public:
    virtual ~OdometryBackend() = default;

    /**
     * @brief 处理一帧已同步传感器输入
     *
     * @param[in] frame 已同步的图像、标定和可选运动观测
     * @return 里程计估计
     */
    virtual BackendEstimate track(const SensorFrame &frame) = 0;

    //! 清除后端内部轨迹、关键帧和融合状态
    virtual void reset() = 0;
};

/**
 * @brief 可重定位的 SLAM 后端
 * @details SLAM 后端仍通过 track() 输出连续局部位姿，可额外在 BackendEstimate::map_to_odom 中提供全局修正。
 * @note relocalize() 的默认实现返回 LocalizationStatus::Unsupported 。当前 LocalizationPipeline
 * 不转发重定位请求，应用层应通过具体后端对象或独立服务完成该调用。
 */
class SlamBackend : public OdometryBackend {
public:
    /**
     * @brief 请求在全局坐标系中重定位
     *
     * @param[in] pose 初始位姿及其协方差
     * @param[in] frame_id 初始位姿参考坐标系，通常为 map
     * @return 请求状态，不支持时返回 LocalizationStatus::Unsupported
     */
    virtual LocalizationStatus relocalize(const msg::PoseWithCovariance &pose, std::string_view frame_id);
};

/**
 * @brief Mapper 后端返回的二维导航地图更新
 * @details @p map 和 @p update 可以分别表示全量地图与其后的矩形增量。若同一结果同时包含二者，
 * 消费方应先装载全量地图，再按版本顺序应用增量。
 * @note LocalizationPipeline 不校验地图尺寸、坐标系和版本，消费方应使用 GridMap 完成校验。
 */
struct MappingResult {
    LocalizationStatus status{LocalizationStatus::Unsupported}; //!< 建图状态
    std::string detail{};                                       //!< 可选诊断说明
    std::optional<msg::OccupancyGrid> map{};                    //!< 可选全量占据栅格
    std::optional<msg::OccupancyGridUpdate> update{};           //!< 可选矩形增量
};

/**
 * @brief 可替换的深度地图后端
 * @details Mapper 负责将 SensorFrame 中的深度观测转换为导航可消费的二维占据栅格。
 * 定位用稀疏特征地图不能直接当作障碍地图。需要接入点云时，由具体 Mapper 自行接收和同步点云，
 * 当前 SensorFrame 尚未提供通用点云消息。
 */
class MapperBackend {
public:
    virtual ~MapperBackend() = default;

    /**
     * @brief 使用当前传感器帧和定位结果更新地图
     *
     * @param[in] frame 已同步传感器输入
     * @param[in] odometry 当前定位结果
     * @return 全量地图或增量更新
     */
    virtual MappingResult update(const SensorFrame &frame, const msg::Odometry &odometry) = 0;

    //! 清除后端内部地图和累积状态
    virtual void reset() = 0;
};

//! 定位输出整理配置
struct LocalizationOptions {
    std::size_t path_capacity{1000}; //!< 保留的最近轨迹点数，设为 0 时不生成 Path
};

/**
 * @brief 一帧视觉里程计或 SLAM 的标准导航输出
 * @note operator bool() 只表示 Odometry 和动态 TF 有效。配置 Mapper 时仍应单独检查
 * @p mapping 内部的 MappingResult::status 。
 */
struct LocalizationResult {
    LocalizationStatus status{LocalizationStatus::BackendError}; //!< 定位状态
    std::string detail{};                                        //!< 可选诊断说明
    msg::Odometry odometry{};                                    //!< 标准里程计消息
    msg::TransformStamped transform{};                           //!< 与 odometry 等价的动态 TF
    std::optional<msg::TransformStamped> map_to_odom{};          //!< 可选全局修正 TF
    msg::Path path{};                                            //!< 有界历史轨迹
    std::optional<MappingResult> mapping{};                      //!< 配置 Mapper 时的建图结果

    //! @return 是否生成有效 Odometry 和 TF
    explicit operator bool() const noexcept { return status == LocalizationStatus::Ok; }
};

/**
 * @brief 将第三方定位结果转换为 RMVL 导航消息
 * @details
 * - 成功时统一使用主图像采集时间生成 Odometry、等价动态 TF 和有界 Path
 * - 后端返回的位姿四元数以及可选全局修正四元数会被归一化
 * - 参考坐标系改变时清空历史 Path，@p path_capacity 为 0 时不生成 Path
 * - 可选 Mapper 在定位成功后运行，其失败不会使已经生成的定位结果失效
 * - 后端抛出的异常转换为 BackendError，Mapper 抛出的异常只记录在 MappingResult 中
 *
 * 调用方可将结果通过 LPSS Publisher 与 lpss::tf::Broadcaster 发布。
 * @note 本类串行调用后端，即使多个订阅回调同时调用 process() 也不会并发进入第三方实现。
 */
class LocalizationPipeline {
public:
    /**
     * @brief 创建视觉定位流水线
     *
     * @param[in] backend 视觉里程计或 SLAM 后端，所有权转移至 Pipeline，允许为空
     * @param[in] mapper 可选地图后端，所有权转移至 Pipeline
     * @param[in] options 输出整理配置
     * @note 空后端允许构造，但 process() 将返回 LocalizationStatus::BackendError 。
     */
    explicit LocalizationPipeline(std::unique_ptr<OdometryBackend> backend,
                                  std::unique_ptr<MapperBackend> mapper = {},
                                  LocalizationOptions options = {});

    ~LocalizationPipeline();
    LocalizationPipeline(LocalizationPipeline &&) noexcept;
    LocalizationPipeline &operator=(LocalizationPipeline &&) noexcept;

    LocalizationPipeline(const LocalizationPipeline &) = delete;
    LocalizationPipeline &operator=(const LocalizationPipeline &) = delete;

    /**
     * @brief 处理一帧同步输入并生成标准导航输出
     *
     * @param[in] frame 已同步传感器帧
     * @return 定位、TF、轨迹和可选地图结果
     */
    LocalizationResult process(const SensorFrame &frame);

    //! 重置定位后端、可选地图后端和历史轨迹，后端 reset() 抛出的异常会向调用方传播
    void reset();

    //! @return 当前输出整理配置
    const LocalizationOptions &options() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

//! @} nav_localization

} // namespace rm::nav
