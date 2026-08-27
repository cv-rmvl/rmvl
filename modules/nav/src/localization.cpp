/**
 * @file localization.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 视觉里程计与 SLAM 接入实现
 * @version 1.0
 * @date 2026-08-24
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include "rmvl/nav/localization.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <exception>
#include <iterator>
#include <limits>
#include <mutex>
#include <utility>

namespace rm::nav {

namespace {

constexpr int64_t kNanosecondsPerSecond = 1'000'000'000LL;

bool validTime(const msg::Time &time) noexcept { return time.nsec < static_cast<uint32_t>(kNanosecondsPerSecond); }

int64_t timestamp(const msg::Time &time) noexcept {
    return static_cast<int64_t>(time.sec) * kNanosecondsPerSecond + static_cast<int64_t>(time.nsec);
}

uint64_t distance(int64_t lhs, int64_t rhs) noexcept {
    return lhs >= rhs ? static_cast<uint64_t>(lhs - rhs) : static_cast<uint64_t>(rhs - lhs);
}

bool finite(double value) noexcept { return std::isfinite(value); }

bool validQuaternion(const msg::Quaternion &quaternion) noexcept {
    if (!finite(quaternion.x) || !finite(quaternion.y) || !finite(quaternion.z) || !finite(quaternion.w))
        return false;
    const double norm = quaternion.x * quaternion.x + quaternion.y * quaternion.y +
                        quaternion.z * quaternion.z + quaternion.w * quaternion.w;
    return finite(norm) && norm > std::numeric_limits<double>::epsilon();
}

void normalize(msg::Quaternion &quaternion) noexcept {
    const double inverse_norm = 1.0 / std::sqrt(quaternion.x * quaternion.x + quaternion.y * quaternion.y +
                                                quaternion.z * quaternion.z + quaternion.w * quaternion.w);
    quaternion.x *= inverse_norm;
    quaternion.y *= inverse_norm;
    quaternion.z *= inverse_norm;
    quaternion.w *= inverse_norm;
}

bool validPose(const msg::Pose &pose) noexcept {
    return finite(pose.position.x) && finite(pose.position.y) && finite(pose.position.z) &&
           validQuaternion(pose.orientation);
}

bool validTwist(const msg::Twist &twist) noexcept {
    return finite(twist.linear.x) && finite(twist.linear.y) && finite(twist.linear.z) &&
           finite(twist.angular.x) && finite(twist.angular.y) && finite(twist.angular.z);
}

template <std::size_t N>
bool allFinite(const std::array<double, N> &values) noexcept {
    return std::all_of(values.begin(), values.end(), [](double value) { return finite(value); });
}

bool validCameraInfo(const msg::CameraInfo &info) noexcept {
    return !info.header.frame_id.empty() && validTime(info.header.stamp) && info.width > 0 && info.height > 0 &&
           allFinite(info.D) && allFinite(info.K) && info.K[0] > 0.0 && info.K[4] > 0.0 &&
           finite(info.K[2]) && finite(info.K[5]);
}

std::optional<std::size_t> imageBytes(const msg::Image &image) noexcept {
    if (image.width <= 0 || image.height <= 0)
        return std::nullopt;
    const auto width = static_cast<uint64_t>(image.width);
    const auto height = static_cast<uint64_t>(image.height);
    if (height > std::numeric_limits<uint64_t>::max() / width)
        return std::nullopt;
    const uint64_t pixels = width * height;
    uint64_t bytes{};
    switch (image.encoding) {
    case msg::Image::encoding_rgb8:
    case msg::Image::encoding_bgr8:
        bytes = pixels * 3;
        break;
    case msg::Image::encoding_mono8:
    case msg::Image::encoding_bayer_rggb8:
    case msg::Image::encoding_bayer_bggr8:
        bytes = pixels;
        break;
    case msg::Image::encoding_mono16:
    case msg::Image::encoding_bayer_rggb16:
    case msg::Image::encoding_bayer_bggbr16:
    case msg::Image::encoding_16uc1:
    case msg::Image::encoding_yuv422:
        bytes = pixels * 2;
        break;
    case msg::Image::encoding_rgba8:
    case msg::Image::encoding_bgra8:
    case msg::Image::encoding_32fc1:
        bytes = pixels * 4;
        break;
    case msg::Image::encoding_yuv420:
        if ((width & 1U) != 0 || (height & 1U) != 0)
            return std::nullopt;
        bytes = pixels + pixels / 2;
        break;
    default:
        return std::nullopt;
    }
    if (bytes > std::numeric_limits<std::size_t>::max())
        return std::nullopt;
    return static_cast<std::size_t>(bytes);
}

bool validImage(const msg::Image &image) noexcept {
    const auto bytes = imageBytes(image);
    return !image.header.frame_id.empty() && validTime(image.header.stamp) && bytes && image.data.size() == *bytes;
}

bool imageMatches(const msg::Image &image, const msg::CameraInfo &info) noexcept {
    return image.header.frame_id == info.header.frame_id && static_cast<uint32_t>(image.width) == info.width &&
           static_cast<uint32_t>(image.height) == info.height;
}

bool validImu(const msg::Imu &imu) noexcept {
    const bool orientation_valid = imu.orientation_covariance[0] == -1.0 || validQuaternion(imu.orientation);
    return !imu.header.frame_id.empty() && validTime(imu.header.stamp) && orientation_valid &&
           finite(imu.angular_velocity.x) && finite(imu.angular_velocity.y) && finite(imu.angular_velocity.z) &&
           finite(imu.linear_acceleration.x) && finite(imu.linear_acceleration.y) &&
           finite(imu.linear_acceleration.z) && allFinite(imu.orientation_covariance) &&
           allFinite(imu.angular_velocity_covariance) && allFinite(imu.linear_acceleration_covariance);
}

bool validOdometry(const msg::Odometry &odometry) noexcept {
    return !odometry.header.frame_id.empty() && !odometry.child_frame_id.empty() &&
           odometry.header.frame_id != odometry.child_frame_id && validTime(odometry.header.stamp) &&
           validPose(odometry.pose.pose) && validTwist(odometry.twist.twist) &&
           allFinite(odometry.pose.covariance) && allFinite(odometry.twist.covariance);
}

bool validTransform(const msg::TransformStamped &transform) noexcept {
    return !transform.header.frame_id.empty() && !transform.child_frame_id.empty() &&
           transform.header.frame_id != transform.child_frame_id && validTime(transform.header.stamp) &&
           finite(transform.transform.translation.x) && finite(transform.transform.translation.y) &&
           finite(transform.transform.translation.z) && validQuaternion(transform.transform.rotation);
}

template <typename Container, typename Value, typename Stamp>
void insertOrdered(Container &container, Value &&value, Stamp stamp) {
    const auto value_stamp = stamp(value);
    const auto position = std::upper_bound(container.begin(), container.end(), value_stamp,
                                           [&](int64_t time, const auto &item) { return time < stamp(item); });
    container.insert(position, std::forward<Value>(value));
}

} // namespace

const char *to_string(LocalizationStatus status) noexcept {
    switch (status) {
    case LocalizationStatus::Ok:
        return "ok";
    case LocalizationStatus::WaitingForPrimary:
        return "waiting for primary image";
    case LocalizationStatus::WaitingForSecondary:
        return "waiting for secondary image";
    case LocalizationStatus::MissingCalibration:
        return "missing camera calibration";
    case LocalizationStatus::InvalidOptions:
        return "invalid options";
    case LocalizationStatus::InvalidSensorData:
        return "invalid sensor data";
    case LocalizationStatus::FrameMismatch:
        return "frame mismatch";
    case LocalizationStatus::TimestampMismatch:
        return "timestamp mismatch";
    case LocalizationStatus::Initializing:
        return "initializing";
    case LocalizationStatus::TrackingLost:
        return "tracking lost";
    case LocalizationStatus::BackendError:
        return "backend error";
    case LocalizationStatus::InvalidEstimate:
        return "invalid estimate";
    case LocalizationStatus::Unsupported:
        return "unsupported";
    }
    return "unknown";
}

const msg::Time &SensorFrame::stamp() const noexcept {
    static const msg::Time empty{};
    return primary ? primary->header.stamp : empty;
}

class FrameSynchronizer::Impl {
public:
    explicit Impl(FrameSyncOptions sync_options) : options(std::move(sync_options)) {}

    bool validOptions() const noexcept {
        return options.tolerance.count() >= 0 && options.image_queue_size > 0 && options.imu_queue_size > 0 &&
               options.odometry_queue_size > 0;
    }

    LocalizationStatus pushImage(std::shared_ptr<const msg::Image> image, bool primary_image) {
        if (!image || !validImage(*image)) {
            std::lock_guard<std::mutex> lock(mutex);
            ++statistics.rejected;
            return LocalizationStatus::InvalidSensorData;
        }
        std::lock_guard<std::mutex> lock(mutex);
        if (!validOptions()) {
            ++statistics.rejected;
            return LocalizationStatus::InvalidOptions;
        }
        auto &queue = primary_image ? primary : secondary;
        insertOrdered(queue, std::move(image), [](const auto &item) { return timestamp(item->header.stamp); });
        if (primary_image)
            ++statistics.received_primary;
        else
            ++statistics.received_secondary;
        while (queue.size() > options.image_queue_size) {
            queue.pop_front();
            if (primary_image)
                ++statistics.dropped_primary;
            else
                ++statistics.dropped_secondary;
        }
        return LocalizationStatus::Ok;
    }

    FrameSyncOptions options{};
    mutable std::mutex mutex{};
    std::optional<msg::CameraInfo> primary_info{};
    std::optional<msg::CameraInfo> secondary_info{};
    std::deque<std::shared_ptr<const msg::Image>> primary{};
    std::deque<std::shared_ptr<const msg::Image>> secondary{};
    std::deque<msg::Imu> imu{};
    std::deque<msg::Odometry> odometry{};
    std::optional<int64_t> previous_frame{};
    FrameSyncStatistics statistics{};
};

FrameSynchronizer::FrameSynchronizer(FrameSyncOptions options) : _impl(std::make_unique<Impl>(std::move(options))) {}

FrameSynchronizer::~FrameSynchronizer() = default;
FrameSynchronizer::FrameSynchronizer(FrameSynchronizer &&) noexcept = default;
FrameSynchronizer &FrameSynchronizer::operator=(FrameSynchronizer &&) noexcept = default;

const FrameSyncOptions &FrameSynchronizer::options() const noexcept { return _impl->options; }

LocalizationStatus FrameSynchronizer::setPrimaryInfo(const msg::CameraInfo &info) {
    if (!validCameraInfo(info)) {
        std::lock_guard<std::mutex> lock(_impl->mutex);
        ++_impl->statistics.rejected;
        return LocalizationStatus::InvalidSensorData;
    }
    std::lock_guard<std::mutex> lock(_impl->mutex);
    _impl->primary_info = info;
    return LocalizationStatus::Ok;
}

LocalizationStatus FrameSynchronizer::setSecondaryInfo(const msg::CameraInfo &info) {
    if (!validCameraInfo(info)) {
        std::lock_guard<std::mutex> lock(_impl->mutex);
        ++_impl->statistics.rejected;
        return LocalizationStatus::InvalidSensorData;
    }
    std::lock_guard<std::mutex> lock(_impl->mutex);
    _impl->secondary_info = info;
    return LocalizationStatus::Ok;
}

LocalizationStatus FrameSynchronizer::pushPrimary(msg::Image image) {
    return pushPrimary(std::make_shared<const msg::Image>(std::move(image)));
}

LocalizationStatus FrameSynchronizer::pushPrimary(std::shared_ptr<const msg::Image> image) {
    return _impl->pushImage(std::move(image), true);
}

LocalizationStatus FrameSynchronizer::pushSecondary(msg::Image image) {
    return pushSecondary(std::make_shared<const msg::Image>(std::move(image)));
}

LocalizationStatus FrameSynchronizer::pushSecondary(std::shared_ptr<const msg::Image> image) {
    return _impl->pushImage(std::move(image), false);
}

LocalizationStatus FrameSynchronizer::pushImu(msg::Imu imu) {
    if (!validImu(imu)) {
        std::lock_guard<std::mutex> lock(_impl->mutex);
        ++_impl->statistics.rejected;
        return LocalizationStatus::InvalidSensorData;
    }
    std::lock_guard<std::mutex> lock(_impl->mutex);
    if (!_impl->validOptions()) {
        ++_impl->statistics.rejected;
        return LocalizationStatus::InvalidOptions;
    }
    insertOrdered(_impl->imu, std::move(imu), [](const auto &item) { return timestamp(item.header.stamp); });
    while (_impl->imu.size() > _impl->options.imu_queue_size)
        _impl->imu.pop_front();
    return LocalizationStatus::Ok;
}

LocalizationStatus FrameSynchronizer::pushOdometry(msg::Odometry odometry) {
    if (!validOdometry(odometry)) {
        std::lock_guard<std::mutex> lock(_impl->mutex);
        ++_impl->statistics.rejected;
        return LocalizationStatus::InvalidSensorData;
    }
    std::lock_guard<std::mutex> lock(_impl->mutex);
    if (!_impl->validOptions()) {
        ++_impl->statistics.rejected;
        return LocalizationStatus::InvalidOptions;
    }
    insertOrdered(_impl->odometry, std::move(odometry), [](const auto &item) { return timestamp(item.header.stamp); });
    while (_impl->odometry.size() > _impl->options.odometry_queue_size)
        _impl->odometry.pop_front();
    return LocalizationStatus::Ok;
}

FrameSyncResult FrameSynchronizer::next() {
    std::lock_guard<std::mutex> lock(_impl->mutex);
    if (!_impl->validOptions())
        return {LocalizationStatus::InvalidOptions, {}};
    if (!_impl->primary_info || (_impl->options.mode != CameraMode::Monocular && !_impl->secondary_info))
        return {LocalizationStatus::MissingCalibration, {}};
    if (_impl->primary.empty())
        return {LocalizationStatus::WaitingForPrimary, {}};

    const auto primary_image = _impl->primary.front();
    const int64_t primary_stamp = timestamp(primary_image->header.stamp);
    if (_impl->previous_frame && primary_stamp <= *_impl->previous_frame) {
        _impl->primary.pop_front();
        ++_impl->statistics.dropped_primary;
        return {LocalizationStatus::TimestampMismatch, {}};
    }
    if (!imageMatches(*primary_image, *_impl->primary_info)) {
        _impl->primary.pop_front();
        ++_impl->statistics.dropped_primary;
        return {LocalizationStatus::FrameMismatch, {}};
    }

    std::shared_ptr<const msg::Image> secondary_image{};
    if (_impl->options.mode != CameraMode::Monocular) {
        const uint64_t tolerance = static_cast<uint64_t>(_impl->options.tolerance.count());
        while (!_impl->secondary.empty() && timestamp(_impl->secondary.front()->header.stamp) < primary_stamp &&
               distance(timestamp(_impl->secondary.front()->header.stamp), primary_stamp) > tolerance) {
            _impl->secondary.pop_front();
            ++_impl->statistics.dropped_secondary;
        }
        if (_impl->secondary.empty())
            return {LocalizationStatus::WaitingForSecondary, {}};

        auto best = _impl->secondary.begin();
        uint64_t best_distance = distance(timestamp((*best)->header.stamp), primary_stamp);
        for (auto it = std::next(best); it != _impl->secondary.end(); ++it) {
            const auto current_distance = distance(timestamp((*it)->header.stamp), primary_stamp);
            if (current_distance >= best_distance)
                break;
            best = it;
            best_distance = current_distance;
        }
        if (best_distance > tolerance) {
            if (timestamp(_impl->secondary.front()->header.stamp) > primary_stamp) {
                _impl->primary.pop_front();
                ++_impl->statistics.dropped_primary;
                return {LocalizationStatus::TimestampMismatch, {}};
            }
            return {LocalizationStatus::WaitingForSecondary, {}};
        }
        secondary_image = *best;
        if (!imageMatches(*secondary_image, *_impl->secondary_info)) {
            _impl->secondary.erase(best);
            ++_impl->statistics.dropped_secondary;
            return {LocalizationStatus::FrameMismatch, {}};
        }
        const auto stale = static_cast<std::size_t>(std::distance(_impl->secondary.begin(), best));
        _impl->statistics.dropped_secondary += stale;
        _impl->secondary.erase(_impl->secondary.begin(), std::next(best));
    }

    SensorFrame frame{};
    frame.mode = _impl->options.mode;
    frame.primary = primary_image;
    frame.secondary = std::move(secondary_image);
    frame.primary_info = *_impl->primary_info;
    if (_impl->options.mode != CameraMode::Monocular)
        frame.secondary_info = *_impl->secondary_info;
    _impl->primary.pop_front();

    while (!_impl->imu.empty() && timestamp(_impl->imu.front().header.stamp) <= primary_stamp) {
        const auto imu_stamp = timestamp(_impl->imu.front().header.stamp);
        if (!_impl->previous_frame || imu_stamp > *_impl->previous_frame)
            frame.imu.push_back(std::move(_impl->imu.front()));
        _impl->imu.pop_front();
    }

    if (!_impl->odometry.empty()) {
        const uint64_t tolerance = static_cast<uint64_t>(_impl->options.tolerance.count());
        auto best = _impl->odometry.begin();
        uint64_t best_distance = distance(timestamp(best->header.stamp), primary_stamp);
        for (auto it = std::next(best); it != _impl->odometry.end(); ++it) {
            const auto current_distance = distance(timestamp(it->header.stamp), primary_stamp);
            if (current_distance >= best_distance)
                break;
            best = it;
            best_distance = current_distance;
        }
        if (best_distance <= tolerance) {
            frame.auxiliary_odometry = std::move(*best);
            _impl->odometry.erase(best);
        }
        while (!_impl->odometry.empty() && timestamp(_impl->odometry.front().header.stamp) <= primary_stamp)
            _impl->odometry.pop_front();
    }

    _impl->previous_frame = primary_stamp;
    ++_impl->statistics.matched;
    return {LocalizationStatus::Ok, std::move(frame)};
}

FrameSyncStatistics FrameSynchronizer::statistics() const noexcept {
    std::lock_guard<std::mutex> lock(_impl->mutex);
    return _impl->statistics;
}

void FrameSynchronizer::clear() noexcept {
    std::lock_guard<std::mutex> lock(_impl->mutex);
    _impl->primary.clear();
    _impl->secondary.clear();
    _impl->imu.clear();
    _impl->odometry.clear();
    _impl->previous_frame.reset();
}

LocalizationStatus SlamBackend::relocalize(const msg::PoseWithCovariance &, std::string_view) {
    return LocalizationStatus::Unsupported;
}

class LocalizationPipeline::Impl {
public:
    Impl(std::unique_ptr<OdometryBackend> localization_backend, std::unique_ptr<MapperBackend> mapping_backend,
         LocalizationOptions localization_options)
        : backend(std::move(localization_backend)), mapper(std::move(mapping_backend)), options(localization_options) {}

    std::unique_ptr<OdometryBackend> backend{};
    std::unique_ptr<MapperBackend> mapper{};
    LocalizationOptions options{};
    msg::Path path{};
    std::mutex mutex{};
};

LocalizationPipeline::LocalizationPipeline(std::unique_ptr<OdometryBackend> backend,
                                           std::unique_ptr<MapperBackend> mapper,
                                           LocalizationOptions options)
    : _impl(std::make_unique<Impl>(std::move(backend), std::move(mapper), options)) {}

LocalizationPipeline::~LocalizationPipeline() = default;
LocalizationPipeline::LocalizationPipeline(LocalizationPipeline &&) noexcept = default;
LocalizationPipeline &LocalizationPipeline::operator=(LocalizationPipeline &&) noexcept = default;

LocalizationResult LocalizationPipeline::process(const SensorFrame &frame) {
    std::lock_guard<std::mutex> lock(_impl->mutex);
    LocalizationResult result{};
    if (!_impl->backend) {
        result.detail = "localization backend is null";
        return result;
    }
    if (!frame.primary || !validImage(*frame.primary) || !validCameraInfo(frame.primary_info) ||
        !imageMatches(*frame.primary, frame.primary_info) ||
        (frame.mode != CameraMode::Monocular &&
         (!frame.secondary || !frame.secondary_info || !validImage(*frame.secondary) ||
          !validCameraInfo(*frame.secondary_info) || !imageMatches(*frame.secondary, *frame.secondary_info)))) {
        result.status = LocalizationStatus::InvalidSensorData;
        return result;
    }

    BackendEstimate estimate{};
    try {
        estimate = _impl->backend->track(frame);
    } catch (const std::exception &error) {
        result.detail = error.what();
        return result;
    } catch (...) {
        result.detail = "localization backend threw an unknown exception";
        return result;
    }
    result.status = estimate.status;
    result.detail = std::move(estimate.detail);
    if (estimate.status != LocalizationStatus::Ok)
        return result;

    if (estimate.reference_frame.empty() || estimate.child_frame.empty() ||
        estimate.reference_frame == estimate.child_frame || !validPose(estimate.pose.pose) ||
        !validTwist(estimate.twist.twist) || !allFinite(estimate.pose.covariance) ||
        !allFinite(estimate.twist.covariance) ||
        (estimate.map_to_odom &&
         (!validTransform(*estimate.map_to_odom) || estimate.map_to_odom->child_frame_id != estimate.reference_frame))) {
        result.status = LocalizationStatus::InvalidEstimate;
        return result;
    }

    normalize(estimate.pose.pose.orientation);
    result.odometry.header.stamp = frame.stamp();
    result.odometry.header.frame_id = std::move(estimate.reference_frame);
    result.odometry.child_frame_id = std::move(estimate.child_frame);
    result.odometry.pose = std::move(estimate.pose);
    result.odometry.twist = std::move(estimate.twist);

    result.transform.header = result.odometry.header;
    result.transform.child_frame_id = result.odometry.child_frame_id;
    result.transform.transform.translation.x = result.odometry.pose.pose.position.x;
    result.transform.transform.translation.y = result.odometry.pose.pose.position.y;
    result.transform.transform.translation.z = result.odometry.pose.pose.position.z;
    result.transform.transform.rotation = result.odometry.pose.pose.orientation;

    if (estimate.map_to_odom) {
        estimate.map_to_odom->header.stamp = frame.stamp();
        normalize(estimate.map_to_odom->transform.rotation);
        result.map_to_odom = std::move(estimate.map_to_odom);
    }

    if (_impl->options.path_capacity > 0) {
        if (_impl->path.header.frame_id != result.odometry.header.frame_id) {
            _impl->path = {};
            _impl->path.header.frame_id = result.odometry.header.frame_id;
        }
        _impl->path.header.stamp = frame.stamp();
        msg::PoseStamped pose{};
        pose.header = result.odometry.header;
        pose.pose = result.odometry.pose.pose;
        _impl->path.poses.push_back(std::move(pose));
        if (_impl->path.poses.size() > _impl->options.path_capacity) {
            const auto excess = _impl->path.poses.size() - _impl->options.path_capacity;
            _impl->path.poses.erase(_impl->path.poses.begin(), _impl->path.poses.begin() + static_cast<std::ptrdiff_t>(excess));
        }
        result.path = _impl->path;
    }

    if (_impl->mapper) {
        try {
            result.mapping = _impl->mapper->update(frame, result.odometry);
        } catch (const std::exception &error) {
            result.mapping = MappingResult{LocalizationStatus::BackendError, error.what(), {}, {}};
        } catch (...) {
            result.mapping = MappingResult{LocalizationStatus::BackendError,
                                           "mapper backend threw an unknown exception", {}, {}};
        }
    }
    return result;
}

void LocalizationPipeline::reset() {
    std::lock_guard<std::mutex> lock(_impl->mutex);
    if (_impl->backend)
        _impl->backend->reset();
    if (_impl->mapper)
        _impl->mapper->reset();
    _impl->path = {};
}

const LocalizationOptions &LocalizationPipeline::options() const noexcept { return _impl->options; }

} // namespace rm::nav
