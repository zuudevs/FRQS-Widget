/**
 * @file message_queue.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Provides implementation for thread-safe message queue utilities and a simple thread pool.
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "platform/message_queue.hpp"
#include <thread>
#include <vector>
#include <functional>
#include <chrono>

/**
 * @namespace frqs::platform
 * @brief Contains platform-specific implementations, including threading and message passing.
 */
namespace frqs::platform {

// ============================================================================
// MESSAGE QUEUE IMPLEMENTATION
// ============================================================================

// Note: MessageQueue is fully implemented as a template in the header
// This file exists for any specialized implementations or helper functions

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Get hardware concurrency for optimal thread pool sizing.
 * 
 * This function retrieves the number of concurrent threads supported by the hardware.
 * If detection fails, it returns a sensible default value.
 * 
 * @return unsigned int The recommended number of threads. Defaults to 4 if detection fails.
 * @noexcept
 */
unsigned int getOptimalThreadCount() noexcept {
    unsigned int hwThreads = std::thread::hardware_concurrency();
    return hwThreads > 0 ? hwThreads : 4;  // Fallback to 4 if detection fails
}

/**
 * @class SimpleThreadPool
 * @brief A basic thread pool for executing tasks concurrently.
 * 
 * This class manages a collection of worker threads and a queue of tasks.
 * Tasks (as `std::function<void()>`) can be enqueued and will be executed by the next available thread.
 */
class SimpleThreadPool {
private:
    std::vector<std::thread> workers_;       ///< Pool of worker threads.
    MessageQueue<std::function<void()>> taskQueue_; ///< Queue of tasks to be executed.
    bool running_ = true;                    ///< Flag to control the worker loop.

public:
    /**
     * @brief Constructs a SimpleThreadPool and starts worker threads.
     * @param numThreads The number of worker threads to create. Defaults to the optimal count from `getOptimalThreadCount()`.
     */
    explicit SimpleThreadPool(size_t numThreads = getOptimalThreadCount()) {
        workers_.reserve(numThreads);
        
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this]() {
                workerLoop();
            });
        }
    }

    /**
     * @brief Destroys the SimpleThreadPool, stopping and joining all worker threads.
     */
    ~SimpleThreadPool() {
        stop();
    }

    // Non-copyable, non-movable
    SimpleThreadPool(const SimpleThreadPool&) = delete;
    SimpleThreadPool& operator=(const SimpleThreadPool&) = delete;

    /**
     * @brief Enqueues a task to be executed by the thread pool.
     * @param task A `std::function<void()>` representing the task.
     */
    void enqueue(std::function<void()> task) {
        if (running_) {
            taskQueue_.push(std::move(task));
        }
    }

    /**
     * @brief Stops the thread pool and joins all worker threads.
     * 
     * This signals worker threads to stop, closes the task queue, and waits for all threads to finish.
     */
    void stop() {
        running_ = false;
        taskQueue_.close();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        workers_.clear();
    }

private:
    /**
     * @brief The main loop for each worker thread.
     * 
     * Waits for a task from the queue and executes it. The loop continues until the pool is stopped.
     */
    void workerLoop() {
        while (running_) {
            auto task = taskQueue_.waitPop(std::chrono::milliseconds(100));
            
            if (task.has_value()) {
                try {
                    (*task)();
                } catch (...) {
                    // Swallow exceptions in worker thread to prevent thread termination.
                }
            }
        }
    }
};

// ============================================================================
// GLOBAL THREAD POOL (Optional utility)
// ============================================================================

/**
 * @brief Provides access to a global singleton instance of `SimpleThreadPool`.
 * @return SimpleThreadPool& A reference to the global thread pool.
 */
SimpleThreadPool& getGlobalThreadPool() {
    static SimpleThreadPool pool;
    return pool;
}

/**
 * @brief Posts a task to the global thread pool for execution.
 * @param task The task to execute.
 */
void postToThreadPool(std::function<void()> task) {
    getGlobalThreadPool().enqueue(std::move(task));
}

// ============================================================================
// PERFORMANCE METRICS (Optional debugging)
// ============================================================================

/**
 * @struct QueueMetrics
 * @brief A simple structure to hold performance metrics for a message queue.
 * @note This is only active when `FRQS_DEBUG_METRICS` is defined.
 */
struct QueueMetrics {
    size_t totalPushed = 0; ///< Total number of items pushed to the queue.
    size_t totalPopped = 0; ///< Total number of items popped from the queue.
    size_t currentSize = 0; ///< The current number of items in the queue.
    size_t maxSize = 0;     ///< The maximum number of items the queue has held.
};

#ifdef FRQS_DEBUG_METRICS
/**
 * @brief Thread-local storage for queue metrics.
 */
thread_local QueueMetrics g_queueMetrics;

/**
 * @brief Updates metrics after a push operation.
 * @param queueSize The new size of the queue.
 */
void updatePushMetrics(size_t queueSize) {
    ++g_queueMetrics.totalPushed;
    g_queueMetrics.currentSize = queueSize;
    if (queueSize > g_queueMetrics.maxSize) {
        g_queueMetrics.maxSize = queueSize;
    }
}

/**
 * @brief Updates metrics after a pop operation.
 * @param queueSize The new size of the queue.
 */
void updatePopMetrics(size_t queueSize) {
    ++g_queueMetrics.totalPopped;
    g_queueMetrics.currentSize = queueSize;
}

/**
 * @brief Retrieves the performance metrics for the current thread's queue.
 * @return const QueueMetrics& A reference to the metrics data.
 */
const QueueMetrics& getQueueMetrics() {
    return g_queueMetrics;
}
#endif

} // namespace frqs::platform