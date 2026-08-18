/**
 * @file map.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 二维占据栅格与分层代价地图
 * @version 1.0
 * @date 2026-08-16
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "rmvlmsg/geometry/pose.hpp"
#include "rmvlmsg/nav/occupancy_grid.hpp"
#include "rmvlmsg/nav/occupancy_grid_update.hpp"

namespace rm::nav {

//! @defgroup nav_map 二维栅格地图与代价地图
//! @ingroup nav
//! @{
//! @brief 提供占据栅格维护、坐标转换、局部观测融合、障碍膨胀和 footprint 碰撞检测

//! 地图操作状态
enum class MapStatus : uint8_t {
    Ok,                 //!< 操作成功
    InvalidResolution,  //!< 分辨率不是有限正数
    InvalidDimensions,  //!< 地图或更新区域尺寸无效、溢出或数据长度不匹配
    InvalidOrigin,      //!< 地图原点不是有效的平面位姿
    InvalidValue,       //!< 栅格值不在 \f$[-1,\,100]\f$ 范围
    InvalidProbability, //!< 概率不是 \f$[0,\,1]\f$ 范围内的有限值
    InvalidOptions,     //!< 代价地图配置无效
    InvalidRevision,    //!< 新版本号未严格递增或本地版本已溢出
    RevisionMismatch,   //!< 增量基线版本与当前地图不一致
    FrameMismatch,      //!< 增量与地图不属于同一坐标系
    GeometryMismatch,   //!< 静态层与现有代价地图的尺寸、分辨率或原点不一致
    OutOfBounds,        //!< 坐标、区域或射线不在地图范围内
    InvalidFootprint,   //!< footprint 或机器人位姿无效
};

/**
 * @brief 获取地图状态的稳定文本描述
 *
 * @param[in] status 地图操作状态
 * @return 指向静态文本的字符串指针
 */
const char *to_string(MapStatus status) noexcept;

//! 离散栅格坐标
struct Cell {
    uint32_t x{}; //!< 横向索引
    uint32_t y{}; //!< 纵向索引
};

/**
 * @brief 二维占据栅格内存模型
 * @details
 * - 数据采用 row-major 布局，索引为 `y * width + x`
 * - 栅格值 `-1` 表示未知，`0` 表示空闲，`100` 表示占据
 * - 世界坐标转换支持带平移和偏航角的地图原点
 * - 每次成功且实际改变地图内容的本地 API 调用视为一次修改，版本递增 1
 * - 一次调用即使修改多个栅格也只递增一次版本
 * - 合并矩形增量时采用消息携带的目标版本
 * @note 该类不提供内部同步。跨线程访问时由调用方加锁。
 */
class GridMap {
public:
    //! 构造空地图
    GridMap();
    /**
     * @brief 使用全量占据栅格构造地图
     *
     * @param[in] grid 全量占据栅格消息
     * @remark 输入无效时构造为空地图，可使用 valid() 检查。
     */
    explicit GridMap(const msg::OccupancyGrid &grid);

    /**
     * @brief 使用全量占据栅格移动构造地图
     *
     * @param[in] grid 全量占据栅格消息
     * @remark 输入无效时构造为空地图，可使用 valid() 检查。
     */
    explicit GridMap(msg::OccupancyGrid &&grid);

    //! @cond
    ~GridMap();
    GridMap(GridMap &&) noexcept;
    GridMap &operator=(GridMap &&) noexcept;
    GridMap(const GridMap &);
    GridMap &operator=(const GridMap &);
    //! @endcond

    /**
     * @brief 装载并校验一张全量地图
     *
     * @param[in] grid 全量占据栅格消息
     * @return 地图装载状态
     * @remark 校验失败时保持原地图不变。
     */
    MapStatus reset(const msg::OccupancyGrid &grid);

    /**
     * @brief 移动装载并校验一张全量地图
     *
     * @param[in] grid 全量占据栅格消息
     * @return 地图装载状态
     * @remark 校验失败时保持原地图不变。
     */
    MapStatus reset(msg::OccupancyGrid &&grid);

    //! @return 地图是否已装载且有效
    bool valid() const noexcept;

    //! @return 当前全量地图消息的常引用
    const msg::OccupancyGrid &message() const noexcept;

    //! @return 地图宽度，单位为格
    uint32_t width() const noexcept;

    //! @return 地图高度，单位为格
    uint32_t height() const noexcept;

    //! @return 占据地图所属坐标系，无效地图返回空字符串
    std::string_view frameId() const noexcept;

    //! @return 地图分辨率，单位为米/格
    double resolution() const noexcept;

    //! @return 当前地图版本
    uint64_t revision() const noexcept;

    /**
     * @brief 判断栅格坐标是否在地图范围内
     *
     * @param[in] x 栅格横向索引
     * @param[in] y 栅格纵向索引
     * @return 栅格坐标是否在地图范围内
     */
    bool contains(uint32_t x, uint32_t y) const noexcept;

