#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <optional>
#include <chrono>

namespace frqs::platform {

// ============================================================================
// THREAD-SAFE MESSAGE QUEUE (MPSC: Multiple Producer, Single Consumer)
// ============================================================================

template <typename T>
class MessageQueue {
private:
    mutable std::mutex mutex_;
    std::condition_variable condVar_;
    std::queue<T> queue_;
    bool closed_ = false;

public:
    MessageQueue() = default;

    // Non-copyable, non-movable
    MessageQueue(const MessageQueue&) = delete;
    MessageQueue(MessageQueue&&) = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;
    MessageQueue& operator=(MessageQueue&&) = delete;

    ~MessageQueue() noexcept {
        close();
    }

    // Push a message (from worker threads)
    void push(T&& message) {
        std::lock_guard lock(mutex_);
        if (closed_) return;  // Ignore messages after close
        
        queue_.push(std::move(message));
        condVar_.notify_one();
    }

    void push(const T& message) {
        std::lock_guard lock(mutex_);
        if (closed_) return;
        
        queue_.push(message);
        condVar_.notify_one();
    }

    // Try to pop a message (non-blocking, for UI thread polling)
    [[nodiscard]] std::optional<T> tryPop() noexcept {
        std::lock_guard lock(mutex_);
        if (queue_.empty()) return std::nullopt;
        
        T message = std::move(queue_.front());
        queue_.pop();
        return message;
    }

    // Pop a message (blocking with timeout, for UI thread)
    template <typename Rep, typename Period>
    [[nodiscard]] std::optional<T> waitPop(
        const std::chrono::duration<Rep, Period>& timeout
    ) {
        std::unique_lock lock(mutex_);
        
        if (!condVar_.wait_for(lock, timeout, [this] { 
            return !queue_.empty() || closed_; 
        })) {
            return std::nullopt;  // Timeout
        }
        
        if (queue_.empty()) return std::nullopt;  // Closed without messages
        
        T message = std::move(queue_.front());
        queue_.pop();
        return message;
    }

    // Process all pending messages (for UI thread batch processing)
    template <typename Handler>
    void processAll(Handler&& handler) {
        std::unique_lock lock(mutex_);
        
        while (!queue_.empty()) {
            T message = std::move(queue_.front());
            queue_.pop();
            
            // Unlock during handler execution to allow producers to continue
            lock.unlock();
            handler(std::move(message));
            lock.lock();
        }
    }

    // Check if queue has messages
    [[nodiscard]] bool isEmpty() const noexcept {
        std::lock_guard lock(mutex_);
        return queue_.empty();
    }

    // Get queue size
    [[nodiscard]] size_t size() const noexcept {
        std::lock_guard lock(mutex_);
        return queue_.size();
    }

    // Close the queue (stop accepting new messages)
    void close() noexcept {
        std::lock_guard lock(mutex_);
        closed_ = true;
        condVar_.notify_all();
    }

    // Check if closed
    [[nodiscard]] bool isClosed() const noexcept {
        std::lock_guard lock(mutex_);
        return closed_;
    }
};

// ============================================================================
// UI TASK TYPE (Function wrapper for worker->UI communication)
// ============================================================================

using UiTask = std::function<void()>;

// Convenience type alias for UI task queue
using UiTaskQueue = MessageQueue<UiTask>;

} // namespace frqs::platform