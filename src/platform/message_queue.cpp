#include "../../include/platform/message_queue.hpp"
#include <thread>

namespace frqs::platform {

// ============================================================================
// MESSAGE QUEUE IMPLEMENTATION
// ============================================================================

// Note: MessageQueue is fully implemented as a template in the header
// This file exists for any specialized implementations or helper functions

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Get hardware concurrency for optimal thread pool sizing
unsigned int getOptimalThreadCount() noexcept {
    unsigned int hwThreads = std::thread::hardware_concurrency();
    return hwThreads > 0 ? hwThreads : 4;  // Fallback to 4 if detection fails
}

// Create a thread pool for worker tasks (example utility)
class SimpleThreadPool {
private:
    std::vector<std::thread> workers_;
    MessageQueue<std::function<void()>> taskQueue_;
    bool running_ = true;

public:
    explicit SimpleThreadPool(size_t numThreads = getOptimalThreadCount()) {
        workers_.reserve(numThreads);
        
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this]() {
                workerLoop();
            });
        }
    }

    ~SimpleThreadPool() {
        stop();
    }

    // Non-copyable, non-movable
    SimpleThreadPool(const SimpleThreadPool&) = delete;
    SimpleThreadPool& operator=(const SimpleThreadPool&) = delete;

    void enqueue(std::function<void()> task) {
        if (running_) {
            taskQueue_.push(std::move(task));
        }
    }

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
    void workerLoop() {
        while (running_) {
            auto task = taskQueue_.waitPop(std::chrono::milliseconds(100));
            
            if (task.has_value()) {
                try {
                    (*task)();
                } catch (...) {
                    // Swallow exceptions in worker thread
                }
            }
        }
    }
};

// ============================================================================
// GLOBAL THREAD POOL (Optional utility)
// ============================================================================

SimpleThreadPool& getGlobalThreadPool() {
    static SimpleThreadPool pool;
    return pool;
}

// Helper: Post task to global thread pool
void postToThreadPool(std::function<void()> task) {
    getGlobalThreadPool().enqueue(std::move(task));
}

// ============================================================================
// PERFORMANCE METRICS (Optional debugging)
// ============================================================================

struct QueueMetrics {
    size_t totalPushed = 0;
    size_t totalPopped = 0;
    size_t currentSize = 0;
    size_t maxSize = 0;
};

// Track queue metrics (disabled in release builds)
#ifdef FRQS_DEBUG_METRICS
thread_local QueueMetrics g_queueMetrics;

void updatePushMetrics(size_t queueSize) {
    ++g_queueMetrics.totalPushed;
    g_queueMetrics.currentSize = queueSize;
    if (queueSize > g_queueMetrics.maxSize) {
        g_queueMetrics.maxSize = queueSize;
    }
}

void updatePopMetrics(size_t queueSize) {
    ++g_queueMetrics.totalPopped;
    g_queueMetrics.currentSize = queueSize;
}

const QueueMetrics& getQueueMetrics() {
    return g_queueMetrics;
}
#endif

} // namespace frqs::platform