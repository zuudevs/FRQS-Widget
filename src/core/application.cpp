/**
 * @file application.cpp
 * @brief Implements the Application class for lifecycle and window management.
 * @author zuudevs (zuudevs@gmail.com)
 * @version 0.1
 * @date 2025-12-24
 *
 * @copyright Copyright (c) 2025
 */

#include "core/application.hpp"
#include "platform/win32_safe.hpp"
#include <thread> // For std::this_thread::sleep_for

namespace frqs::core {

// ============================================================================
// APPLICATION PIMPL
// ============================================================================

/**
 * @struct Application::Impl
 * @brief Private implementation details for the Application class.
 *
 * This struct holds platform-specific handles and internal state, hiding them
 * from the public header and reducing compilation dependencies.
 */
struct Application::Impl {
    /** @brief The native instance handle for the application (e.g., HINSTANCE on Windows). */
    platform::NativeInstance hInstance = nullptr;
    /** @brief The target frames per second for the render loop. 0 means unlimited. */
    uint32_t targetFps = 60;
    /** @brief The time point of the last rendered frame, used for FPS limiting. */
    std::chrono::steady_clock::time_point lastFrameTime;

    /**
     * @brief Construct a new Impl object and get the module handle.
     */
    Impl() {
        hInstance = GetModuleHandleW(nullptr);
        lastFrameTime = std::chrono::steady_clock::now();
    }
};

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

Application::Application() : pImpl_(std::make_unique<Impl>()) {}

Application::~Application() noexcept = default;

// ============================================================================
// LIFECYCLE MANAGEMENT
// ============================================================================

void Application::initialize() {
    // Platform-specific initializations are handled within the respective
    // platform modules (e.g., Win32WindowClass, RendererD2D) to ensure
    // they are constructed on first use.
}

void Application::run() {
    if (running_) {
        return; // Already running
    }
    running_ = true;
    runMainLoop();
}

void Application::quit() noexcept {
    // This simply sets the flag. The runMainLoop will detect this and exit.
    running_ = false;
}

// ============================================================================
// WINDOW MANAGEMENT
// ============================================================================

std::shared_ptr<Window> Application::createWindow(const WindowParams& params) {
    // The Window::create factory handles registration with the WindowRegistry.
    return Window::create(params);
}

std::shared_ptr<Window> Application::getWindow(WindowId id) const {
    // value_or provides a safe way to handle the optional return from the registry.
    auto window = WindowRegistry::instance().getWindow(id);
    return window.value_or(nullptr);
}

void Application::closeWindow(WindowId id) {
    if (auto window = getWindow(id)) {
        window->close(); // This will trigger the window's destruction process.
    }
}

void Application::closeAllWindows() {
    // Clearing the registry will close and release all managed windows.
    WindowRegistry::instance().clear();
}

// ============================================================================
// EVENT LOOP CONTROL
// ============================================================================

void Application::processPendingTasks() {
    // Execute all tasks currently in the queue from worker threads.
    taskQueue_.processAll([](platform::UiTask&& task) {
        if (task) {
            task();
        }
    });
}

bool Application::pollEvents() {
    processWindowMessages();
    processPendingTasks();
    return isRunning();
}

/**
 * @brief Processes the Windows message queue for the current thread.
 * @internal
 *
 * This function retrieves and dispatches messages (e.g., mouse, keyboard, paint)
 * to the appropriate window procedures. It also checks for the WM_QUIT message
 * to terminate the application loop.
 */
void Application::processWindowMessages() {
    platform::NativeMessage msg;
    // PM_REMOVE pulls messages from the queue.
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            // A WM_QUIT message signals that the application should terminate.
            // This is typically posted when the last window is closed.
            running_ = false;
            return;
        }
        
        // Translates virtual-key messages into character messages.
        TranslateMessage(&msg);
        // Dispatches the message to a window's procedure.
        DispatchMessageW(&msg);
    }
}

// ============================================================================
// RENDER CONTROL
// ============================================================================

