#pragma once

#include <memory>
#include <functional>
#include <chrono>
#include "window.hpp"
#include "window_registry.hpp"
#include "platform/message_queue.hpp"

namespace frqs::core {

// ============================================================================
// APPLICATION (Singleton Lifecycle Manager)
// ============================================================================

class Application {
private:
    struct Impl;  // PImpl to hide platform details
    std::unique_ptr<Impl> pImpl_;
    
    bool running_ = false;
    platform::UiTaskQueue taskQueue_;  // Worker->UI communication

    Application();

public:
    ~Application() noexcept;

    // Singleton access
    static Application& instance() noexcept {
        static Application app;
        return app;
    }

    // Non-copyable, non-movable
    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;

    // ========================================================================
    // LIFECYCLE MANAGEMENT
    // ========================================================================

    // Initialize the application (call once before creating windows)
    void initialize();

    // Run the main event loop (blocking)
    void run();

    // Stop the main loop (can be called from any thread)
    void quit() noexcept;

    // Check if app is running
    [[nodiscard]] bool isRunning() const noexcept { return running_; }

    // ========================================================================
    // WINDOW MANAGEMENT
    // ========================================================================

    // Create a new window
    [[nodiscard]] std::shared_ptr<Window> createWindow(
        const WindowParams& params = WindowParams{}
    );

    // Get window by ID
    [[nodiscard]] std::shared_ptr<Window> getWindow(WindowId id) const;

    // Close a specific window
    void closeWindow(WindowId id);

    // Close all windows
    void closeAllWindows();

    // Get window registry (for advanced use)
    [[nodiscard]] WindowRegistry& getRegistry() noexcept {
        return WindowRegistry::instance();
    }

    // ========================================================================
    // WORKER THREAD COMMUNICATION (Thread-Safe)
    // ========================================================================

    // Post a task to be executed on the UI thread
    // Workers MUST use this to modify UI state
    void postToUiThread(platform::UiTask task) {
        taskQueue_.push(std::move(task));
    }

    // Post a delayed task (executed after duration)
    template <typename Rep, typename Period>
    void postDelayed(
        platform::UiTask task,
        const std::chrono::duration<Rep, Period>& delay
    );

    // ========================================================================
    // EVENT LOOP CONTROL
    // ========================================================================

    // Process pending UI tasks (called internally by event loop)
    void processPendingTasks();

    // Single iteration of event loop (for custom loop integration)
    bool pollEvents();

    // ========================================================================
    // RENDER CONTROL
    // ========================================================================

    // Request render for all windows (next frame)
    void requestRender();

    // Request render for specific window
    void requestRender(WindowId id);

    // Set target FPS (0 = unlimited, default: 60)
    void setTargetFps(uint32_t fps) noexcept;

    [[nodiscard]] uint32_t getTargetFps() const noexcept;

private:
    // Main event loop implementation
    void runMainLoop();

    // Process window messages
    void processWindowMessages();

    // Render all dirty windows
    void renderWindows();
};

} // namespace frqs::core

// ============================================================================
// CONVENIENCE MACROS
// ============================================================================

#define FRQS_APP frqs::core::Application::instance()

// Post to UI thread from worker
#define FRQS_POST_UI(code) \
    FRQS_APP.postToUiThread([=]() { code; })

// Post to UI with capture
#define FRQS_POST_UI_CAPTURE(captures, code) \
    FRQS_APP.postToUiThread([captures]() { code; })