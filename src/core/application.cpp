#include "../../include/core/application.hpp"
#include "../../include/platform/win32_safe.hpp"
#include <chrono>

namespace frqs::core {

// ============================================================================
// APPLICATION PIMPL
// ============================================================================

struct Application::Impl {
    platform::NativeInstance hInstance = nullptr;
    uint32_t targetFps = 60;
    std::chrono::steady_clock::time_point lastFrameTime;
    
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
    // Direct2D initialization happens per-window in RendererD2D constructor
    // Window class registration happens in Win32WindowClass singleton
}

void Application::run() {
    running_ = true;
    runMainLoop();
}

void Application::quit() noexcept {
    running_ = false;
}

// ============================================================================
// WINDOW MANAGEMENT
// ============================================================================

std::shared_ptr<Window> Application::createWindow(const WindowParams& params) {
    return Window::create(params);
}

std::shared_ptr<Window> Application::getWindow(WindowId id) const {
    auto window = WindowRegistry::instance().getWindow(id);
    return window.value_or(nullptr);
}

void Application::closeWindow(WindowId id) {
    if (auto window = getWindow(id)) {
        window->close();
    }
}

void Application::closeAllWindows() {
    WindowRegistry::instance().clear();
}

// ============================================================================
// EVENT LOOP CONTROL
// ============================================================================

void Application::processPendingTasks() {
    taskQueue_.processAll([](platform::UiTask&& task) {
        task();
    });
}

bool Application::pollEvents() {
    return true;
}

void Application::processWindowMessages() {
    platform::NativeMessage msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            running_ = false;
            return;
        }
        
        TranslateMessage(&msg);
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
            window->invalidate();
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

void Application::renderWindows() {
    // FIX: Actually render all visible windows
    auto windows = WindowRegistry::instance().getAllWindows();
    for (auto& window : windows) {
        if (window && window->isVisible()) {
            // Force immediate render for smooth animations
            window->forceRedraw();
        }
    }
}

// ============================================================================
// MAIN EVENT LOOP
// ============================================================================

void Application::runMainLoop() {
    using namespace std::chrono;
    
    auto frameDuration = pImpl_->targetFps > 0
        ? duration_cast<milliseconds>(seconds(1)) / pImpl_->targetFps
        : milliseconds(16); // Default 60 FPS

    while (running_) {
        auto frameStart = steady_clock::now();

        // Process Windows messages (MUST be first to handle input)
        processWindowMessages();

        // Process UI tasks from worker threads
        processPendingTasks();

        // FIX: Render all dirty windows (commented out for now - WM_PAINT handles it)
        // renderWindows();

        // Check if any windows remain
        if (WindowRegistry::instance().getWindowCount() == 0) {
            running_ = false;
            break;
        }

        // Frame rate limiting
        if (pImpl_->targetFps > 0) {
            auto frameEnd = steady_clock::now();
            auto frameTime = duration_cast<milliseconds>(frameEnd - frameStart);
            
            if (frameTime < frameDuration) {
                auto sleepTime = frameDuration - frameTime;
                std::this_thread::sleep_for(sleepTime);
            }
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
    // TODO: Implement proper delayed execution with timer
    (void)delay;
    postToUiThread(std::move(task));
}

// Explicit template instantiation
template void Application::postDelayed(
    platform::UiTask,
    const std::chrono::milliseconds&
);

template void Application::postDelayed(
    platform::UiTask,
    const std::chrono::seconds&
);

} // namespace frqs::core