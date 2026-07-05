/**
 * @file node_impl.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief LPSS 节点实现
 * @version 1.0
 * @date 2025-12-29
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#include "rmvl/lpss/node.hpp"

#include "node_rstp.hpp"
#include "rmvl/core/util.hpp"

namespace rm::lpss {

//! @cond

template <typename MsgType>
void Publisher<MsgType>::publish(const MsgType &msg) {
    RMVL_Assert(!invalid());
    _writer->write(msg.serialize());
}

template <typename MsgType>
Publisher<MsgType> Node::createPublisher(std::string_view topic) noexcept {
    if (topic.size() > 63 || std::string_view(MsgType::msg_type).size() > 63) {
        WARNING_("[LPSS Node] MTP limits topic and message type names to 63 bytes");
        return nullptr;
    }
    if (_local_writers.find(std::string(topic)) != _local_writers.end())
        return nullptr;
    Guid pub_guid = _uid;
    pub_guid.set_entity(_next_eid.fetch_add(1, std::memory_order_relaxed));
    DataWriterBase::ptr writer = std::make_shared<DataWriter<MsgType>>(pub_guid, topic);
    // 设置 SHM 通道和 UDPv4 缓存
    {
        std::shared_lock lk(_discovered_mtx);
        auto it = _discovered_readers.find(std::string(topic));
        if (it != _discovered_readers.end())
            for (const auto &[reader_guid, locator] : it->second.readers)
                writer->add(reader_guid, locator);
    }
    // 注册本地 DataWriter
    {
        std::lock_guard lk(_local_mtx);
        _local_writers[std::string(topic)] = writer;
    }
    // 向已发现的节点发送 addWriter 的 EDP 消息
    REDPMessage redp_msg = REDPMessage::addWriter(pub_guid, topic, writer->msgtype());
    std::shared_lock lk(_discovered_mtx);
    for (const auto &discovered_node : _discovered_nodes)
        sendREDPMessage(discovered_node.second.ctrl_loc, redp_msg);
    return Publisher<MsgType>(topic, std::move(writer));
}

template <typename MsgType, typename SubscribeMsgCallback, typename Enable>
Subscriber<MsgType> Node::createSubscriber(std::string_view topic, SubscribeMsgCallback &&callback) noexcept {
    if (topic.size() > 63 || std::string_view(MsgType::msg_type).size() > 63) {
        WARNING_("[LPSS Node] MTP limits topic and message type names to 63 bytes");
        return nullptr;
    }
    if (_local_readers.find(std::string(topic)) != _local_readers.end())
        return nullptr;
    Guid sub_guid = _uid;
    sub_guid.set_entity(_next_eid.fetch_add(1, std::memory_order_relaxed));
    // 注册本地 DataReader
    DataReaderBase::ptr reader = std::make_shared<DataReader<MsgType>>(sub_guid, topic, callback);
    {
        std::shared_lock lk(_discovered_mtx);
        auto it = _discovered_writers.find(std::string(topic));
        if (it != _discovered_writers.end())
            for (const auto &writer_guid : it->second.writers)
                reader->add(writer_guid);
    }
    {
        std::lock_guard lk(_local_mtx);
        _local_readers[std::string(topic)] = reader;
    }
    // 向已发现的节点发送 addReader 的 EDP 消息
    REDPMessage redp_msg = REDPMessage::addReader(sub_guid, topic, reader->port(), reader->msgtype());
    std::shared_lock lk(_discovered_mtx);
    for (const auto &discovered_node : _discovered_nodes)
        sendREDPMessage(discovered_node.second.ctrl_loc, redp_msg);

    return Subscriber<MsgType>(topic, std::move(reader));
}

template <typename MsgType>
void Node::destroyPublisher(Publisher<MsgType> &pub) {
    if (pub.invalid())
        return;
    // 移除本地 DataWriter
    {
        std::lock_guard lk(_local_mtx);
        _local_writers.erase(pub._topic);
    }
    // 向已发现的节点发送 removeWriter 的 EDP 消息
    REDPMessage redp_msg = REDPMessage::removeWriter(pub._writer->guid(), pub._topic);
    std::shared_lock lk(_discovered_mtx);
    for (const auto &discovered_node : _discovered_nodes)
        sendREDPMessage(discovered_node.second.ctrl_loc, redp_msg);
    // 释放资源
    pub._writer.reset();
}

template <typename MsgType>
void Node::destroySubscriber(Subscriber<MsgType> &sub) {
    if (sub.invalid())
        return;
    // 移除本地 DataReader
    {
        std::lock_guard lk(_local_mtx);
        _local_readers.erase(sub._topic);
    }
    // 向已发现的节点发送 removeReader 的 EDP 消息
    REDPMessage redp_msg = REDPMessage::removeReader(sub._reader->guid(), sub._topic);
    std::shared_lock lk(_discovered_mtx);
    for (const auto &discovered_node : _discovered_nodes)
        sendREDPMessage(discovered_node.second.ctrl_loc, redp_msg);
    // 释放资源
    sub._reader.reset();
}

#if __cplusplus >= 202002L

namespace async {

template <typename MsgType>
void Publisher<MsgType>::publish(const MsgType &msg) {
    RMVL_Assert(!invalid());
    co_spawn(_ctx, &DataWriterBase::write, _writer, msg.serialize());
}

template <typename MsgType>
typename Publisher<MsgType>::ptr Node::createPublisher(std::string_view topic) noexcept {
    if (topic.size() > 63 || std::string_view(MsgType::msg_type).size() > 63) {
        WARNING_("[LPSS Node] MTP limits topic and message type names to 63 bytes");
        return nullptr;
    }
    if (_local_writers.find(std::string(topic)) != _local_writers.end())
        return nullptr;
    Guid pub_guid = _uid;
    pub_guid.set_entity(_next_eid++);
    DataWriterBase::ptr writer = std::make_shared<DataWriter<MsgType>>(_ctx, pub_guid, topic);
    // 设置 SHM 通道和 UDPv4 缓存
    auto it = _discovered_readers.find(std::string(topic));
    if (it != _discovered_readers.end())
        for (const auto &[reader_guid, locator] : it->second.readers)
            writer->add(reader_guid, locator);
    // 注册本地 DataWriter
    _local_writers[std::string(topic)] = writer;
    // 向已发现的节点发送 addWriter 的 EDP 消息
    REDPMessage redp_msg = REDPMessage::addWriter(pub_guid, topic, writer->msgtype());
    for (const auto &discovered_node : _discovered_nodes)
        sendREDPMessage(discovered_node.second.ctrl_loc, redp_msg);
    return std::make_shared<Publisher<MsgType>>(_ctx, topic, std::move(writer));
}

template <typename MsgType, typename SubscribeMsgCallback, typename Enable>
typename Subscriber<MsgType>::ptr Node::createSubscriber(std::string_view topic, SubscribeMsgCallback callback) noexcept {
    if (topic.size() > 63 || std::string_view(MsgType::msg_type).size() > 63) {
        WARNING_("[LPSS Node] MTP limits topic and message type names to 63 bytes");
        return nullptr;
    }
    if (_local_readers.find(std::string(topic)) != _local_readers.end())
        return nullptr;
    Guid sub_guid = _uid;
    sub_guid.set_entity(_next_eid++);
    // 注册本地 DataReader
    auto typed_reader = std::make_shared<DataReader<MsgType>>(_ctx, sub_guid, topic, std::move(callback));
    DataReaderBase::ptr reader = typed_reader;
    auto writer_it = _discovered_writers.find(std::string(topic));
    if (writer_it != _discovered_writers.end())
        for (const auto &writer_guid : writer_it->second.writers)
            reader->add(writer_guid);
    _local_readers[std::string(topic)] = reader;
    typed_reader->start();
    // 向已发现的节点发送 addReader 的 EDP 消息
    REDPMessage redp_msg = REDPMessage::addReader(sub_guid, topic, reader->port(), reader->msgtype());
    for (const auto &discovered_node : _discovered_nodes)
        sendREDPMessage(discovered_node.second.ctrl_loc, redp_msg);

    return std::make_shared<Subscriber<MsgType>>(_ctx, topic, std::move(reader));
}

template <typename SrvType>
void Service<SrvType>::_delay_start() {
    if (_started)
        return;
    _started = true;
    co_spawn(_ctx, &Service<SrvType>::serve, this->shared_from_this());
}

template <typename SrvType>
rm::async::Task<> Service<SrvType>::serve() {
    while (true) {
        auto data = co_await _lq_reader->read();
        auto request = stp::unpack<Request>(data);
        if (!request)
            continue;
        try {
            auto response = _callback(request->message);
            co_await _lr_writer->write(stp::pack(request->header, response));
        } catch (...) {
            WARNING_("[LPSS Service] Service callback for '%s' threw an exception", _service.c_str());
        }
    }
}

template <typename SrvType>
void Client<SrvType>::_delay_start() {
    if (_started)
        return;
    _started = true;
    co_spawn(_ctx, &Client<SrvType>::receive, this->shared_from_this());
}

template <typename SrvType>
rm::async::Task<> Client<SrvType>::receive() {
    while (true) {
        auto data = co_await _response_reader->read();
        auto response = stp::unpack<Response>(data);
        if (!response || !_calling || response->header.client_guid != _request_writer->guid() ||
            response->header.sequence != _waiting_sequence)
            continue;
        _response = std::move(response->message);
    }
}

template <typename SrvType>
template <typename Rep, typename Period>
auto Client<SrvType>::call(const Request &request, std::chrono::duration<Rep, Period> timeout) -> rm::async::Task<std::optional<Response>> {
    if (invalid() || _calling)
        co_return std::nullopt;

    _calling = true;
    _response.reset();
    _waiting_sequence = _next_sequence++;
    stp::Header header{_request_writer->guid(), _waiting_sequence};
    co_await _request_writer->write(stp::pack(header, request));

    rm::async::Timer timer(_ctx);
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!_response && std::chrono::steady_clock::now() < deadline)
        co_await timer.sleep_for(std::chrono::milliseconds(1));

    auto response = std::move(_response);
    _response.reset();
    _calling = false;
    co_return response;
}

template <typename SrvType, typename ServiceCallback, typename Enable>
typename Service<SrvType>::ptr Node::createService(std::string_view service, ServiceCallback callback) noexcept {
    using Request = typename SrvType::Request;
    using Response = typename SrvType::Response;
    const auto request_topic = srv2topic::request(service);
    const auto response_topic = srv2topic::response(service);
    if (service.empty() || request_topic.size() > 63 || response_topic.size() > 63 ||
        std::string_view(Request::msg_type).size() > 63 || std::string_view(Response::msg_type).size() > 63) {
        WARNING_("[LPSS Node] MTP limits service names and message type names to 63 bytes");
        return nullptr;
    }
    if (_local_readers.find(request_topic) != _local_readers.end() || _local_writers.find(response_topic) != _local_writers.end())
        return nullptr;

    Guid request_guid = _uid;
    request_guid.set_entity(_next_eid++);
    Guid response_guid = _uid;
    response_guid.set_entity(_next_eid++);
    auto request_reader = std::make_shared<DataReaderBase>(_ctx, request_guid, Request::msg_type, request_topic);
    auto response_writer = std::make_shared<DataWriterBase>(_ctx, response_guid, Response::msg_type, response_topic);

    if (auto it = _discovered_writers.find(request_topic); it != _discovered_writers.end())
        for (const auto &writer_guid : it->second.writers)
            request_reader->add(writer_guid);
    if (auto it = _discovered_readers.find(response_topic); it != _discovered_readers.end())
        for (const auto &[reader_guid, locator] : it->second.readers)
            response_writer->add(reader_guid, locator);

    _local_readers[request_topic] = request_reader;
    _local_writers[response_topic] = response_writer;
    const auto add_request = REDPMessage::addReader(request_guid, request_topic, request_reader->port(), Request::msg_type);
    const auto add_response = REDPMessage::addWriter(response_guid, response_topic, Response::msg_type);
    for (const auto &[guid, node] : _discovered_nodes) {
        sendREDPMessage(node.ctrl_loc, add_request);
        sendREDPMessage(node.ctrl_loc, add_response);
    }

    auto result = typename Service<SrvType>::ptr(new Service<SrvType>(_ctx, service, std::move(request_reader), std::move(response_writer),
                                                                      typename Service<SrvType>::Callback(std::move(callback))));
    result->_delay_start();
    return result;
}

template <typename SrvType>
typename Client<SrvType>::ptr Node::createClient(std::string_view service) noexcept {
    using Request = typename SrvType::Request;
    using Response = typename SrvType::Response;
    const auto request_topic = srv2topic::request(service);
    const auto response_topic = srv2topic::response(service);
    if (service.empty() || request_topic.size() > 63 || response_topic.size() > 63 ||
        std::string_view(Request::msg_type).size() > 63 || std::string_view(Response::msg_type).size() > 63) {
        WARNING_("[LPSS Node] MTP limits service names and message type names to 63 bytes");
        return nullptr;
    }
    if (_local_writers.find(request_topic) != _local_writers.end() || _local_readers.find(response_topic) != _local_readers.end())
        return nullptr;

    Guid request_guid = _uid;
    request_guid.set_entity(_next_eid++);
    Guid response_guid = _uid;
    response_guid.set_entity(_next_eid++);
    auto request_writer = std::make_shared<DataWriterBase>(_ctx, request_guid, Request::msg_type, request_topic);
    auto response_reader = std::make_shared<DataReaderBase>(_ctx, response_guid, Response::msg_type, response_topic);

    if (auto it = _discovered_readers.find(request_topic); it != _discovered_readers.end())
        for (const auto &[reader_guid, locator] : it->second.readers)
            request_writer->add(reader_guid, locator);
    if (auto it = _discovered_writers.find(response_topic); it != _discovered_writers.end())
        for (const auto &writer_guid : it->second.writers)
            response_reader->add(writer_guid);

    _local_writers[request_topic] = request_writer;
    _local_readers[response_topic] = response_reader;
    const auto add_request = REDPMessage::addWriter(request_guid, request_topic, Request::msg_type);
    const auto add_response = REDPMessage::addReader(response_guid, response_topic, response_reader->port(), Response::msg_type);
    for (const auto &[guid, node] : _discovered_nodes) {
        sendREDPMessage(node.ctrl_loc, add_request);
        sendREDPMessage(node.ctrl_loc, add_response);
    }

    auto result = typename Client<SrvType>::ptr(new Client<SrvType>(_ctx, service, std::move(request_writer), std::move(response_reader)));
    result->_delay_start();
    return result;
}

template <typename MsgType>
void Node::destroyPublisher(std::shared_ptr<Publisher<MsgType>> pub) {
    if (!pub || pub->invalid())
        return;
    // 移除本地 DataWriter
    _local_writers.erase(pub->_topic);
    // 向已发现的节点发送 removeWriter 的 EDP 消息
    REDPMessage redp_msg = REDPMessage::removeWriter(pub->_writer->guid(), pub->_topic);
    for (const auto &discovered_node : _discovered_nodes)
        sendREDPMessage(discovered_node.second.ctrl_loc, redp_msg);
    // 释放资源
    pub->_writer.reset();
}

template <typename MsgType>
void Node::destroySubscriber(std::shared_ptr<Subscriber<MsgType>> sub) {
    if (!sub || sub->invalid())
        return;
    sub->_reader->stop();
    // 移除本地 DataReader
    _local_readers.erase(sub->_topic);
    // 向已发现的节点发送 removeReader 的 EDP 消息
    REDPMessage redp_msg = REDPMessage::removeReader(sub->_reader->guid(), sub->_topic);
    for (const auto &discovered_node : _discovered_nodes)
        sendREDPMessage(discovered_node.second.ctrl_loc, redp_msg);
    // 释放资源
    sub->_reader.reset();
}

template <typename Rep, typename Period, typename TimerCallback>
static rm::async::Task<> timer_task(Timer::ptr timer, std::chrono::duration<Rep, Period> dur, TimerCallback cb) {
    auto next_time = std::chrono::steady_clock::now();
    try {
        while (true) {
            next_time += dur;
            if (next_time < std::chrono::steady_clock::now())
                next_time = std::chrono::steady_clock::now();
            co_await timer->sleep_until(next_time);
            cb();
        }
    } catch (...) {
    }
    co_return;
}

template <typename Rep, typename Period, typename TimerCallback>
Timer::ptr Node::createTimer(std::chrono::duration<Rep, Period> dur, TimerCallback cb) noexcept {
    auto timer = std::make_shared<Timer>(_ctx);
    co_spawn(_ctx, timer_task<Rep, Period, TimerCallback>, timer, dur, std::move(cb));

    return timer;
}

} // namespace async

#endif

//! @endcond

} // namespace rm::lpss
