#pragma once

#include <vector>
#include <atomic>
#include <optional>
#include <cstddef>

namespace csjutils {
    template <typename T>
    class CSJEffRingQueue {
    public:
        explicit CSJEffRingQueue(size_t capacity)
            : m_pBuffer(capacity + 1)
            , m_iCapacity(capacity) {

        }

        bool enQueue(T && val) {
            const size_t rear = m_iRear.load(std::memory_order_relaxed);
            const size_t front = m_iFront.load(std::memory_order_acquire);

            if ((rear + 1) % m_pBuffer.size() == front) {
                // TODO: When the queue is full, users need to deal with the detail logic.
                return false;
            }

            m_pBuffer[rear] = std::move(val);
            m_iRear.store((rear + 1) % m_pBuffer.size(), std::memory_order_release);
            return true;
        }

        std::optional<T> deQueue() {
            const size_t front = m_iFront.load(std::memory_order_relaxed);
            const size_t rear = m_iRear.load(std::memory_order_acquire);

            if (front == rear) {
                return std::nullopt;
            }

            T val = std::move(m_pBuffer[front]);
            m_iFront.store((front + 1) % m_pBuffer.size(), std::memory_order_release);
            return val;
        }

        bool is_empty() const {
            return m_iFront.load(std::memory_order_acquire) == 
                    m_iRear.load(std::memory_order_acquire);
        }

        bool is_full() const {
            const size_t rear = m_iRear.load(std::memory_order_acquire);
            const size_t front = m_iFront.load(std::memory_order_acquire);
            return (rear + 1) % m_pBuffer.size() == front;
        }

        size_t size() const {
            const size_t rear = m_iRear.load(std::memory_order_acquire);
            const size_t front = m_iFront.load(std::memory_order_acquire);
            return (rear - front + m_pBuffer.size()) % m_pBuffer.size();
        }

        size_t capacity() const {
            return m_iCapacity;
        }

    private:
        std::vector<T>      m_pBuffer;
        size_t              m_iCapacity;

        std::atomic<size_t> m_iFront{0};
        std::atomic<size_t> m_iRear{0};
    };
}