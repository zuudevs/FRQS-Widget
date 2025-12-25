/**
 * @file application.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Header the Application class for lifecycle and window management.
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <memory>
#include <chrono>
#include "window.hpp"
#include "window_registry.hpp"
#include "platform/message_queue.hpp"

namespace frqs::core {

// ============================================================================
// APPLICATION (Singleton Lifecycle Manager)
// ============================================================================

/**
 * @class Application
 * @brief Manages the application's lifecycle, windows, and event loop.
 *
 * This class follows the Singleton pattern to provide a central point of control
 * for the entire application. It is responsible for:
 * - Initializing and shutting down the application.
 * - Running the main event loop.
 * - Creating and managing windows.
 * - Facilitating communication between worker threads and the UI thread.
 *
 * @note This class is not thread-safe for all operations. UI-related
 *       modifications must be done on the main UI thread, typically by
 *       posting tasks via `postToUiThread`.
 */
class Application {
private:
    struct Impl;  // PImpl to hide platform details
    std::unique_ptr<Impl> pImpl_;
    
    bool running_ = false;
    platform::UiTaskQueue taskQueue_;  // Worker->UI communication

    Application();

public:
    ~Application() noexcept;

    /**
     * @brief Gets the singleton instance of the Application.
     * @return A reference to the single Application instance.
     */
    static Application& instance() noexcept {
        static Application app;
        return app;
    }

	/**
     * @brief Convenience function to create and get the singleton instance.
     * @return A reference to the single Application instance.
     */
	static Application& create() noexcept {
		return instance() ;
	}

    // Non-copyable, non-movable
    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;

    // ========================================================================
    // LIFECYCLE MANAGEMENT
    // ========================================================================

    /**
     * @brief Initializes the application and its underlying systems.
     * @note Must be called once before creating any windows or running the loop.
     */
    void initialize();

    /**
     * @brief Runs the main application event loop.
     * @note This is a blocking call that will only return when `quit()` is called.
     */
    void run();

    /**
     * @brief Stops the main event loop and signals the application to shut down.
     * @note This method is thread-safe and can be called from any thread.
     */
    void quit() noexcept;

    /**
     * @brief Checks if the application's main loop is currently running.
     * @return `true` if the application is running, `false` otherwise.
     */
    [[nodiscard]] bool isRunning() const noexcept { return running_; }

    // ========================================================================
    // WINDOW MANAGEMENT
    // ========================================================================

    /**
     * @brief Creates a new window with the specified parameters.
     * @param params A `WindowParams` struct describing the window's properties.
     * @return A `std::shared_ptr<Window>` to the newly created window.
     *         Returns `nullptr` if window creation fails.
     */
    [[nodiscard]] std::shared_ptr<Window> createWindow(
        const WindowParams& params = WindowParams{}
    );

    /**
     * @brief Retrieves a window by its unique identifier.
     * @param id The `WindowId` of the window to retrieve.
     * @return A `std::shared_ptr<Window>` to the found window, or `nullptr` if not found.
     */
    [[nodiscard]] std::shared_ptr<Window> getWindow(WindowId id) const;

    /**
     * @brief Closes a specific window.
     * @param id The `WindowId` of the window to close.
     */
    void closeWindow(WindowId id);

    /**
     * @brief Closes all windows managed by the application.
     */
    void closeAllWindows();

    /**
     * @brief Gets the window registry for advanced window management.
     * @return A reference to the `WindowRegistry` singleton.
     */
    [[nodiscard]] WindowRegistry& getRegistry() noexcept {
        return WindowRegistry::instance();
    }

    // ========================================================================
    // WORKER THREAD COMMUNICATION (Thread-Safe)
    // ========================================================================

    /**
     * @brief Posts a task to be executed on the main UI thread.
     *
     * This is the primary mechanism for worker threads to safely interact with
     * UI elements or application state that is not thread-safe.
     *
     * @param task A callable object (e.g., lambda) to be executed.
     */
    void postToUiThread(platform::UiTask task) {
        taskQueue_.push(std::move(task));
    }

    /**
     * @brief Posts a task to be executed on the UI thread after a specified delay.
     * @tparam Rep The representation type of the duration.
     * @tparam Period The period type of the duration.
     * @param task A callable object to be executed.
     * @param delay The `std::chrono::duration` to wait before execution.
     */
    template <typename Rep, typename Period>
    void postDelayed(
        platform::UiTask task,
        const std::chrono::duration<Rep, Period>& delay
    );

    // ========================================================================
    // EVENT LOOP CONTROL
    // ========================================================================

    /**
     * @brief Processes all pending tasks in the UI task queue.
     * @note This is called internally by the event loop.
     */
    void processPendingTasks();

    /**
     * @brief Processes a single iteration of the event loop.
     *
     * Useful for integrating into a custom or external event loop.
     * Processes window messages and pending tasks.
     *
     * @return `true` if the application should continue running, `false` otherwise.
     */
    bool pollEvents();

    // ========================================================================
    // RENDER CONTROL
    // ========================================================================

    /**
     * @brief Requests a redraw for all windows on the next frame.
     */
    void requestRender();

    /**
     * @brief Requests a redraw for a specific window on the next frame.
     * @param id The `WindowId` of the window to redraw.
     */
    void requestRender(WindowId id);

    /**
     * @brief Sets the target frame rate for the application's render loop.
     * @param fps The target frames per second. Set to 0 for unlimited FPS.
     * @note The actual frame rate may be lower depending on system performance.
     */
    void setTargetFps(uint32_t fps) noexcept;

    /**
     * @brief Gets the current target frame rate.
     * @return The target frames per second (0 means unlimited).
     */
    [[nodiscard]] uint32_t getTargetFps() const noexcept;

private:
    /**
     * @brief The core implementation of the main event loop.
     * @internal
     */
    void runMainLoop();

    /**
     * @brief Processes platform-specific window messages.
     * @internal
     */
    void processWindowMessages();

    /**
     * @brief Renders all windows that have been marked as "dirty" (requiring a redraw).
     * @internal
     */
    void renderWindows();
};

} // namespace frqs::core

// ============================================================================
// CONVENIENCE MACROS
// ============================================================================

/**
 * @def FRQS_APP
 * @brief A convenience macro to get the `frqs::core::Application` singleton instance.
 */
#define FRQS_APP frqs::core::Application::instance()

/**
 * @def FRQS_POST_UI(code)
 * @brief A convenience macro to post a block of code to the UI thread.
 * @param code The code to execute on the UI thread.
 *
 * Example:
 * @code
 * FRQS_POST_UI({
 *     my_label->setText("Updated from worker!");
 * });
 * @endcode
 */
#define FRQS_POST_UI(code) \
    FRQS_APP.postToUiThread([=]() { code; })

/**
 * @def FRQS_POST_UI_CAPTURE(captures, code)
 * @brief A convenience macro to post a block of code to the UI thread with an explicit lambda capture list.
 * @param captures The lambda capture list (e.g., `&`, `this`, `&my_var`).
 * @param code The code to execute on the UI thread.
 *
 * Example:
 * @code
 * FRQS_POST_UI_CAPTURE(&, {
 *     my_label->setText(some_local_variable);
 * });
 * @endcode
 */
#define FRQS_POST_UI_CAPTURE(captures, code) \
    FRQS_APP.postToUiThread([captures]() { code; })