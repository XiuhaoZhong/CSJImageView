#pragma once

#include <vector>
#include <optional>
#include <condition_variable>
#include <mutex>
#include <chrono>

#include "CSJLogger.h"

using cond_va = std::condition_variable;
using namespace std::chrono_literals;

namespace csjutils {
    template <typename T>
    class CSJRingQueue {
    public:
        explicit CSJRingQueue(size_t capacity) 
            : m_pBuffer(capacity + 1) {

        }

        ~CSJRingQueue() {
            LOG_Debug("A CSJRingQueue instance is deconstructed!");
        }

        void enBuffer(T val) {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cvFull.wait(lock, [this] {
                return !is_full() || m_bWakeupForExit;
            });

            if (m_bWakeupForExit) {
                lock.unlock();
                return ;
            }

            m_pBuffer[m_iRear] = std::move(val);
            m_iRear = (m_iRear + 1) % m_pBuffer.size();
            m_cvEmpty.notify_one();
        }

        std::optional<T> deBuffer(std::chrono::milliseconds timeout = 10ms) {
            std::unique_lock<std::mutex> lock(m_mtx);

            bool data_ready = m_cvEmpty.wait_for(lock, timeout, [this] {
                return !is_empty() || m_bWakeupForExit;
            });

            if (!data_ready || m_bWakeupForExit) {
                lock.unlock();
                return std::nullopt;
            }

            T val = std::move(m_pBuffer[m_iFront]);
            m_iFront = (m_iFront + 1) % m_pBuffer.size();
            m_cvFull.notify_one();

            return val;
        }

        bool is_empty() const {
            return m_iFront == m_iRear;
        }

        bool is_full() const {
            if (m_pBuffer.size() == 0) {
                return false;
            }

            return (m_iRear + 1) % m_pBuffer.size() == m_iFront;
        }

        void wakeUpToExit() {
            m_bWakeupForExit = true;
            m_cvEmpty.notify_all();
            m_cvFull.notify_all();
        }

        /**
         * @brief The reset function must be called after all the threads that use the
         *        queue exits.
         */
        void reset() {
            //m_pBuffer.clear();
            m_iFront = 0;
            m_iRear = 0;
            m_bWakeupForExit = false;
        }


    private:
        std::vector<T>     m_pBuffer;
        size_t             m_iFront = 0;
        size_t             m_iRear = 0;
        mutable std::mutex m_mtx;
        cond_va            m_cvEmpty;
        cond_va            m_cvFull;
        bool               m_bWakeupForExit = false;

    };
}