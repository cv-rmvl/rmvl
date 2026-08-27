/**
 * @file test_localization.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 视觉里程计与 SLAM 接入单元测试
 * @version 1.0
 * @date 2026-08-24
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "rmvl/nav/localization.hpp"

namespace rm_test {

using namespace rm;
using namespace rm::nav;

namespace {

msg::Time stamp(int64_t nanoseconds) {
    return {static_cast<int32_t>(nanoseconds / 1'000'000'000LL),
            static_cast<uint32_t>(nanoseconds % 1'000'000'000LL)};
}

msg::CameraInfo cameraInfo(std::string_view frame_id, uint32_t width = 4, uint32_t height = 3) {
    msg::CameraInfo info{};
    info.header.frame_id = frame_id;
    info.width = width;
    info.height = height;
    info.K = {200.0, 0.0, 2.0, 0.0, 200.0, 1.5, 0.0, 0.0, 1.0};
    return info;
}

msg::Image image(std::string_view frame_id, int64_t nanoseconds, uint8_t encoding = msg::Image::encoding_mono8,
                 uint32_t width = 4, uint32_t height = 3) {
    msg::Image value{};
    value.header.stamp = stamp(nanoseconds);
    value.header.frame_id = frame_id;
    value.width = static_cast<int32_t>(width);
    value.height = static_cast<int32_t>(height);
    value.encoding = encoding;
    std::size_t bytes = static_cast<std::size_t>(width) * height;
    if (encoding == msg::Image::encoding_16uc1)
        bytes *= 2;
    else if (encoding == msg::Image::encoding_32fc1)
        bytes *= 4;
    value.data.assign(bytes, 7U);
    return value;
}

msg::Imu imu(int64_t nanoseconds) {
    msg::Imu value{};
    value.header.stamp = stamp(nanoseconds);
    value.header.frame_id = "imu_link";
    value.orientation.w = 1.0;
    return value;
}

msg::Odometry wheelOdometry(int64_t nanoseconds) {
    msg::Odometry value{};
    value.header.stamp = stamp(nanoseconds);
    value.header.frame_id = "odom";
    value.child_frame_id = "base_link";
    value.pose.pose.orientation.w = 1.0;
    return value;
}

SensorFrame monocularFrame(int64_t nanoseconds) {
    SensorFrame frame{};
    frame.mode = CameraMode::Monocular;
    frame.primary = std::make_shared<const msg::Image>(image("camera", nanoseconds));
    frame.primary_info = cameraInfo("camera");
    return frame;
}

class FakeBackend final : public OdometryBackend {
public:
    BackendEstimate track(const SensorFrame &) override {
        ++calls;
        BackendEstimate estimate{};
        estimate.status = status;
        estimate.reference_frame = reference_frame;
        estimate.child_frame = "base_link";
        estimate.pose.pose.position.x = static_cast<double>(calls);
        estimate.pose.pose.orientation.w = 2.0;
        estimate.twist.twist.linear.x = 0.5;
        return estimate;
    }

    void reset() override { calls = 0; }

    LocalizationStatus status{LocalizationStatus::Ok};
    std::string reference_frame{"odom"};
    int calls{};
};

class FakeMapper final : public MapperBackend {
public:
    MappingResult update(const SensorFrame &, const msg::Odometry &odometry) override {
        ++calls;
        MappingResult result{};
        result.status = LocalizationStatus::Ok;
        result.map.emplace();
        result.map->header = odometry.header;
        return result;
    }

    void reset() override { calls = 0; }

    int calls{};
};

class FakeSlam final : public SlamBackend {
public:
    BackendEstimate track(const SensorFrame &) override { return {}; }
    void reset() override {}
};

} // namespace

TEST(Nav_Localization, exposes_stable_status_strings) {
    EXPECT_STREQ(to_string(LocalizationStatus::WaitingForSecondary), "waiting for secondary image");
    EXPECT_STREQ(to_string(LocalizationStatus::TrackingLost), "tracking lost");
}

TEST(Nav_FrameSynchronizer, matches_rgbd_and_collects_optional_motion_samples) {
    FrameSyncOptions options{};
    options.mode = CameraMode::Rgbd;
    options.tolerance = std::chrono::milliseconds(10);
    FrameSynchronizer synchronizer(options);
    ASSERT_EQ(synchronizer.setPrimaryInfo(cameraInfo("rgb_optical")), LocalizationStatus::Ok);
    ASSERT_EQ(synchronizer.setSecondaryInfo(cameraInfo("depth_optical")), LocalizationStatus::Ok);
    ASSERT_EQ(synchronizer.pushImu(imu(90'000'000)), LocalizationStatus::Ok);
    ASSERT_EQ(synchronizer.pushImu(imu(100'000'000)), LocalizationStatus::Ok);
    ASSERT_EQ(synchronizer.pushOdometry(wheelOdometry(102'000'000)), LocalizationStatus::Ok);
    ASSERT_EQ(synchronizer.pushSecondary(image("depth_optical", 105'000'000, msg::Image::encoding_16uc1)),
              LocalizationStatus::Ok);
    ASSERT_EQ(synchronizer.pushPrimary(image("rgb_optical", 100'000'000)), LocalizationStatus::Ok);

    auto result = synchronizer.next();
    ASSERT_TRUE(result) << to_string(result.status);
    EXPECT_EQ(result.frame.mode, CameraMode::Rgbd);
    ASSERT_TRUE(result.frame.secondary);
    EXPECT_EQ(result.frame.secondary->encoding, msg::Image::encoding_16uc1);
    EXPECT_EQ(result.frame.imu.size(), 2U);
    ASSERT_TRUE(result.frame.auxiliary_odometry);
    EXPECT_EQ(result.frame.auxiliary_odometry->header.stamp.sec, 0);
    EXPECT_EQ(result.frame.auxiliary_odometry->header.stamp.nsec, 102'000'000U);
    EXPECT_EQ(synchronizer.statistics().matched, 1U);

    ASSERT_EQ(synchronizer.pushImu(imu(110'000'000)), LocalizationStatus::Ok);
    ASSERT_EQ(synchronizer.pushSecondary(image("depth_optical", 119'000'000, msg::Image::encoding_32fc1)),
              LocalizationStatus::Ok);
    ASSERT_EQ(synchronizer.pushPrimary(image("rgb_optical", 120'000'000)), LocalizationStatus::Ok);
    result = synchronizer.next();
    ASSERT_TRUE(result);
    ASSERT_EQ(result.frame.imu.size(), 1U);
    EXPECT_EQ(result.frame.imu.front().header.stamp.sec, 0);
    EXPECT_EQ(result.frame.imu.front().header.stamp.nsec, 110'000'000U);
}

TEST(Nav_FrameSynchronizer, supports_monocular_and_rejects_malformed_inputs) {
    FrameSyncOptions options{};
    options.mode = CameraMode::Monocular;
    FrameSynchronizer synchronizer(options);
    ASSERT_EQ(synchronizer.setPrimaryInfo(cameraInfo("camera")), LocalizationStatus::Ok);
    auto malformed = image("camera", 1);
    malformed.data.pop_back();
    EXPECT_EQ(synchronizer.pushPrimary(std::move(malformed)), LocalizationStatus::InvalidSensorData);
    EXPECT_EQ(synchronizer.statistics().rejected, 1U);

    ASSERT_EQ(synchronizer.pushPrimary(image("camera", 2)), LocalizationStatus::Ok);
    const auto result = synchronizer.next();
    ASSERT_TRUE(result);
    EXPECT_FALSE(result.frame.secondary);
    EXPECT_FALSE(result.frame.secondary_info);
}

TEST(Nav_FrameSynchronizer, reports_timestamp_and_frame_mismatches) {
    FrameSyncOptions options{};
    options.mode = CameraMode::Stereo;
    options.tolerance = std::chrono::milliseconds(5);
    FrameSynchronizer synchronizer(options);
    ASSERT_EQ(synchronizer.setPrimaryInfo(cameraInfo("left")), LocalizationStatus::Ok);
    ASSERT_EQ(synchronizer.setSecondaryInfo(cameraInfo("right")), LocalizationStatus::Ok);
    ASSERT_EQ(synchronizer.pushPrimary(image("left", 100'000'000)), LocalizationStatus::Ok);
    ASSERT_EQ(synchronizer.pushSecondary(image("right", 120'000'000)), LocalizationStatus::Ok);
    EXPECT_EQ(synchronizer.next().status, LocalizationStatus::TimestampMismatch);

    ASSERT_EQ(synchronizer.pushPrimary(image("other_left", 125'000'000)), LocalizationStatus::Ok);
    EXPECT_EQ(synchronizer.next().status, LocalizationStatus::FrameMismatch);
}

TEST(Nav_LocalizationPipeline, normalizes_and_converts_backend_output) {
    auto backend = std::make_unique<FakeBackend>();
    auto mapper = std::make_unique<FakeMapper>();
    auto *mapper_ptr = mapper.get();
    LocalizationOptions options{};
    options.path_capacity = 2;
    LocalizationPipeline pipeline(std::move(backend), std::move(mapper), options);

    auto first = pipeline.process(monocularFrame(10));
    ASSERT_TRUE(first) << first.detail;
    EXPECT_EQ(first.odometry.header.stamp.sec, 0);
    EXPECT_EQ(first.odometry.header.stamp.nsec, 10U);
    EXPECT_EQ(first.odometry.header.frame_id, "odom");
    EXPECT_EQ(first.odometry.child_frame_id, "base_link");
    EXPECT_DOUBLE_EQ(first.odometry.pose.pose.orientation.w, 1.0);
    EXPECT_DOUBLE_EQ(first.transform.transform.translation.x, 1.0);
    EXPECT_EQ(first.path.poses.size(), 1U);
    ASSERT_TRUE(first.mapping);
    EXPECT_EQ(first.mapping->status, LocalizationStatus::Ok);
    EXPECT_EQ(mapper_ptr->calls, 1);

    EXPECT_TRUE(pipeline.process(monocularFrame(20)));
    const auto third = pipeline.process(monocularFrame(30));
    ASSERT_TRUE(third);
    ASSERT_EQ(third.path.poses.size(), 2U);
    EXPECT_DOUBLE_EQ(third.path.poses.front().pose.position.x, 2.0);
    EXPECT_DOUBLE_EQ(third.path.poses.back().pose.position.x, 3.0);

    pipeline.reset();
    const auto reset = pipeline.process(monocularFrame(40));
    ASSERT_TRUE(reset);
    EXPECT_DOUBLE_EQ(reset.odometry.pose.pose.position.x, 1.0);
    EXPECT_EQ(reset.path.poses.size(), 1U);
}

TEST(Nav_LocalizationPipeline, propagates_non_tracking_state_without_publishing_pose) {
    auto backend = std::make_unique<FakeBackend>();
    backend->status = LocalizationStatus::TrackingLost;
    LocalizationPipeline pipeline(std::move(backend));
    const auto result = pipeline.process(monocularFrame(10));
    EXPECT_FALSE(result);
    EXPECT_EQ(result.status, LocalizationStatus::TrackingLost);
    EXPECT_TRUE(result.path.poses.empty());
}

TEST(Nav_SlamBackend, default_relocalization_is_optional) {
    FakeSlam backend;
    msg::PoseWithCovariance pose{};
    pose.pose.orientation.w = 1.0;
    EXPECT_EQ(backend.relocalize(pose, "map"), LocalizationStatus::Unsupported);
}

} // namespace rm_test
