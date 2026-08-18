/**
 * @file transform.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 通用坐标变换与时间缓存实现
 * @version 1.0
 * @date 2026-08-16
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "rmvl/lpss/transform.hpp"
#include "rmvl/lpss/node.hpp"

namespace rm {

namespace msg {

Quaternion operator*(const Quaternion &lhs, const Quaternion &rhs) noexcept {
    return {lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x,
            lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w,
            lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z};
}

Vector3 rotate(const Quaternion &rotation, const Vector3 &vector) noexcept {
    const double tx = 2.0 * (rotation.y * vector.z - rotation.z * vector.y);
    const double ty = 2.0 * (rotation.z * vector.x - rotation.x * vector.z);
    const double tz = 2.0 * (rotation.x * vector.y - rotation.y * vector.x);
    return {
        vector.x + rotation.w * tx + (rotation.y * tz - rotation.z * ty),
        vector.y + rotation.w * ty + (rotation.z * tx - rotation.x * tz),
        vector.z + rotation.w * tz + (rotation.x * ty - rotation.y * tx)};
}

Point operator*(const Transform &transform, const Point &point) noexcept {
    const auto rotated = rotate(transform.rotation, {point.x, point.y, point.z});
    return {transform.translation.x + rotated.x,
            transform.translation.y + rotated.y,
            transform.translation.z + rotated.z};
}

Pose operator*(const Transform &transform, const Pose &pose) noexcept {
    const auto point = transform * pose.position;
    return {point, transform.rotation * pose.orientation};
}

Transform operator*(const Transform &lhs, const Transform &rhs) noexcept {
    const auto rotated = rotate(lhs.rotation, rhs.translation);
    return {{lhs.translation.x + rotated.x,
             lhs.translation.y + rotated.y,
             lhs.translation.z + rotated.z},
            lhs.rotation * rhs.rotation};
}

Transform inverse(const Transform &transform) noexcept {
    const auto &q = transform.rotation;
    const double norm2 = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (!(norm2 > 0.0) || !std::isfinite(norm2))
        return {};

    const double inv_norm = 1.0 / std::sqrt(norm2);
    const Quaternion inv_rotation{-q.x * inv_norm, -q.y * inv_norm, -q.z * inv_norm, q.w * inv_norm};
    const Vector3 neg_translation{-transform.translation.x, -transform.translation.y, -transform.translation.z};
    return {rotate(inv_rotation, neg_translation), inv_rotation};
}

} // namespace msg

namespace lpss::tf {

namespace {

constexpr int64_t kNanosecondsPerSecond = 1'000'000'000ll;

struct TransformSample {
    int64_t time{};
    msg::Transform transform{};
};

struct TransformEdge {
    std::string parent{};
    bool is_static{};
    msg::Transform static_transform{};
    std::vector<TransformSample> samples{};
};

msg::Transform identityTransform() noexcept {
    msg::Transform result{};
    result.rotation.w = 1.0;
    return result;
}

bool isZeroTime(const msg::Time &time) noexcept { return time.sec == 0 && time.nsec == 0; }

bool validTime(const msg::Time &time) noexcept { return time.nsec < static_cast<uint32_t>(kNanosecondsPerSecond); }

int64_t toNanoseconds(const msg::Time &time) noexcept {
    return static_cast<int64_t>(time.sec) * kNanosecondsPerSecond + static_cast<int64_t>(time.nsec);
}

msg::Time fromNanoseconds(int64_t time) noexcept {
    int64_t sec = time / kNanosecondsPerSecond;
    int64_t nsec = time % kNanosecondsPerSecond;
    if (nsec < 0) {
        --sec;
        nsec += kNanosecondsPerSecond;
    }
    return {static_cast<int32_t>(sec), static_cast<uint32_t>(nsec)};
}

bool normalizeQuaternion(msg::Quaternion &q) noexcept {
    if (!std::isfinite(q.x) || !std::isfinite(q.y) || !std::isfinite(q.z) || !std::isfinite(q.w))
        return false;
    const double norm2 = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (!(norm2 > std::numeric_limits<double>::epsilon()) || !std::isfinite(norm2))
        return false;
    const double inv_norm = 1.0 / std::sqrt(norm2);
    q.x *= inv_norm;
    q.y *= inv_norm;
    q.z *= inv_norm;
    q.w *= inv_norm;
    return true;
}

bool normalizeTransform(msg::Transform &transform) noexcept {
    const auto &translation = transform.translation;
    if (!std::isfinite(translation.x) || !std::isfinite(translation.y) || !std::isfinite(translation.z))
        return false;
    return normalizeQuaternion(transform.rotation);
}

msg::Quaternion slerp(msg::Quaternion lhs, msg::Quaternion rhs, double ratio) noexcept {
    double dot = lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
    if (dot < 0.0) {
        rhs.x = -rhs.x;
        rhs.y = -rhs.y;
        rhs.z = -rhs.z;
        rhs.w = -rhs.w;
        dot = -dot;
    }

    dot = std::clamp(dot, -1.0, 1.0);
    msg::Quaternion result{};
    if (dot > 0.9995) {
        result = {lhs.x + ratio * (rhs.x - lhs.x),
                  lhs.y + ratio * (rhs.y - lhs.y),
                  lhs.z + ratio * (rhs.z - lhs.z),
                  lhs.w + ratio * (rhs.w - lhs.w)};
        normalizeQuaternion(result);
        return result;
    }

    const double theta = std::acos(dot);
    const double sin_theta = std::sin(theta);
    const double lhs_scale = std::sin((1.0 - ratio) * theta) / sin_theta;
    const double rhs_scale = std::sin(ratio * theta) / sin_theta;
    return {lhs_scale * lhs.x + rhs_scale * rhs.x,
            lhs_scale * lhs.y + rhs_scale * rhs.y,
            lhs_scale * lhs.z + rhs_scale * rhs.z,
            lhs_scale * lhs.w + rhs_scale * rhs.w};
}

msg::Transform interpolate(const TransformSample &lhs, const TransformSample &rhs, int64_t time) noexcept {
    const double ratio = static_cast<double>(time - lhs.time) / static_cast<double>(rhs.time - lhs.time);
    const auto &lt = lhs.transform.translation;
    const auto &rt = rhs.transform.translation;
    return {{lt.x + ratio * (rt.x - lt.x),
             lt.y + ratio * (rt.y - lt.y),
             lt.z + ratio * (rt.z - lt.z)},
            slerp(lhs.transform.rotation, rhs.transform.rotation, ratio)};
}

Status sampleEdge(const TransformEdge &edge, int64_t time, msg::Transform &transform) noexcept {
    if (edge.is_static) {
        transform = edge.static_transform;
        return Status::Ok;
    }
    if (time < edge.samples.front().time)
        return Status::ExtrapolationPast;
    if (time > edge.samples.back().time)
        return Status::ExtrapolationFuture;

    const auto upper = std::lower_bound(
        edge.samples.begin(), edge.samples.end(), time,
        [](const TransformSample &sample, int64_t value) { return sample.time < value; });
    if (upper->time == time || upper == edge.samples.begin()) {
        transform = upper->transform;
        return Status::Ok;
    }
    transform = interpolate(*std::prev(upper), *upper, time);
    return Status::Ok;
}

int64_t cacheCutoff(int64_t latest, std::chrono::nanoseconds cache_duration) noexcept {
    const int64_t duration = cache_duration.count();
    return latest < std::numeric_limits<int64_t>::min() + duration
               ? std::numeric_limits<int64_t>::min()
               : latest - duration;
}

void pruneEdge(TransformEdge &edge, std::chrono::nanoseconds cache_duration) {
    if (edge.is_static || edge.samples.empty())
        return;
    const int64_t cutoff = cacheCutoff(edge.samples.back().time, cache_duration);
    const auto first = std::lower_bound(
        edge.samples.begin(), edge.samples.end(), cutoff,
        [](const TransformSample &sample, int64_t value) { return sample.time < value; });
    edge.samples.erase(edge.samples.begin(), first);
}

std::string topicName(std::string_view name, std::string_view suffix) {
    std::string result(name);
    if (!result.empty() && result.back() != '/')
        result.push_back('/');
    result.append(suffix);
    return result;
}

} // namespace

class Buffer::Impl {
public:
    explicit Impl(std::chrono::nanoseconds duration) : cache_duration(std::max(duration, std::chrono::nanoseconds::zero())) {}

    mutable std::shared_mutex mutex{};
    std::unordered_map<std::string, TransformEdge> edges{};
    std::unordered_set<std::string> frames{};
    std::chrono::nanoseconds cache_duration{};
};

const char *to_string(Status status) noexcept {
    switch (status) {
    case Status::Ok: return "ok";
    case Status::InvalidArgument: return "invalid argument";
    case Status::InvalidTransform: return "invalid transform";
    case Status::MultipleParents: return "multiple parents";
    case Status::CycleDetected: return "cycle detected";
    case Status::StaticDynamicConflict: return "static/dynamic conflict";
    case Status::FrameNotFound: return "frame not found";
    case Status::ConnectivityError: return "frames are not connected";
    case Status::ExtrapolationPast: return "query precedes transform history";
    case Status::ExtrapolationFuture: return "query exceeds transform history";
    case Status::NoCommonTime: return "no common time on transform path";
    case Status::TimestampOutOfRange: return "timestamp is outside cache window";
    }
    return "unknown transform status";
}

Buffer::Buffer(std::chrono::nanoseconds cache_duration)
    : _impl(std::make_unique<Impl>(cache_duration)) {}

Buffer::Buffer(Buffer &&) noexcept = default;
Buffer &Buffer::operator=(Buffer &&) noexcept = default;
Buffer::~Buffer() = default;

Status Buffer::set(const msg::TransformStamped &input) { return setImpl(input, false); }

Status Buffer::set(const msg::TF &transforms) {
    Status result = Status::Ok;
    for (const auto &transform : transforms.transforms) {
        const auto current = set(transform);
        if (result == Status::Ok && current != Status::Ok)
            result = current;
    }
    return result;
}

Status Buffer::setStatic(const msg::TransformStamped &input) { return setImpl(input, true); }

Status Buffer::setStatic(const msg::TF &transforms) {
    Status result = Status::Ok;
    for (const auto &transform : transforms.transforms) {
        const auto current = setStatic(transform);
        if (result == Status::Ok && current != Status::Ok)
            result = current;
    }
    return result;
}

Status Buffer::setImpl(const msg::TransformStamped &input, bool is_static) {
    if (input.header.frame_id.empty() || input.child_frame_id.empty() || input.header.frame_id == input.child_frame_id || !validTime(input.header.stamp))
        return Status::InvalidArgument;

    msg::Transform transform = input.transform;
    if (!normalizeTransform(transform))
        return Status::InvalidTransform;

    const std::string parent = input.header.frame_id;
    const std::string child = input.child_frame_id;
    const int64_t time = toNanoseconds(input.header.stamp);
    std::unique_lock lock(_impl->mutex);

    auto edge_it = _impl->edges.find(child);
    if (edge_it != _impl->edges.end()) {
        if (edge_it->second.parent != parent)
            return Status::MultipleParents;
        if (edge_it->second.is_static != is_static)
            return Status::StaticDynamicConflict;
    } else {
        for (std::string frame = parent;;) {
            if (frame == child)
                return Status::CycleDetected;
            const auto ancestor = _impl->edges.find(frame);
            if (ancestor == _impl->edges.end())
                break;
            frame = ancestor->second.parent;
        }
        edge_it = _impl->edges.emplace(child, TransformEdge{parent, is_static, {}, {}}).first;
    }

    auto &edge = edge_it->second;
    if (is_static) {
        edge.static_transform = transform;
    } else {
        if (!edge.samples.empty() && time < cacheCutoff(edge.samples.back().time, _impl->cache_duration))
            return Status::TimestampOutOfRange;
        const auto pos = std::lower_bound(
            edge.samples.begin(), edge.samples.end(), time,
            [](const TransformSample &sample, int64_t value) { return sample.time < value; });
        if (pos != edge.samples.end() && pos->time == time)
            pos->transform = transform;
        else
            edge.samples.insert(pos, {time, transform});
        pruneEdge(edge, _impl->cache_duration);
    }

    _impl->frames.insert(parent);
    _impl->frames.insert(child);
    return Status::Ok;
}

LookupResult Buffer::lookup(
    std::string_view target_frame_view,
    std::string_view source_frame_view,
    const msg::Time &time) const {
    LookupResult result{};
    if (target_frame_view.empty() || source_frame_view.empty() || !validTime(time)) {
        result.status = Status::InvalidArgument;
        return result;
    }

    const std::string target_frame(target_frame_view);
    const std::string source_frame(source_frame_view);
    std::shared_lock lock(_impl->mutex);
    if (_impl->frames.find(target_frame) == _impl->frames.end() || _impl->frames.find(source_frame) == _impl->frames.end()) {
        result.status = Status::FrameNotFound;
        return result;
    }

    result.transform.header.frame_id = target_frame;
    result.transform.child_frame_id = source_frame;
    if (target_frame == source_frame) {
        result.status = Status::Ok;
        result.transform.header.stamp = time;
        result.transform.transform = identityTransform();
        return result;
    }

    std::unordered_set<std::string> source_ancestors{};
    for (std::string frame = source_frame;;) {
        source_ancestors.insert(frame);
        const auto edge = _impl->edges.find(frame);
        if (edge == _impl->edges.end())
            break;
        frame = edge->second.parent;
    }

    std::string common = target_frame;
    while (source_ancestors.find(common) == source_ancestors.end()) {
        const auto edge = _impl->edges.find(common);
        if (edge == _impl->edges.end()) {
            result.status = Status::ConnectivityError;
            return result;
        }
        common = edge->second.parent;
    }

    std::vector<const TransformEdge *> source_edges{};
    for (std::string frame = source_frame; frame != common;) {
        const auto edge = _impl->edges.find(frame);
        source_edges.push_back(&edge->second);
        frame = edge->second.parent;
    }
    std::vector<const TransformEdge *> target_edges{};
    for (std::string frame = target_frame; frame != common;) {
        const auto edge = _impl->edges.find(frame);
        target_edges.push_back(&edge->second);
        frame = edge->second.parent;
    }

    int64_t query_time = toNanoseconds(time);
    if (isZeroTime(time)) {
        bool has_dynamic_edge = false;
        int64_t earliest = std::numeric_limits<int64_t>::min();
        int64_t latest = std::numeric_limits<int64_t>::max();
        const auto update_interval = [&](const TransformEdge *edge) {
            if (!edge->is_static) {
                has_dynamic_edge = true;
                earliest = std::max(earliest, edge->samples.front().time);
                latest = std::min(latest, edge->samples.back().time);
            }
        };
        for (const auto *edge : source_edges)
            update_interval(edge);
        for (const auto *edge : target_edges)
            update_interval(edge);
        if (has_dynamic_edge && latest < earliest) {
            result.status = Status::NoCommonTime;
            return result;
        }
        query_time = has_dynamic_edge ? latest : 0;
    }

    msg::Transform common_from_source = identityTransform();
    for (const auto *edge : source_edges) {
        msg::Transform parent_from_child{};
        result.status = sampleEdge(*edge, query_time, parent_from_child);
        if (result.status != Status::Ok)
            return result;
        common_from_source = parent_from_child * common_from_source;
    }

    msg::Transform common_from_target = identityTransform();
    for (const auto *edge : target_edges) {
        msg::Transform parent_from_child{};
        result.status = sampleEdge(*edge, query_time, parent_from_child);
        if (result.status != Status::Ok)
            return result;
        common_from_target = parent_from_child * common_from_target;
    }

    result.transform.header.stamp = fromNanoseconds(query_time);
    result.transform.transform = msg::inverse(common_from_target) * common_from_source;
    normalizeQuaternion(result.transform.transform.rotation);
    result.status = Status::Ok;
    return result;
}

bool Buffer::can(
    std::string_view target_frame,
    std::string_view source_frame,
    const msg::Time &time) const {
    return static_cast<bool>(lookup(target_frame, source_frame, time));
}

void Buffer::setCacheDuration(std::chrono::nanoseconds cache_duration) noexcept {
    cache_duration = std::max(cache_duration, std::chrono::nanoseconds::zero());
    std::unique_lock lock(_impl->mutex);
    _impl->cache_duration = cache_duration;
    for (auto &[_, edge] : _impl->edges)
        pruneEdge(edge, cache_duration);
}

std::chrono::nanoseconds Buffer::cacheDuration() const noexcept {
    std::shared_lock lock(_impl->mutex);
    return _impl->cache_duration;
}

std::size_t Buffer::size() const noexcept {
    std::shared_lock lock(_impl->mutex);
    return _impl->edges.size();
}

void Buffer::clear() noexcept {
    std::unique_lock lock(_impl->mutex);
    _impl->edges.clear();
    _impl->frames.clear();
}

namespace {

class PublisherHandle {
public:
    PublisherHandle(std::string_view name, std::string_view suffix, Node &node)
        : _node(&node), _publisher(node.createPublisher<msg::TF>(topicName(name, suffix))) {}

#if __cplusplus >= 202002L
    PublisherHandle(std::string_view name, std::string_view suffix, async::Node &node)
        : _async_node(&node), _async_publisher(node.createPublisher<msg::TF>(topicName(name, suffix))) {}
#endif

    ~PublisherHandle() {
        if (_node)
            _node->destroyPublisher(_publisher);
#if __cplusplus >= 202002L
        if (_async_node)
            _async_node->destroyPublisher(_async_publisher);
#endif
    }

    bool invalid() const noexcept {
        if (_node)
            return _publisher.invalid();
#if __cplusplus >= 202002L
        return !_async_publisher || _async_publisher->invalid();
#else
        return true;
#endif
    }

    void send(const msg::TF &transforms) {
        RMVL_Assert(!invalid());
        if (_node) {
            _publisher.publish(transforms);
            return;
        }
#if __cplusplus >= 202002L
        _async_publisher->publish(transforms);
#endif
    }

private:
    Node *_node{};
    Publisher<msg::TF> _publisher{nullptr};
#if __cplusplus >= 202002L
    async::Node *_async_node{};
    async::Publisher<msg::TF>::ptr _async_publisher{};
#endif
};

class ListenerState {
public:
    explicit ListenerState(Buffer &buffer) : _buffer(&buffer) {}

    void receive(const msg::TF &transforms, bool is_static) {
        std::lock_guard lock(_mutex);
        if (!_active)
            return;
        _status.store(is_static ? _buffer->setStatic(transforms) : _buffer->set(transforms), std::memory_order_release);
    }

    void deactivate() noexcept {
        std::lock_guard lock(_mutex);
        _active = false;
    }

    Status status() const noexcept { return _status.load(std::memory_order_acquire); }

private:
    std::mutex _mutex;
    Buffer *_buffer;
    std::atomic<Status> _status{Status::Ok};
    bool _active{true};
};

class ListenerHandle {
public:
    ListenerHandle(std::string_view name, Node &node, Buffer &buffer)
        : _node(&node), _state(std::make_shared<ListenerState>(buffer)),
          _subscriber(node.createSubscriber<msg::TF>(topicName(name, "tf"), [state = _state](const msg::TF &msg) {
              state->receive(msg, false);
          })),
          _static_subscriber(node.createSubscriber<msg::TF>(topicName(name, "tf_static"), [state = _state](const msg::TF &msg) {
              state->receive(msg, true);
          })) {}

#if __cplusplus >= 202002L
    ListenerHandle(std::string_view name, async::Node &node, Buffer &buffer)
        : _async_node(&node), _state(std::make_shared<ListenerState>(buffer)),
          _async_subscriber(node.createSubscriber<msg::TF>(topicName(name, "tf"), [state = _state](const msg::TF &msg) {
              state->receive(msg, false);
          })),
          _async_static_subscriber(node.createSubscriber<msg::TF>(topicName(name, "tf_static"), [state = _state](const msg::TF &msg) {
              state->receive(msg, true);
          })) {}
#endif

    ~ListenerHandle() {
        _state->deactivate();
        if (_node) {
            _node->destroySubscriber(_subscriber);
            _node->destroySubscriber(_static_subscriber);
        }
#if __cplusplus >= 202002L
        if (_async_node) {
            _async_node->destroySubscriber(_async_subscriber);
            _async_node->destroySubscriber(_async_static_subscriber);
        }
#endif
    }

    bool invalid() const noexcept {
        if (_node)
            return _subscriber.invalid() || _static_subscriber.invalid();
#if __cplusplus >= 202002L
        return !_async_subscriber || !_async_static_subscriber || _async_subscriber->invalid() || _async_static_subscriber->invalid();
#else
        return true;
#endif
    }

    Status status() const noexcept { return _state->status(); }

private:
    Node *_node{};
#if __cplusplus >= 202002L
    async::Node *_async_node{};
#endif
    std::shared_ptr<ListenerState> _state;
    Subscriber<msg::TF> _subscriber{nullptr};
    Subscriber<msg::TF> _static_subscriber{nullptr};
#if __cplusplus >= 202002L
    async::Subscriber<msg::TF>::ptr _async_subscriber{};
    async::Subscriber<msg::TF>::ptr _async_static_subscriber{};
#endif
};

void sendOne(PublisherHandle &publisher, const msg::TransformStamped &transform) {
    msg::TF transforms{};
    transforms.transforms.push_back(transform);
    publisher.send(transforms);
}

} // namespace

class Broadcaster::Impl final : public PublisherHandle {
public:
    using PublisherHandle::PublisherHandle;
};

Broadcaster::Broadcaster(std::string_view name, Node &node) : _impl(std::make_unique<Impl>(name, "tf", node)) {}
#if __cplusplus >= 202002L
Broadcaster::Broadcaster(std::string_view name, async::Node &node) : _impl(std::make_unique<Impl>(name, "tf", node)) {}
#endif
Broadcaster::~Broadcaster() = default;

void Broadcaster::send(const msg::TransformStamped &transform) { sendOne(*_impl, transform); }
void Broadcaster::send(const msg::TF &transforms) { _impl->send(transforms); }
bool Broadcaster::invalid() const noexcept { return _impl->invalid(); }

class StaticBroadcaster::Impl final : public PublisherHandle {
public:
    using PublisherHandle::PublisherHandle;
};

StaticBroadcaster::StaticBroadcaster(std::string_view name, Node &node) : _impl(std::make_unique<Impl>(name, "tf_static", node)) {}
#if __cplusplus >= 202002L
StaticBroadcaster::StaticBroadcaster(std::string_view name, async::Node &node) : _impl(std::make_unique<Impl>(name, "tf_static", node)) {}
#endif
StaticBroadcaster::~StaticBroadcaster() = default;

void StaticBroadcaster::send(const msg::TransformStamped &transform) { sendOne(*_impl, transform); }
void StaticBroadcaster::send(const msg::TF &transforms) { _impl->send(transforms); }
bool StaticBroadcaster::invalid() const noexcept { return _impl->invalid(); }

class Listener::Impl final : public ListenerHandle {
public:
    using ListenerHandle::ListenerHandle;
};

Listener::Listener(std::string_view name, Node &node, Buffer &buffer) : _impl(std::make_unique<Impl>(name, node, buffer)) {}
#if __cplusplus >= 202002L
Listener::Listener(std::string_view name, async::Node &node, Buffer &buffer) : _impl(std::make_unique<Impl>(name, node, buffer)) {}
#endif
Listener::~Listener() = default;

bool Listener::invalid() const noexcept { return _impl->invalid(); }
Status Listener::status() const noexcept { return _impl->status(); }

} // namespace lpss::tf

} // namespace rm