void Application::requestRender() {
    auto windows = WindowRegistry::instance().getAllWindows();
    for (auto& window : windows) {
        if (window && window->isVisible()) {
            window->invalidate(); // Mark the window as needing a redraw.
        }
    }
}

void Application::requestRender(WindowId id) {
    if (auto window = getWindow(id)) {
        window->invalidate();
    }
}

void Application::setTargetFps(uint32_t fps) noexcept {
    pImpl_->targetFps = fps;
}

uint32_t Application::getTargetFps() const noexcept {
    return pImpl_->targetFps;
}

/**
 * @brief Renders all visible windows that require redrawing.
 * @internal
 *
 * This function iterates through all registered windows and calls their
 * rendering logic if they are visible and have been marked as dirty.
 * NOTE: In the current implementation, rendering is primarily driven by
 * the OS WM_PAINT message, but this function allows for forced, immediate
*  updates needed for smooth animations.
 */
void Application::renderWindows() {
    auto windows = WindowRegistry::instance().getAllWindows();
    for (auto& window : windows) {
        if (window && window->isVisible()) {
            // Calling forceRedraw bypasses the usual invalidation and WM_PAINT
            // cycle, rendering the window's content immediately.
            window->forceRedraw();
        }
    }
}

// ============================================================================
// MAIN EVENT LOOP
// ============================================================================

/**
 * @brief The core event loop of the application.
 * @internal
 *
 * This loop continues as long as `running_` is true. In each iteration, it:
 * 1. Processes system messages (input, paint, etc.).
 * 2. Executes tasks posted from other threads.
 * 3. Checks if it should terminate (e.g., if all windows are closed).
 * 4. Enforces a frame rate limit to control CPU usage.
 */
void Application::runMainLoop() {
    using namespace std::chrono;
    
    while (running_) {
        auto frameStart = steady_clock::now();
        
        auto frameDuration = pImpl_->targetFps > 0
            ? duration_cast<milliseconds>(seconds(1)) / pImpl_->targetFps
            : milliseconds(0);

        // Process Windows messages (MUST be first to handle input and OS events).
        processWindowMessages();

        // If processWindowMessages received WM_QUIT, exit immediately.
        if (!running_) break;

        // Process UI tasks posted from worker threads.
        processPendingTasks();

        // In a non-WM_PAINT driven model, you would render here.
        // For now, renderWindows() is called explicitly where needed.
        // renderWindows();

        // The application automatically quits when the last window is closed.
        if (WindowRegistry::instance().getWindowCount() == 0) {
            quit();
            break;
        }

        // Frame rate limiting to avoid burning CPU cycles.
        if (pImpl_->targetFps > 0) {
            auto frameEnd = steady_clock::now();
            auto frameTime = duration_cast<milliseconds>(frameEnd - frameStart);
            
            if (frameTime < frameDuration) {
                auto sleepTime = frameDuration - frameTime;
                std::this_thread::sleep_for(sleepTime);
            }
        } else {
             // Yield the CPU for a moment if FPS is unlimited to be a good citizen.
             std::this_thread::sleep_for(milliseconds(1));
        }

        pImpl_->lastFrameTime = frameStart;
    }
}

// ============================================================================
// DELAYED TASK (Template Implementation)
// ============================================================================

template <typename Rep, typename Period>
void Application::postDelayed(
    platform::UiTask task,
    const std::chrono::duration<Rep, Period>& delay
) {
    // TODO: Implement a proper delayed execution mechanism, possibly with
    // a priority queue of timed events processed in the main loop.
    // For now, we just post it immediately.
    (void)delay; // Mark delay as unused for now.
    postToUiThread(std::move(task));
}

// Explicit template instantiations are required for template definitions in a .cpp file.
template void Application::postDelayed(
    platform::UiTask,
    const std::chrono::milliseconds&
);

template void Application::postDelayed(
    platform::UiTask,
    const std::chrono::seconds&
);

} // namespace frqs::core