    /**
     * @brief 将世界坐标转换为栅格坐标
     *
     * @param[in] world_x 世界坐标 X，单位为米
     * @param[in] world_y 世界坐标 Y，单位为米
     * @return 坐标位于地图内时返回离散栅格，否则返回 `std::nullopt`
     */
    std::optional<Cell> worldToMap(double world_x, double world_y) const noexcept;

    /**
     * @brief 获取栅格中心的世界坐标
     * @param[in] x 栅格横向索引
     * @param[in] y 栅格纵向索引
     *
     * @return 栅格有效时返回世界坐标，否则返回 `std::nullopt`
     */
    std::optional<msg::Point> mapToWorld(uint32_t x, uint32_t y) const noexcept;

    /**
     * @brief 获取指定栅格值
     *
     * @param[in] x 栅格横向索引
     * @param[in] y 栅格纵向索引
     * @return 不存在或地图无效时返回 `std::nullopt`，否则返回栅格值
     */
    std::optional<int8_t> at(uint32_t x, uint32_t y) const noexcept;

    /**
     * @brief 设置指定栅格值
     * @details 当栅格值实际发生变化时，本次调用作为一次地图修改，版本递增 1。
     *
     * @param[in] x 栅格横向索引
     * @param[in] y 栅格纵向索引
     * @param[in] value 栅格值，`-1` 表示未知，`0` 表示空闲，`100` 表示占据
     * @return 地图操作状态
     * @remark 当 @p value 与原值相同时不修改地图，版本保持不变。
     */
    MapStatus set(uint32_t x, uint32_t y, int8_t value) noexcept;

    /**
     * @brief 使用一次占据概率观测更新指定栅格
     * @details 未知栅格以 0.5 为先验，使用赔率相乘完成贝叶斯更新
     *
     * @param[in] x 栅格横向索引
     * @param[in] y 栅格纵向索引
     * @param[in] probability 本次观测为占据的概率，范围为 \f$[0,\,1]\f$
     * @return 地图操作状态
     * @remark 仅当融合结果改变栅格值时，地图版本递增 1。
     */
    MapStatus updateProbability(uint32_t x, uint32_t y, double probability) noexcept;

    /**
     * @brief 融合一条世界坐标射线
     * @details 清除射线途经栅格。当 @p hit 为 @p true 且终点位于地图内时，将终点更新为占据。
     * 射线允许从地图外进入或离开地图，并会自动裁剪至地图边界。
     *
     * @param[in] start_x 射线起点的世界坐标 X，单位为米
     * @param[in] start_y 射线起点的世界坐标 Y，单位为米
     * @param[in] end_x 射线终点的世界坐标 X，单位为米
     * @param[in] end_y 射线终点的世界坐标 Y，单位为米
     * @param[in] hit 终点是否观测到障碍
     * @param[in] free_probability 射线途经栅格为空闲的观测概率
     * @param[in] occupied_probability 射线终点被占据的观测概率
     * @return 地图操作状态
     * @remark 一次调用可能修改多个栅格，但只作为一次地图修改，版本最多递增 1。
     */
    MapStatus integrateRay(double start_x, double start_y, double end_x, double end_y, bool hit = true,
                           double free_probability = 0.3, double occupied_probability = 0.7);

    /**
     * @brief 合并矩形地图增量
     *
     * @param[in] update 待合并的矩形增量消息
     * @return 地图操作状态
     * @remark 合并成功后采用 @p update 携带的目标版本。
     * @remark 合并失败时保持地图数据和版本不变。
     */
    MapStatus apply(const msg::OccupancyGridUpdate &update);

    /**
     * @brief 获取尚未清除的本地地图增量
     *
     * @return 存在本地修改时返回覆盖全部修改的最小矩形增量，否则返回 `std::nullopt`
     * @note 增量范围从上次全量装载、远端增量合并或 clearPendingUpdate() 之后开始累计。
     */
    std::optional<msg::OccupancyGridUpdate> pendingUpdate() const;

    //! 清除本地增量记录，不修改地图内容和版本
    void clearPendingUpdate() noexcept;

private:
    friend class Costmap;
    class Impl;
    std::unique_ptr<Impl> _impl;
};

//! 代价地图特殊值，与常见二维导航代价语义保持一致
enum Cost : uint8_t {
    Free = 0,        //!< 自由空间
    Inscribed = 253, //!< 位于机器人内切半径以内
    Lethal = 254,    //!< 致命障碍
    Unknown = 255,   //!< 未知空间
};

//! 分层代价地图配置
struct CostmapOptions {
    int8_t lethal_threshold{65};      //!< 静态占据值达到该阈值时视为致命障碍
    bool track_unknown{true};         //!< 是否在主代价地图中保留未知空间
    bool unknown_is_lethal{true};     //!< footprint 是否将未知空间视为碰撞
    bool inflate_unknown{false};      //!< 膨胀代价是否覆盖未知空间
    double inflation_radius{0.55};    //!< 障碍膨胀半径，单位为米
    double inscribed_radius{0.20};    //!< 机器人内切半径，单位为米
    double cost_scaling_factor{10.0}; //!< 膨胀区指数衰减系数
};

