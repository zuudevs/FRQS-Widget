/**
 * @file message_queue.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines a thread-safe, multiple-producer, single-consumer (MPSC) message queue.
 * @version 0.1
 * @date 2025-12-24
 *
 * @copyright Copyright (c) 2025
 *
 * This file contains the implementation of `frqs::platform::MessageQueue`, a template class
 * designed for safe cross-thread communication, primarily for passing messages or tasks
 * from multiple worker threads to a single consumer thread (e.g., the main UI thread).
 */

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

/**
 * @brief A thread-safe message queue for multiple producers and a single consumer.
 * @tparam T The type of message to store in the queue.
 *
 * This queue allows multiple threads to push messages concurrently, while a single
 * consumer thread can pop them. It is designed to be non-copyable and non-movable
 * to ensure safe handling of its internal synchronization primitives.
 */
template <typename T>
class MessageQueue {
private:
    mutable std::mutex mutex_;
    std::condition_variable condVar_;
    std::queue<T> queue_;
    bool closed_ = false;

public:
    /**
     * @brief Constructs a new MessageQueue.
     */
    MessageQueue() = default;

    // Non-copyable, non-movable
    MessageQueue(const MessageQueue&) = delete;
    MessageQueue(MessageQueue&&) = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;
    MessageQueue& operator=(MessageQueue&&) = delete;

    /**
     * @brief Destroys the MessageQueue, automatically closing it.
     */
    ~MessageQueue() noexcept {
        close();
    }

    /**
     * @brief Pushes a message into the queue using move semantics.
     * @param message The message to push.
     * @note This method is thread-safe.
     */
    void push(T&& message) {
        std::lock_guard lock(mutex_);
        if (closed_) return;  // Ignore messages after close

        queue_.push(std::move(message));
        condVar_.notify_one();
    }

    /**
     * @brief Pushes a message into the queue by copying it.
     * @param message The message to push.
     * @note This method is thread-safe.
     */
    void push(const T& message) {
        std::lock_guard lock(mutex_);
        if (closed_) return;

        queue_.push(message);
        condVar_.notify_one();
    }

    /**
     * @brief Tries to pop a message from the queue without blocking.
     * @return An `std::optional<T>` containing the message if the queue was not empty,
     *         otherwise `std::nullopt`.
     * @note This method is thread-safe and intended for polling.
     */
    [[nodiscard]] std::optional<T> tryPop() noexcept {
        std::lock_guard lock(mutex_);
        if (queue_.empty()) return std::nullopt;

        T message = std::move(queue_.front());
        queue_.pop();
        return message;
    }

    /**
     * @brief Waits for a message to become available or until a timeout occurs.
     * @tparam Rep The type of the duration representation.
     * @tparam Period The type of the duration period.
     * @param timeout The maximum duration to wait.
     * @return An `std::optional<T>` containing the message if one was popped,
     *         otherwise `std::nullopt` if the wait timed out or the queue was closed.
     * @note This method is thread-safe and blocking.
     */
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

    /**
     * @brief Processes all currently pending messages in the queue.
     * @tparam Handler A callable type that accepts a message of type T.
     * @param handler The function to call for each message.
     * @note The queue is unlocked during handler execution to allow producers to add new messages.
     */
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

    /**
     * @brief Checks if the queue is empty.
     * @return `true` if the queue has no messages, `false` otherwise.
     * @note This method is thread-safe.
     */
    [[nodiscard]] bool isEmpty() const noexcept {
        std::lock_guard lock(mutex_);
        return queue_.empty();
    }

    /**
     * @brief Gets the current number of messages in the queue.
     * @return The number of messages.
     * @note This method is thread-safe.
     */
    [[nodiscard]] size_t size() const noexcept {
        std::lock_guard lock(mutex_);
        return queue_.size();
    }

    /**
     * @brief Closes the queue, preventing new messages from being pushed.
     *
     * Once closed, any threads waiting on the queue are notified.
     * @note This method is thread-safe.
     */
    void close() noexcept {
        std::lock_guard lock(mutex_);
        closed_ = true;
        condVar_.notify_all();
    }

    /**
     * @brief Checks if the queue has been closed.
     * @return `true` if `close()` has been called, `false` otherwise.
     * @note This method is thread-safe.
     */
    [[nodiscard]] bool isClosed() const noexcept {
        std::lock_guard lock(mutex_);
        return closed_;
    }
};

// ============================================================================
// UI TASK TYPE (Function wrapper for worker->UI communication)
// ============================================================================

/**
 * @brief A type alias for a function wrapper representing a task to be executed on the UI thread.
 *
 * This is typically used for dispatching work from a worker thread to the main application thread.
 */
using UiTask = std::function<void()>;

/**
 * @brief A convenience type alias for a queue of UI tasks.
 */
using UiTaskQueue = MessageQueue<UiTask>;

} // namespace frqs::platform