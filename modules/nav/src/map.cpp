/**
 * @file map.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 二维占据栅格与分层代价地图实现
 * @version 1.0
 * @date 2026-08-16
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

#include "rmvl/nav/map.hpp"

namespace rm::nav {

namespace {

constexpr double kPlanarTolerance = 1e-9;
constexpr double kProbabilityEpsilon = 0.01;

struct Geometry {
    uint32_t width{};
    uint32_t height{};
    double resolution{};
    double origin_x{};
    double origin_y{};
    double cos_yaw{1.0};
    double sin_yaw{};
};

bool checkedArea(uint32_t width, uint32_t height, std::size_t &area) noexcept {
    if (width == 0 || height == 0)
        return false;
    if (static_cast<std::size_t>(width) > std::numeric_limits<std::size_t>::max() / height)
        return false;
    area = static_cast<std::size_t>(width) * height;
    return true;
}

bool validCellValue(int8_t value) noexcept { return value >= -1 && value <= 100; }

bool planarYaw(const msg::Quaternion &orientation, double &cos_yaw, double &sin_yaw) noexcept {
    if (!std::isfinite(orientation.x) || !std::isfinite(orientation.y) ||
        !std::isfinite(orientation.z) || !std::isfinite(orientation.w))
        return false;
    const double norm2 = orientation.x * orientation.x + orientation.y * orientation.y +
                         orientation.z * orientation.z + orientation.w * orientation.w;
    if (!(norm2 > std::numeric_limits<double>::epsilon()) || !std::isfinite(norm2))
        return false;
    const double inv_norm = 1.0 / std::sqrt(norm2);
    const double x = orientation.x * inv_norm;
    const double y = orientation.y * inv_norm;
    const double z = orientation.z * inv_norm;
    const double w = orientation.w * inv_norm;
    if (std::abs(x) > kPlanarTolerance || std::abs(y) > kPlanarTolerance)
        return false;
    cos_yaw = 1.0 - 2.0 * z * z;
    sin_yaw = 2.0 * w * z;
    return true;
}

MapStatus validateGrid(const msg::OccupancyGrid &grid, Geometry &geometry) noexcept {
    if (!std::isfinite(grid.info.resolution) || grid.info.resolution <= 0.0f)
        return MapStatus::InvalidResolution;

    std::size_t area{};
    if (!checkedArea(grid.info.width, grid.info.height, area) || grid.data.size() != area)
        return MapStatus::InvalidDimensions;

    const auto &position = grid.info.origin.position;
    if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z))
        return MapStatus::InvalidOrigin;
    if (!planarYaw(grid.info.origin.orientation, geometry.cos_yaw, geometry.sin_yaw))
        return MapStatus::InvalidOrigin;

    if (!std::all_of(grid.data.begin(), grid.data.end(), validCellValue))
        return MapStatus::InvalidValue;

    geometry.width = grid.info.width;
    geometry.height = grid.info.height;
    geometry.resolution = grid.info.resolution;
    geometry.origin_x = position.x;
    geometry.origin_y = position.y;
    return MapStatus::Ok;
}

void normalizeOrigin(msg::Pose &origin) noexcept {
    auto &q = origin.orientation;
    const double inv_norm = 1.0 / std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    q = {0.0, 0.0, q.z * inv_norm, q.w * inv_norm};
    if (q.w < 0.0 || (q.w == 0.0 && q.z < 0.0)) {
        q.z = -q.z;
        q.w = -q.w;
    }
}

std::size_t cellIndex(const Geometry &geometry, uint32_t x, uint32_t y) noexcept {
    return static_cast<std::size_t>(y) * geometry.width + x;
}

bool containsCell(const Geometry &geometry, uint32_t x, uint32_t y) noexcept {
    return x < geometry.width && y < geometry.height;
}

bool worldToContinuous(const Geometry &geometry, double world_x, double world_y,
                       double &map_x, double &map_y) noexcept {
    if (!std::isfinite(world_x) || !std::isfinite(world_y))
        return false;
    const double dx = world_x - geometry.origin_x;
    const double dy = world_y - geometry.origin_y;
    map_x = (geometry.cos_yaw * dx + geometry.sin_yaw * dy) / geometry.resolution;
    map_y = (-geometry.sin_yaw * dx + geometry.cos_yaw * dy) / geometry.resolution;
    return std::isfinite(map_x) && std::isfinite(map_y);
}

std::optional<Cell> continuousToCell(const Geometry &geometry, double map_x, double map_y) noexcept {
    if (map_x < 0.0 || map_y < 0.0 || map_x >= geometry.width || map_y >= geometry.height)
        return std::nullopt;
    return Cell{static_cast<uint32_t>(std::floor(map_x)), static_cast<uint32_t>(std::floor(map_y))};
}

std::optional<Cell> worldToCell(const Geometry &geometry, double world_x, double world_y) noexcept {
    double map_x{}, map_y{};
    if (!worldToContinuous(geometry, world_x, world_y, map_x, map_y))
        return std::nullopt;
    return continuousToCell(geometry, map_x, map_y);
}

std::optional<msg::Point> cellToWorld(const Geometry &geometry, uint32_t x, uint32_t y) noexcept {
    if (!containsCell(geometry, x, y))
        return std::nullopt;
    const double local_x = (static_cast<double>(x) + 0.5) * geometry.resolution;
    const double local_y = (static_cast<double>(y) + 0.5) * geometry.resolution;
    return msg::Point{
        geometry.origin_x + geometry.cos_yaw * local_x - geometry.sin_yaw * local_y,
        geometry.origin_y + geometry.sin_yaw * local_x + geometry.cos_yaw * local_y,
        0.0};
}

bool clipTest(double p, double q, double &lower, double &upper) noexcept {
    if (p == 0.0)
        return q >= 0.0;
    const double ratio = q / p;
    if (p < 0.0) {
        if (ratio > upper)
            return false;
        lower = std::max(lower, ratio);
    } else {
        if (ratio < lower)
            return false;
        upper = std::min(upper, ratio);
    }
    return true;
}

bool clipLine(const Geometry &geometry, double &x0, double &y0, double &x1, double &y1) noexcept {
    const double dx = x1 - x0;
    const double dy = y1 - y0;
    double lower = 0.0;
    double upper = 1.0;
    const double max_x = std::nextafter(static_cast<double>(geometry.width), 0.0);
    const double max_y = std::nextafter(static_cast<double>(geometry.height), 0.0);
    if (!clipTest(-dx, x0, lower, upper) || !clipTest(dx, max_x - x0, lower, upper) ||
        !clipTest(-dy, y0, lower, upper) || !clipTest(dy, max_y - y0, lower, upper))
        return false;
    const double start_x = x0;
    const double start_y = y0;
    x0 = start_x + lower * dx;
    y0 = start_y + lower * dy;
    x1 = start_x + upper * dx;
    y1 = start_y + upper * dy;
    return true;
}

std::vector<Cell> rasterLine(const Geometry &geometry, double x0, double y0, double x1, double y1) {
    std::vector<Cell> result{};
    if (!clipLine(geometry, x0, y0, x1, y1))
        return result;

    int x = static_cast<int>(std::floor(x0));
    int y = static_cast<int>(std::floor(y0));
    const int end_x = static_cast<int>(std::floor(x1));
    const int end_y = static_cast<int>(std::floor(y1));
    const int dx = std::abs(end_x - x);
    const int sx = x < end_x ? 1 : -1;
    const int dy = -std::abs(end_y - y);
    const int sy = y < end_y ? 1 : -1;
    int error = dx + dy;

    while (true) {
        result.push_back({static_cast<uint32_t>(x), static_cast<uint32_t>(y)});
        if (x == end_x && y == end_y)
            break;
        const int twice_error = 2 * error;
        if (twice_error >= dy) {
            error += dy;
            x += sx;
        }
        if (twice_error <= dx) {
            error += dx;
            y += sy;
        }
    }
    return result;
}

bool sameGeometry(const msg::OccupancyGrid &lhs, const msg::OccupancyGrid &rhs) noexcept {
    const auto &li = lhs.info;
    const auto &ri = rhs.info;
    const auto &lp = li.origin.position;
    const auto &rp = ri.origin.position;
    const auto &lq = li.origin.orientation;
    const auto &rq = ri.origin.orientation;
    return lhs.header.frame_id == rhs.header.frame_id && li.width == ri.width && li.height == ri.height &&
           li.resolution == ri.resolution && lp.x == rp.x && lp.y == rp.y && lp.z == rp.z &&
           lq.x == rq.x && lq.y == rq.y && lq.z == rq.z && lq.w == rq.w;
}

int8_t probabilityUpdate(int8_t current, double evidence) noexcept {
    if (evidence <= 0.0)
        return 0;
    if (evidence >= 1.0)
        return 100;
    double prior = current < 0 ? 0.5 : static_cast<double>(current) / 100.0;
    prior = std::clamp(prior, kProbabilityEpsilon, 1.0 - kProbabilityEpsilon);
    evidence = std::clamp(evidence, kProbabilityEpsilon, 1.0 - kProbabilityEpsilon);
    const double odds = (prior / (1.0 - prior)) * (evidence / (1.0 - evidence));
    return static_cast<int8_t>(std::clamp(std::lround(100.0 * odds / (1.0 + odds)), 0l, 100l));
}

MapStatus validateOptions(const CostmapOptions &options) noexcept {
    if (options.lethal_threshold < 1 || options.lethal_threshold > 100)
        return MapStatus::InvalidOptions;
    if (!std::isfinite(options.inflation_radius) || options.inflation_radius < 0.0 ||
        !std::isfinite(options.inscribed_radius) || options.inscribed_radius < 0.0 ||
        options.inscribed_radius > options.inflation_radius ||
        !std::isfinite(options.cost_scaling_factor) || options.cost_scaling_factor < 0.0)
        return MapStatus::InvalidOptions;
    return MapStatus::Ok;
}

std::vector<uint8_t> makeStaticLayer(const GridMap &map, const CostmapOptions &options) {
    std::vector<uint8_t> result{};
    result.reserve(map.message().data.size());
    for (const int8_t value : map.message().data) {
        if (value < 0) {
            result.push_back(options.track_unknown ? Unknown : Free);
        } else if (value >= options.lethal_threshold) {
            result.push_back(Lethal);
        } else if (value == 0 || options.lethal_threshold == 1) {
            result.push_back(Free);
        } else {
            const auto cost = static_cast<uint8_t>(std::clamp(
                std::lround(static_cast<double>(value) * (Inscribed - 1) / (options.lethal_threshold - 1)), 1l, 252l));
            result.push_back(cost);
        }
    }
    return result;
}

bool pointInPolygon(double x, double y, const std::vector<std::pair<double, double>> &polygon) noexcept {
    bool inside = false;
    for (std::size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        const auto &[xi, yi] = polygon[i];
        const auto &[xj, yj] = polygon[j];
        if ((yi > y) != (yj > y)) {
            const double crossing = (xj - xi) * (y - yi) / (yj - yi) + xi;
            if (x < crossing)
                inside = !inside;
        }
    }
    return inside;
}

double cross(double ax, double ay, double bx, double by, double cx, double cy) noexcept {
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

bool onSegment(double ax, double ay, double bx, double by, double px, double py) noexcept {
    return std::abs(cross(ax, ay, bx, by, px, py)) <= kPlanarTolerance &&
           px >= std::min(ax, bx) - kPlanarTolerance && px <= std::max(ax, bx) + kPlanarTolerance &&
           py >= std::min(ay, by) - kPlanarTolerance && py <= std::max(ay, by) + kPlanarTolerance;
}

bool segmentsIntersect(double ax, double ay, double bx, double by,
                       double cx, double cy, double dx, double dy) noexcept {
    const double c1 = cross(ax, ay, bx, by, cx, cy);
    const double c2 = cross(ax, ay, bx, by, dx, dy);
    const double c3 = cross(cx, cy, dx, dy, ax, ay);
    const double c4 = cross(cx, cy, dx, dy, bx, by);
    if (((c1 > 0.0 && c2 < 0.0) || (c1 < 0.0 && c2 > 0.0)) &&
        ((c3 > 0.0 && c4 < 0.0) || (c3 < 0.0 && c4 > 0.0)))
        return true;
    return onSegment(ax, ay, bx, by, cx, cy) || onSegment(ax, ay, bx, by, dx, dy) ||
           onSegment(cx, cy, dx, dy, ax, ay) || onSegment(cx, cy, dx, dy, bx, by);
}

bool polygonIntersectsCell(const std::vector<std::pair<double, double>> &polygon, uint32_t x, uint32_t y) noexcept {
    const double left = x;
    const double right = static_cast<double>(x) + 1.0;
    const double bottom = y;
    const double top = static_cast<double>(y) + 1.0;
    for (const auto &[px, py] : polygon)
        if (px >= left && px <= right && py >= bottom && py <= top)
            return true;
    if (pointInPolygon(left, bottom, polygon) || pointInPolygon(right, bottom, polygon) ||
        pointInPolygon(right, top, polygon) || pointInPolygon(left, top, polygon))
        return true;

    constexpr double square[4][2]{{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}};
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        const auto &[ax, ay] = polygon[i];
        const auto &[bx, by] = polygon[(i + 1) % polygon.size()];
        for (std::size_t edge = 0; edge < 4; ++edge) {
            const auto &start = square[edge];
            const auto &end = square[(edge + 1) % 4];
            if (segmentsIntersect(ax, ay, bx, by, left + start[0], bottom + start[1],
                                  left + end[0], bottom + end[1]))
                return true;
        }
    }
    return false;
}

} // namespace

const char *to_string(MapStatus status) noexcept {
    switch (status) {
    case MapStatus::Ok: return "ok";
    case MapStatus::InvalidResolution: return "invalid map resolution";
    case MapStatus::InvalidDimensions: return "invalid map dimensions";
    case MapStatus::InvalidOrigin: return "invalid map origin";
    case MapStatus::InvalidValue: return "invalid cell value";
    case MapStatus::InvalidProbability: return "invalid occupancy probability";
    case MapStatus::InvalidOptions: return "invalid costmap options";
    case MapStatus::InvalidRevision: return "invalid map revision";
    case MapStatus::RevisionMismatch: return "map revision mismatch";
    case MapStatus::FrameMismatch: return "map frame mismatch";
    case MapStatus::GeometryMismatch: return "costmap geometry mismatch";
    case MapStatus::OutOfBounds: return "map coordinate out of bounds";
    case MapStatus::InvalidFootprint: return "invalid robot footprint";
    }
    return "unknown map status";
}

class GridMap::Impl {
public:
    struct DirtyRegion {
        uint64_t base_revision{};
        uint32_t min_x{};
        uint32_t min_y{};
        uint32_t max_x{};
        uint32_t max_y{};
    };

    void markDirty(uint32_t x, uint32_t y, uint64_t base_revision) noexcept {
        if (!dirty) {
            dirty = DirtyRegion{base_revision, x, y, x, y};
            return;
        }
        dirty->min_x = std::min(dirty->min_x, x);
        dirty->min_y = std::min(dirty->min_y, y);
        dirty->max_x = std::max(dirty->max_x, x);
        dirty->max_y = std::max(dirty->max_y, y);
    }

    msg::OccupancyGrid grid{};
    Geometry geometry{};
    std::optional<DirtyRegion> dirty{};
    bool valid{};
};

GridMap::GridMap() : _impl(std::make_unique<Impl>()) {}

GridMap::GridMap(const msg::OccupancyGrid &grid) : GridMap() { reset(grid); }

GridMap::GridMap(msg::OccupancyGrid &&grid) : GridMap() { reset(std::move(grid)); }

GridMap::~GridMap() = default;
GridMap::GridMap(GridMap &&) noexcept = default;
GridMap &GridMap::operator=(GridMap &&) noexcept = default;
GridMap::GridMap(const GridMap &other) : _impl(std::make_unique<Impl>(*other._impl)) {}
GridMap &GridMap::operator=(const GridMap &other) {
    if (this != &other)
        *_impl = *other._impl;
    return *this;
}

MapStatus GridMap::reset(const msg::OccupancyGrid &grid) {
    Geometry geometry{};
    const auto status = validateGrid(grid, geometry);
    if (status != MapStatus::Ok)
        return status;
    _impl->grid = grid;
    normalizeOrigin(_impl->grid.info.origin);
    _impl->geometry = geometry;
    _impl->dirty.reset();
    _impl->valid = true;
    return MapStatus::Ok;
}

MapStatus GridMap::reset(msg::OccupancyGrid &&grid) {
    Geometry geometry{};
    const auto status = validateGrid(grid, geometry);
    if (status != MapStatus::Ok)
        return status;
    _impl->grid = std::move(grid);
    normalizeOrigin(_impl->grid.info.origin);
    _impl->geometry = geometry;
    _impl->dirty.reset();
    _impl->valid = true;
    return MapStatus::Ok;
}

bool GridMap::valid() const noexcept { return _impl->valid; }
const msg::OccupancyGrid &GridMap::message() const noexcept { return _impl->grid; }
uint32_t GridMap::width() const noexcept { return _impl->geometry.width; }
uint32_t GridMap::height() const noexcept { return _impl->geometry.height; }
std::string_view GridMap::frameId() const noexcept {
    return _impl->valid ? std::string_view(_impl->grid.header.frame_id) : std::string_view{};
}
double GridMap::resolution() const noexcept { return _impl->geometry.resolution; }
uint64_t GridMap::revision() const noexcept { return _impl->grid.revision; }

bool GridMap::contains(uint32_t x, uint32_t y) const noexcept {
    return _impl->valid && containsCell(_impl->geometry, x, y);
}

std::optional<Cell> GridMap::worldToMap(double world_x, double world_y) const noexcept {
    return _impl->valid ? worldToCell(_impl->geometry, world_x, world_y) : std::nullopt;
}

std::optional<msg::Point> GridMap::mapToWorld(uint32_t x, uint32_t y) const noexcept {
    return _impl->valid ? cellToWorld(_impl->geometry, x, y) : std::nullopt;
}

std::optional<int8_t> GridMap::at(uint32_t x, uint32_t y) const noexcept {
    if (!contains(x, y))
        return std::nullopt;
    return _impl->grid.data[cellIndex(_impl->geometry, x, y)];
}

MapStatus GridMap::set(uint32_t x, uint32_t y, int8_t value) noexcept {
    if (!contains(x, y))
        return MapStatus::OutOfBounds;
    if (!validCellValue(value))
        return MapStatus::InvalidValue;
    auto &cell = _impl->grid.data[cellIndex(_impl->geometry, x, y)];
    if (cell == value)
        return MapStatus::Ok;
    if (_impl->grid.revision == std::numeric_limits<uint64_t>::max())
        return MapStatus::InvalidRevision;
    const uint64_t base_revision = _impl->grid.revision;
    cell = value;
    ++_impl->grid.revision;
    _impl->markDirty(x, y, base_revision);
    return MapStatus::Ok;
}

MapStatus GridMap::updateProbability(uint32_t x, uint32_t y, double probability) noexcept {
    if (!contains(x, y))
        return MapStatus::OutOfBounds;
    if (!std::isfinite(probability) || probability < 0.0 || probability > 1.0)
        return MapStatus::InvalidProbability;
    auto &cell = _impl->grid.data[cellIndex(_impl->geometry, x, y)];
    const int8_t updated = probabilityUpdate(cell, probability);
    if (cell == updated)
        return MapStatus::Ok;
    if (_impl->grid.revision == std::numeric_limits<uint64_t>::max())
        return MapStatus::InvalidRevision;
    const uint64_t base_revision = _impl->grid.revision;
    cell = updated;
    ++_impl->grid.revision;
    _impl->markDirty(x, y, base_revision);
    return MapStatus::Ok;
}

MapStatus GridMap::integrateRay(double start_x, double start_y, double end_x, double end_y, bool hit,
                                double free_probability, double occupied_probability) {
    if (!_impl->valid)
        return MapStatus::InvalidDimensions;
    if (!std::isfinite(free_probability) || free_probability < 0.0 || free_probability > 1.0 ||
        !std::isfinite(occupied_probability) || occupied_probability < 0.0 || occupied_probability > 1.0)
        return MapStatus::InvalidProbability;

    double map_start_x{}, map_start_y{}, map_end_x{}, map_end_y{};
    if (!worldToContinuous(_impl->geometry, start_x, start_y, map_start_x, map_start_y) ||
        !worldToContinuous(_impl->geometry, end_x, end_y, map_end_x, map_end_y))
        return MapStatus::OutOfBounds;
    const bool end_inside = continuousToCell(_impl->geometry, map_end_x, map_end_y).has_value();
    const auto cells = rasterLine(_impl->geometry, map_start_x, map_start_y, map_end_x, map_end_y);
    if (cells.empty())
        return MapStatus::OutOfBounds;

    auto updated_data = _impl->grid.data;
    const std::size_t clear_count = hit && end_inside ? cells.size() - 1 : cells.size();
    for (std::size_t i = 0; i < clear_count; ++i) {
        auto &value = updated_data[cellIndex(_impl->geometry, cells[i].x, cells[i].y)];
        value = probabilityUpdate(value, free_probability);
    }
    if (hit && end_inside) {
        const auto &endpoint = cells.back();
        auto &value = updated_data[cellIndex(_impl->geometry, endpoint.x, endpoint.y)];
        value = probabilityUpdate(value, occupied_probability);
    }

    if (updated_data == _impl->grid.data)
        return MapStatus::Ok;
    if (_impl->grid.revision == std::numeric_limits<uint64_t>::max())
        return MapStatus::InvalidRevision;
    const uint64_t base_revision = _impl->grid.revision;
    for (const auto &cell : cells) {
        const auto index = cellIndex(_impl->geometry, cell.x, cell.y);
        if (updated_data[index] != _impl->grid.data[index])
            _impl->markDirty(cell.x, cell.y, base_revision);
    }
    _impl->grid.data = std::move(updated_data);
    ++_impl->grid.revision;
    return MapStatus::Ok;
}

MapStatus GridMap::apply(const msg::OccupancyGridUpdate &update) {
    if (!_impl->valid)
        return MapStatus::InvalidDimensions;
    if (update.header.frame_id != _impl->grid.header.frame_id)
        return MapStatus::FrameMismatch;
    if (update.base_revision != _impl->grid.revision)
        return MapStatus::RevisionMismatch;
    if (update.revision <= update.base_revision)
        return MapStatus::InvalidRevision;

    std::size_t area{};
    if (!checkedArea(update.width, update.height, area) || update.data.size() != area)
        return MapStatus::InvalidDimensions;
    if (update.x >= width() || update.y >= height() || update.width > width() - update.x ||
        update.height > height() - update.y)
        return MapStatus::OutOfBounds;
    if (!std::all_of(update.data.begin(), update.data.end(), validCellValue))
        return MapStatus::InvalidValue;

    auto data = _impl->grid.data;
    for (uint32_t row = 0; row < update.height; ++row) {
        const auto source = update.data.begin() + static_cast<std::size_t>(row) * update.width;
        const auto target = data.begin() + cellIndex(_impl->geometry, update.x, update.y + row);
        std::copy_n(source, update.width, target);
    }
    _impl->grid.data = std::move(data);
    _impl->grid.header.stamp = update.header.stamp;
    _impl->grid.revision = update.revision;
    _impl->dirty.reset();
    return MapStatus::Ok;
}

std::optional<msg::OccupancyGridUpdate> GridMap::pendingUpdate() const {
    if (!_impl->valid || !_impl->dirty)
        return std::nullopt;
    const auto &dirty = *_impl->dirty;
    msg::OccupancyGridUpdate result{};
    result.header = _impl->grid.header;
    result.base_revision = dirty.base_revision;
    result.revision = revision();
    result.x = dirty.min_x;
    result.y = dirty.min_y;
    result.width = dirty.max_x - dirty.min_x + 1;
    result.height = dirty.max_y - dirty.min_y + 1;
    result.data.reserve(static_cast<std::size_t>(result.width) * result.height);
    for (uint32_t row = 0; row < result.height; ++row) {
        const auto first = _impl->grid.data.begin() + cellIndex(_impl->geometry, result.x, result.y + row);
        result.data.insert(result.data.end(), first, first + result.width);
    }
    return result;
}

void GridMap::clearPendingUpdate() noexcept { _impl->dirty.reset(); }

class Costmap::Impl {
public:
    GridMap geometry_map{};
    CostmapOptions options{};
    std::vector<uint8_t> static_layer{};
    std::vector<uint8_t> obstacle_layer{};
    std::vector<uint8_t> master{};
    uint64_t revision{};
    bool valid{};
};

Costmap::Costmap() : _impl(std::make_unique<Impl>()) {}

Costmap::Costmap(const GridMap &static_map, CostmapOptions options) : Costmap() {
    reset(static_map, options);
}

Costmap::~Costmap() = default;
Costmap::Costmap(Costmap &&) noexcept = default;
Costmap &Costmap::operator=(Costmap &&) noexcept = default;
Costmap::Costmap(const Costmap &other) : _impl(std::make_unique<Impl>(*other._impl)) {}
Costmap &Costmap::operator=(const Costmap &other) {
    if (this != &other)
        *_impl = *other._impl;
    return *this;
}

MapStatus Costmap::reset(const GridMap &static_map, CostmapOptions options) {
    if (!static_map.valid())
        return MapStatus::InvalidDimensions;
    const auto option_status = validateOptions(options);
    if (option_status != MapStatus::Ok)
        return option_status;

    Impl replacement{};
    replacement.geometry_map = static_map;
    replacement.options = options;
    replacement.static_layer = makeStaticLayer(static_map, options);
    replacement.obstacle_layer.assign(replacement.static_layer.size(), Free);
    replacement.master = replacement.static_layer;
    replacement.revision = static_map.revision();
    replacement.valid = true;
    *_impl = std::move(replacement);
    updateCosts();
    return MapStatus::Ok;
}

MapStatus Costmap::setStaticMap(const GridMap &static_map) {
    if (!_impl->valid || !static_map.valid())
        return MapStatus::InvalidDimensions;
    if (!sameGeometry(_impl->geometry_map.message(), static_map.message()))
        return MapStatus::GeometryMismatch;
    if (_impl->revision == std::numeric_limits<uint64_t>::max())
        return MapStatus::InvalidRevision;
    _impl->geometry_map = static_map;
    _impl->static_layer = makeStaticLayer(static_map, _impl->options);
    ++_impl->revision;
    updateCosts();
    return MapStatus::Ok;
}

bool Costmap::valid() const noexcept { return _impl->valid; }
const CostmapOptions &Costmap::options() const noexcept { return _impl->options; }
uint32_t Costmap::width() const noexcept { return _impl->geometry_map.width(); }
uint32_t Costmap::height() const noexcept { return _impl->geometry_map.height(); }
std::string_view Costmap::frameId() const noexcept {
    return _impl->valid ? _impl->geometry_map.frameId() : std::string_view{};
}

std::optional<Cell> Costmap::worldToMap(double world_x, double world_y) const noexcept {
    return _impl->geometry_map.worldToMap(world_x, world_y);
}

std::optional<msg::Point> Costmap::mapToWorld(uint32_t x, uint32_t y) const noexcept {
    return _impl->geometry_map.mapToWorld(x, y);
}

MapStatus Costmap::markObstacle(Cell cell) noexcept {
    if (!_impl->valid || !_impl->geometry_map.contains(cell.x, cell.y))
        return MapStatus::OutOfBounds;
    auto &cost = _impl->obstacle_layer[cellIndex(_impl->geometry_map._impl->geometry, cell.x, cell.y)];
    if (cost == Lethal)
        return MapStatus::Ok;
    if (_impl->revision == std::numeric_limits<uint64_t>::max())
        return MapStatus::InvalidRevision;
    cost = Lethal;
    ++_impl->revision;
    return MapStatus::Ok;
}

MapStatus Costmap::markObstacle(double world_x, double world_y) noexcept {
    const auto cell = worldToMap(world_x, world_y);
    return cell ? markObstacle(*cell) : MapStatus::OutOfBounds;
}

MapStatus Costmap::clearObstacles() noexcept {
    if (!_impl->valid)
        return MapStatus::InvalidDimensions;
    if (std::none_of(_impl->obstacle_layer.begin(), _impl->obstacle_layer.end(),
                     [](uint8_t value) { return value != Free; }))
        return MapStatus::Ok;
    if (_impl->revision == std::numeric_limits<uint64_t>::max())
        return MapStatus::InvalidRevision;
    std::fill(_impl->obstacle_layer.begin(), _impl->obstacle_layer.end(), Free);
    ++_impl->revision;
    return MapStatus::Ok;
}

MapStatus Costmap::clearRay(double start_x, double start_y, double end_x, double end_y) {
    if (!_impl->valid)
        return MapStatus::InvalidDimensions;
    const auto &geometry = _impl->geometry_map._impl->geometry;
    double map_start_x{}, map_start_y{}, map_end_x{}, map_end_y{};
    if (!worldToContinuous(geometry, start_x, start_y, map_start_x, map_start_y) ||
        !worldToContinuous(geometry, end_x, end_y, map_end_x, map_end_y))
        return MapStatus::OutOfBounds;
    const auto cells = rasterLine(geometry, map_start_x, map_start_y, map_end_x, map_end_y);
    if (cells.empty())
        return MapStatus::OutOfBounds;

    auto updated = _impl->obstacle_layer;
    for (const auto &cell : cells)
        updated[cellIndex(geometry, cell.x, cell.y)] = Free;
    if (updated == _impl->obstacle_layer)
        return MapStatus::Ok;
    if (_impl->revision == std::numeric_limits<uint64_t>::max())
        return MapStatus::InvalidRevision;
    _impl->obstacle_layer = std::move(updated);
    ++_impl->revision;
    return MapStatus::Ok;
}

void Costmap::updateCosts() {
    if (!_impl->valid)
        return;
    _impl->master = _impl->static_layer;
    for (std::size_t i = 0; i < _impl->master.size(); ++i)
        if (_impl->obstacle_layer[i] == Lethal)
            _impl->master[i] = Lethal;

    const auto &geometry = _impl->geometry_map._impl->geometry;
    const int cell_radius = static_cast<int>(std::ceil(_impl->options.inflation_radius / geometry.resolution));
    if (cell_radius <= 0)
        return;
    std::vector<Cell> obstacles{};
    for (uint32_t y = 0; y < geometry.height; ++y)
        for (uint32_t x = 0; x < geometry.width; ++x)
            if (_impl->master[cellIndex(geometry, x, y)] == Lethal)
                obstacles.push_back({x, y});

    for (const auto &obstacle : obstacles) {
        const int min_x = std::max(0, static_cast<int>(obstacle.x) - cell_radius);
        const int max_x = std::min(static_cast<int>(geometry.width) - 1, static_cast<int>(obstacle.x) + cell_radius);
        const int min_y = std::max(0, static_cast<int>(obstacle.y) - cell_radius);
        const int max_y = std::min(static_cast<int>(geometry.height) - 1, static_cast<int>(obstacle.y) + cell_radius);
        for (int y = min_y; y <= max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                const double distance = std::hypot(x - static_cast<int>(obstacle.x), y - static_cast<int>(obstacle.y)) * geometry.resolution;
                if (distance > _impl->options.inflation_radius)
                    continue;
                auto &master = _impl->master[cellIndex(geometry, static_cast<uint32_t>(x), static_cast<uint32_t>(y))];
                if (master == Lethal || (master == Unknown && !_impl->options.inflate_unknown))
                    continue;
                uint8_t inflation_cost{};
                if (distance <= _impl->options.inscribed_radius) {
                    inflation_cost = Inscribed;
                } else {
                    inflation_cost = static_cast<uint8_t>(std::clamp(
                        std::lround((Inscribed - 1) * std::exp(-_impl->options.cost_scaling_factor *
                                                              (distance - _impl->options.inscribed_radius))),
                        1l, 252l));
                }
                master = master == Unknown ? inflation_cost : std::max(master, inflation_cost);
            }
        }
    }
}

std::optional<uint8_t> Costmap::at(uint32_t x, uint32_t y) const noexcept {
    if (!_impl->valid || !_impl->geometry_map.contains(x, y))
        return std::nullopt;
    return _impl->master[cellIndex(_impl->geometry_map._impl->geometry, x, y)];
}

bool Costmap::collides(const std::vector<msg::Point> &footprint, const msg::Pose &pose) const {
    if (!_impl->valid || footprint.size() < 3 || !std::isfinite(pose.position.x) ||
        !std::isfinite(pose.position.y) || !std::isfinite(pose.position.z))
        return true;
    double pose_cos{}, pose_sin{};
    if (!planarYaw(pose.orientation, pose_cos, pose_sin))
        return true;

    const auto &geometry = _impl->geometry_map._impl->geometry;
    std::vector<std::pair<double, double>> polygon{};
    polygon.reserve(footprint.size());
    double min_x = std::numeric_limits<double>::max();
    double min_y = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double max_y = std::numeric_limits<double>::lowest();
    for (const auto &point : footprint) {
        if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
            return true;
        const double world_x = pose.position.x + pose_cos * point.x - pose_sin * point.y;
        const double world_y = pose.position.y + pose_sin * point.x + pose_cos * point.y;
        double map_x{}, map_y{};
        if (!worldToContinuous(geometry, world_x, world_y, map_x, map_y) ||
            map_x < 0.0 || map_y < 0.0 || map_x >= geometry.width || map_y >= geometry.height)
            return true;
        polygon.emplace_back(map_x, map_y);
        min_x = std::min(min_x, map_x);
        min_y = std::min(min_y, map_y);
        max_x = std::max(max_x, map_x);
        max_y = std::max(max_y, map_y);
    }

    const uint32_t first_x = static_cast<uint32_t>(std::max(0.0, std::floor(min_x)));
    const uint32_t first_y = static_cast<uint32_t>(std::max(0.0, std::floor(min_y)));
    const uint32_t last_x = static_cast<uint32_t>(std::min(static_cast<double>(geometry.width - 1), std::floor(max_x)));
    const uint32_t last_y = static_cast<uint32_t>(std::min(static_cast<double>(geometry.height - 1), std::floor(max_y)));
    for (uint32_t y = first_y; y <= last_y; ++y) {
        for (uint32_t x = first_x; x <= last_x; ++x) {
            if (!polygonIntersectsCell(polygon, x, y))
                continue;
            const uint8_t cost = _impl->master[cellIndex(geometry, x, y)];
            if ((cost == Unknown && _impl->options.unknown_is_lethal) ||
                (cost != Unknown && cost >= Inscribed))
                return true;
        }
    }
    return false;
}

msg::OccupancyGrid Costmap::message() const {
    if (!_impl->valid)
        return {};
    msg::OccupancyGrid result = _impl->geometry_map.message();
    result.revision = _impl->revision;
    result.data.resize(_impl->master.size());
    for (std::size_t i = 0; i < _impl->master.size(); ++i) {
        const uint8_t cost = _impl->master[i];
        if (cost == Unknown)
            result.data[i] = -1;
        else if (cost == Lethal)
            result.data[i] = 100;
        else if (cost == Free)
            result.data[i] = 0;
        else
            result.data[i] = static_cast<int8_t>(std::clamp(
                std::lround(static_cast<double>(cost) * 99.0 / static_cast<double>(Inscribed)), 1l, 99l));
    }
    return result;
}

} // namespace rm::nav
