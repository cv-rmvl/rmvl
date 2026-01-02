/**
 * @file shm.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 共享内存实现细节
 * @version 1.0
 * @date 2025-07-31
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

/**
 * @brief MPMC 原子共享内存对象
 *
 * @tparam T 共享内存数据类型
 */
template <typename T, typename Enable>
AtomicSHM<T, Enable>::AtomicSHM(std::string_view name) : SHMBase(name, sizeof(Layout)) {
    if (this->data() && this->isCreator())
        new (this->data()) Layout();
}

template <typename T, typename Enable>
bool AtomicSHM<T, Enable>::read(T &value) noexcept {
    auto layout = static_cast<Layout *>(this->data());
    if (!layout)
        return false;
    int retry{};
    while (true) {
        uint32_t s1 = layout->seq.load(std::memory_order_acquire);
        if (s1 == 0)
            return false;

        if (!(s1 & 1)) {
            value = layout->data;
            std::atomic_thread_fence(std::memory_order_acquire); // 读屏障
            uint32_t s2 = layout->seq.load(std::memory_order_relaxed);
            if (s1 == s2)
                return true;
        }
        if (retry++ > 100)
            std::this_thread::yield();
    }
}

template <typename T, typename Enable>
void AtomicSHM<T, Enable>::write(const T &value) noexcept {
    auto layout = static_cast<Layout *>(this->data());
    if (!layout)
        return;
    // 获取写锁 (防止多写者竞争)
    while (layout->writer_mtx.test_and_set(std::memory_order_acquire))
        std::this_thread::yield();

    uint32_t s = layout->seq.load(std::memory_order_relaxed);
    // 开始写
    layout->seq.store(s + 1, std::memory_order_release);
    layout->data = value;
    layout->seq.store(s + 2, std::memory_order_release);
    // 释放写锁
    layout->writer_mtx.clear(std::memory_order_release);
}

template <typename T, typename Enable>
bool AtomicSHM<T, Enable>::empty() const noexcept {
    auto layout = static_cast<const Layout *>(this->data());
    if (!layout)
        return true;
    return layout->seq.load(std::memory_order_relaxed) == 0;
}

template <typename T, size_t Capacity, typename Enable>
RingBufferSlotSHM<T, Capacity, Enable>::RingBufferSlotSHM(std::string_view name) : SHMBase(name, sizeof(Buffer)) {
    if (this->data() && this->isCreator()) {
        auto buf = static_cast<Buffer *>(this->data());
        new (&buf->head) std::atomic_size_t(0);
        new (&buf->tail) std::atomic_size_t(0);
        for (size_t i = 0; i < Capacity; ++i)
            new (&buf->slots[i].sequence) std::atomic_size_t(i);
    }
}

template <typename T, size_t Capacity, typename Enable>
bool RingBufferSlotSHM<T, Capacity, Enable>::write(const T &value, bool overwrite) noexcept {
    auto buf = static_cast<Buffer *>(this->data());
    if (!buf)
        return false;
    size_t current_tail = buf->tail.load(std::memory_order_relaxed);

    while (true) {
        Slot &slot = buf->slots[current_tail & (Capacity - 1)];
        size_t seq = slot.sequence.load(std::memory_order_acquire);

        intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(current_tail);
        if (diff == 0) {
            if (buf->tail.compare_exchange_weak(current_tail, current_tail + 1, std::memory_order_relaxed)) {
                slot.data = value;
                slot.sequence.store(current_tail + 1, std::memory_order_release);
                return true;
            }
        } else if (diff < 0) {
            if (!overwrite)
                return false; // 缓冲区已满，且不覆盖旧数据
            size_t current_head = buf->head.load(std::memory_order_relaxed);
            Slot &head_slot = buf->slots[current_head & (Capacity - 1)];
            size_t head_seq = head_slot.sequence.load(std::memory_order_acquire);

            intptr_t head_diff = static_cast<intptr_t>(head_seq) - static_cast<intptr_t>(current_head + 1);
            if (head_diff == 0)
                if (buf->head.compare_exchange_weak(current_head, current_head + 1, std::memory_order_relaxed))
                    head_slot.sequence.store(current_head + Capacity, std::memory_order_release);
        } else
            current_tail = buf->tail.load(std::memory_order_relaxed);
    }
}

template <typename T, size_t Capacity, typename Enable>
bool RingBufferSlotSHM<T, Capacity, Enable>::read(T &value) noexcept {
    auto buf = static_cast<Buffer *>(this->data());
    if (!buf)
        return false;

    size_t current_head;
    current_head = buf->head.load(std::memory_order_relaxed);

    while (true) {
        Slot &slot = buf->slots[current_head & (Capacity - 1)];
        size_t seq = slot.sequence.load(std::memory_order_acquire);

        intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(current_head + 1);
        if (diff == 0) {
            if (buf->head.compare_exchange_weak(current_head, current_head + 1, std::memory_order_relaxed)) {
                value = slot.data;
                slot.sequence.store(current_head + Capacity, std::memory_order_release);
                return true;
            }
        } else if (diff < 0)
            return false; // 缓冲区为空
        else
            current_head = buf->head.load(std::memory_order_relaxed);
    }
}
