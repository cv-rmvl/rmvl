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

#include "rmvl/core/util.hpp"

namespace rm::lpss {

template <typename MsgType>
void Publisher<MsgType>::publish(const MsgType &msg) {
    RMVL_Assert(!invalid());
    _writer->write(msg.serialize());
}

template <typename MsgType>
Publisher<MsgType> Node::createPublisher(std::string_view topic) noexcept {
    if (_local_writers.find(std::string(topic)) != _local_writers.end())
        return nullptr;
    Guid pub_guid = _uid;
    pub_guid.fields.entity = _next_eid.fetch_add(1, std::memory_order_relaxed);
    DataWriterBase::ptr writer = std::make_shared<DataWriter<MsgType>>(pub_guid, topic);
    // 设置 SHM 通道和 UDPv4 缓存
    {
        std::shared_lock lk(_discovered_mtx);
        auto it = _discovered_readers.find(std::string(topic));
        if (it != _discovered_readers.end())
            for (const auto &[reader_guid, locator] : it->second)
                writer->add(reader_guid, locator);
    }
    // 注册本地 DataWriter
    {
        std::lock_guard lk(_local_mtx);
        _local_writers[std::string(topic)] = writer;
    }
    // 向已发现的节点发送 addWriter 的 REDP 消息
    REDPMessage redp_msg = REDPMessage::addWriter(pub_guid, topic);
    std::shared_lock lk(_discovered_mtx);
    for (const auto &[guid, node_info] : _discovered_nodes)
        sendREDPMessage(node_info.ctrl_loc, redp_msg);
    return Publisher<MsgType>(topic, std::move(writer));
}

template <typename MsgType, typename SubscribeMsgCallback, typename Enable>
Subscriber<MsgType> Node::createSubscriber(std::string_view topic, SubscribeMsgCallback &&callback) noexcept {
    if (_local_readers.find(std::string(topic)) != _local_readers.end())
        return nullptr;
    Guid sub_guid = _uid;
    sub_guid.fields.entity = _next_eid.fetch_add(1, std::memory_order_relaxed);
    // 注册本地 DataReader
    DataReaderBase::ptr reader = std::make_shared<DataReader<MsgType>>(sub_guid, topic, callback);
    {
        std::lock_guard lk(_local_mtx);
        _local_readers[std::string(topic)] = reader;
    }
    // 向已发现的节点发送 addReader 的 REDP 消息
    REDPMessage redp_msg = REDPMessage::addReader(sub_guid, topic, reader->port());
    std::shared_lock lk(_discovered_mtx);
    for (const auto &[guid, node_info] : _discovered_nodes)
        sendREDPMessage(node_info.ctrl_loc, redp_msg);

    return Subscriber<MsgType>(topic, std::move(reader));
}

} // namespace rm::lpss