/**
 * @brief 二维分层代价地图
 * @details 内部维护静态层和局部障碍层，`updateCosts()` 将两层合并后执行障碍膨胀。
 * 局部障碍支持单点标记和射线清除，适合批量接收传感器观测后统一更新主代价地图。
 * @note 该类不提供内部同步。跨线程访问时由调用方加锁。
 */
class Costmap {
public:
    //! 构造空代价地图
    Costmap();

    /**
     * @brief 使用静态占据栅格构造代价地图
     *
     * @param[in] static_map 静态占据栅格
     * @param[in] options 代价地图配置
     * @remark 输入无效时构造为空代价地图，可使用 valid() 检查。
     */
    explicit Costmap(const GridMap &static_map, CostmapOptions options = {});

    //! @cond
    ~Costmap();

    Costmap(Costmap &&) noexcept;
    Costmap &operator=(Costmap &&) noexcept;
    Costmap(const Costmap &);
    Costmap &operator=(const Costmap &);
    //! @endcond

    /**
     * @brief 重建几何信息、静态层和空白局部障碍层
     *
     * @param[in] static_map 静态占据栅格
     * @param[in] options 代价地图配置
     * @return 地图操作状态
     * @remark 操作失败时保持原代价地图不变。
     */
    MapStatus reset(const GridMap &static_map, CostmapOptions options = {});

    /**
     * @brief 使用几何信息一致的新地图更新静态层
     *
     * @param[in] static_map 新的静态占据栅格
     * @return 地图操作状态
     * @note 新地图的坐标系、尺寸、分辨率和原点必须与当前地图一致。
     */
    MapStatus setStaticMap(const GridMap &static_map);

    //! @return 代价地图是否有效
    bool valid() const noexcept;

    //! @return 当前代价地图配置的常引用
    const CostmapOptions &options() const noexcept;

    //! @return 地图宽度，单位为格
    uint32_t width() const noexcept;

    //! @return 地图高度，单位为格
    uint32_t height() const noexcept;

    //! @return 代价地图所属坐标系，无效地图返回空字符串
    std::string_view frameId() const noexcept;

    /**
     * @brief 将世界坐标转换为栅格坐标
     *
     * @param[in] world_x 世界坐标 X，单位为米
     * @param[in] world_y 世界坐标 Y，单位为米
     * @return 坐标位于地图内时返回离散栅格，否则返回 `std::nullopt`
     */
    std::optional<Cell> worldToMap(double world_x, double world_y) const noexcept;

    /**
     * @brief 获取栅格中心的世界坐标
     *
     * @param[in] x 栅格横向索引
     * @param[in] y 栅格纵向索引
     * @return 栅格有效时返回世界坐标，否则返回 `std::nullopt`
     */
    std::optional<msg::Point> mapToWorld(uint32_t x, uint32_t y) const noexcept;

    /**
     * @brief 在局部障碍层标记一个栅格障碍
     *
     * @param[in] cell 障碍所在的离散栅格
     * @return 地图操作状态
     */
    MapStatus markObstacle(Cell cell) noexcept;

    /**
     * @brief 在局部障碍层标记一个世界坐标障碍
     *
     * @param[in] world_x 障碍的世界坐标 X，单位为米
     * @param[in] world_y 障碍的世界坐标 Y，单位为米
     * @return 地图操作状态
     */
    MapStatus markObstacle(double world_x, double world_y) noexcept;

    //! @return 清空局部障碍层的操作状态
    MapStatus clearObstacles() noexcept;

    /**
     * @brief 清除局部障碍层中的一条世界坐标射线
     * @details 射线会裁剪到地图边界，且不修改静态层。
     *
     * @param[in] start_x 射线起点的世界坐标 X，单位为米
     * @param[in] start_y 射线起点的世界坐标 Y，单位为米
     * @param[in] end_x 射线终点的世界坐标 X，单位为米
     * @param[in] end_y 射线终点的世界坐标 Y，单位为米
     * @return 地图操作状态
     */
    MapStatus clearRay(double start_x, double start_y, double end_x, double end_y);

    //! 合并静态层和局部障碍层，并根据配置重新计算膨胀代价
    void updateCosts();

    /**
     * @brief 获取主代价地图栅格值
     *
     * @param[in] x 栅格横向索引
     * @param[in] y 栅格纵向索引
     * @return 栅格有效时返回代价值，否则返回 `std::nullopt`
     */
    std::optional<uint8_t> at(uint32_t x, uint32_t y) const noexcept;

    /**
     * @brief 检测机器人 footprint 是否发生碰撞
     * @param[in] footprint 机器人局部坐标系中的闭合多边形顶点，无需重复首顶点
     * @param[in] pose 机器人在地图坐标系中的平面位姿
     * @return footprint 无效、超出地图，或覆盖致命/内切/按配置判定的未知栅格时返回 `true`
     */
    bool collides(const std::vector<msg::Point> &footprint, const msg::Pose &pose) const;

    /**
     * @brief 将当前主代价地图导出为 OccupancyGrid
     *
     * @return 有效时返回转换后的全量占据栅格，否则返回空消息
     * @note 中间代价值会缩放至 [1, 99]。
     */
    msg::OccupancyGrid message() const;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

//! @} nav_map

} // namespace rm::nav
