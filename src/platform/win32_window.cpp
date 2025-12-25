/**
 * @file win32_window.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implements the Win32 windowing backend for the application.
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "core/window.hpp"
#include "core/window_impl.hpp"
#include "event/event_types.hpp"
#include <shellapi.h>
#include <mutex>

namespace frqs::platform {

// ============================================================================
// WINDOW CLASS REGISTRATION
// ============================================================================

/**
 * @class Win32WindowClass
 * @brief Manages the registration and lifecycle of the Win32 window class.
 * 
 * This class is a singleton that ensures the WNDCLASSEXW for the application's
 * windows is registered only once. It also handles unregistration on exit.
 */
class Win32WindowClass {
private:
    static constexpr const wchar_t* CLASS_NAME = L"FRQS_Window_Class"; ///< The name of the window class.
    ATOM classAtom_ = 0; ///< The atom representing the registered window class.
    HINSTANCE hInstance_ = nullptr; ///< The instance handle of the application.
    std::mutex mutex_; ///< Mutex for thread-safe initialization.
    
    /**
     * @brief Gets the singleton instance of the Win32WindowClass.
     * @return A reference to the singleton instance.
     */
    static Win32WindowClass& instance() {
        static Win32WindowClass inst;
        return inst;
    }

    /**
     * @brief Private constructor to enforce singleton pattern.
     * 
     * Initializes the instance handle and registers the window class.
     */
    Win32WindowClass() {
        hInstance_ = GetModuleHandleW(nullptr);
        registerClass();
    }

    /**
     * @brief Destructor.
     * 
     * Unregisters the window class if it was successfully registered.
     */
    ~Win32WindowClass() {
        if (classAtom_ && hInstance_) {
            UnregisterClassW(CLASS_NAME, hInstance_);
        }
    }

    /**
     * @brief Registers the WNDCLASSEXW for the application windows.
     * @throws std::runtime_error if class registration fails.
     */
    void registerClass() {
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = windowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(void*);
        wcex.hInstance = hInstance_;
        wcex.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = CLASS_NAME;
        wcex.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

        classAtom_ = RegisterClassExW(&wcex);
        if (!classAtom_) {
            throw std::runtime_error("Failed to register window class");
        }
    }

    /**
     * @brief The main window procedure for all windows created with this class.
     * 
     * This static method dispatches messages to the appropriate `core::Window` instance.
     * @param hwnd Handle to the window.
     * @param msg The message.
     * @param wp Additional message information.
     * @param lp Additional message information.
     * @return The result of the message processing.
     */
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        core::Window* window = nullptr;
        
        if (msg == WM_NCCREATE) {
            auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lp);
            window = static_cast<core::Window*>(createStruct->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        } else {
            window = reinterpret_cast<core::Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (window) {
            return handleMessage(window, hwnd, msg, wp, lp);
        }

        return DefWindowProcW(hwnd, msg, wp, lp);
    }

    /**
     * @brief Handles a specific window message and translates it into an application event.
     * @param window Pointer to the `core::Window` instance.
     * @param hwnd Handle to the window.
     * @param msg The message.
     * @param wp Additional message information.
     * @param lp Additional message information.
     * @return The result of the message processing.
     */
    static LRESULT handleMessage(core::Window* window, HWND hwnd, UINT msg, 
                                WPARAM wp, LPARAM lp) {
        auto* pImpl = window->pImpl_.get();
        
        switch (msg) {
            case WM_GETMINMAXINFO: {
                auto* mmi = reinterpret_cast<MINMAXINFO*>(lp);
                mmi->ptMinTrackSize.x = 1;
                mmi->ptMinTrackSize.y = 1;
                mmi->ptMaxTrackSize.x = 32000;
                mmi->ptMaxTrackSize.y = 32000;
                return 0;
            }
            
            case WM_CLOSE:
                window->close();
                return 0;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            case WM_SIZE: {
                UINT width = LOWORD(lp);
                UINT height = HIWORD(lp);
                pImpl->handleSizeMessage(width, height);
                return 0;
            }

            case WM_MOVE: {
                POINT clientTopLeft = {0, 0};
                ClientToScreen(hwnd, &clientTopLeft);
                pImpl->position = widget::Point<int32_t>(clientTopLeft.x, clientTopLeft.y);
                return 0;
            }

            case WM_ENTERSIZEMOVE:
                pImpl->inSizeMove = true;
                return 0;

            case WM_EXITSIZEMOVE:
                pImpl->inSizeMove = false;
                window->forceRedraw();
                return 0;

            case WM_PAINT: {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);
                
                if (pImpl->renderer && pImpl->rootWidget && pImpl->visible) {
                    try {
                        pImpl->render();
                    } catch (...) {
                        // Ignore render errors
                    }
                }
                
                EndPaint(hwnd, &ps);
                return 0;
            }
            
            case WM_DISPLAYCHANGE:
                window->invalidate();
                return 0;

            case WM_ERASEBKGND:
                return 1;

            case WM_SETFOCUS:
                pImpl->focused = true;
                return 0;

            case WM_KILLFOCUS:
                pImpl->focused = false;
                return 0;

            // ================================================================
            // CRITICAL: WM_CHAR - Text Input
            // ================================================================
            case WM_CHAR: {
                if (!pImpl->rootWidget) return 0;
                
                wchar_t character = static_cast<wchar_t>(wp);
                
                // Ignore control characters except tab, enter, backspace
                if (character < 32 && character != L'\t' && character != L'\r' && character != L'\b') {
                    return 0;
                }
                
                // Get modifiers
                uint32_t mods = 0;
                if (GetKeyState(VK_CONTROL) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Control);
                if (GetKeyState(VK_SHIFT) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Shift);
                if (GetKeyState(VK_MENU) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Alt);
                
                // Set high bit to indicate WM_CHAR
                mods |= 0x80000000;
                
                event::KeyEvent evt{
                    .keyCode = static_cast<uint32_t>(character),
                    .action = event::KeyEvent::Action::Press,
                    .modifiers = mods,
                    .timestamp = static_cast<uint64_t>(GetTickCount64())
                };
                
                window->dispatchEvent(event::Event(evt));
                return 0;
            }

            // ================================================================
            // CRITICAL: WM_KEYDOWN / WM_KEYUP - Navigation Keys
            // ================================================================
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP: {
                if (!pImpl->rootWidget) break;
                
                event::KeyEvent::Action action;
                if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) {
                    bool isRepeat = (lp & (1 << 30)) != 0;
                    action = isRepeat ? event::KeyEvent::Action::Repeat : event::KeyEvent::Action::Press;
                } else {
                    action = event::KeyEvent::Action::Release;
                }
                
                uint32_t keyCode = static_cast<uint32_t>(wp);
                uint32_t mods = 0;
                if (GetKeyState(VK_CONTROL) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Control);
                if (GetKeyState(VK_SHIFT) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Shift);
                if (GetKeyState(VK_MENU) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Alt);
                
                event::KeyEvent evt{
                    .keyCode = keyCode,
                    .action = action,
                    .modifiers = mods,
                    .timestamp = static_cast<uint64_t>(GetTickCount64())
                };
                
                window->dispatchEvent(event::Event(evt));
                
                // Always break to allow DefWindowProc to generate WM_CHAR
                break;
            }

            // ================================================================
            // CRITICAL: WM_LBUTTONDOWN / WM_RBUTTONDOWN / WM_MBUTTONDOWN
            // ================================================================
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN: {
                if (!pImpl->rootWidget) return 0;
                
                event::MouseButtonEvent::Button btn;
                if (msg == WM_LBUTTONDOWN) btn = event::MouseButtonEvent::Button::Left;
                else if (msg == WM_RBUTTONDOWN) btn = event::MouseButtonEvent::Button::Right;
                else btn = event::MouseButtonEvent::Button::Middle;
                
                int32_t x = GET_X_LPARAM(lp);
                int32_t y = GET_Y_LPARAM(lp);
                uint32_t mods = 0;
                if (GetKeyState(VK_CONTROL) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Control);
                if (GetKeyState(VK_SHIFT) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Shift);
                if (GetKeyState(VK_MENU) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Alt);
                
                event::MouseButtonEvent evt{
                    .button = btn,
                    .action = event::MouseButtonEvent::Action::Press,
                    .position = widget::Point<int32_t>(x, y),
                    .modifiers = mods,
                    .timestamp = static_cast<uint64_t>(GetTickCount64())
                };
                
                window->dispatchEvent(event::Event(evt));
                return 0;
            }

            // ================================================================
            // CRITICAL: WM_LBUTTONUP / WM_RBUTTONUP / WM_MBUTTONUP
            // ================================================================
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP: {
                if (!pImpl->rootWidget) return 0;
                
                event::MouseButtonEvent::Button btn;
                if (msg == WM_LBUTTONUP) btn = event::MouseButtonEvent::Button::Left;
                else if (msg == WM_RBUTTONUP) btn = event::MouseButtonEvent::Button::Right;
                else btn = event::MouseButtonEvent::Button::Middle;
                
                int32_t x = GET_X_LPARAM(lp);
                int32_t y = GET_Y_LPARAM(lp);
                uint32_t mods = 0;
                if (GetKeyState(VK_CONTROL) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Control);
                if (GetKeyState(VK_SHIFT) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Shift);
                if (GetKeyState(VK_MENU) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Alt);
                
                event::MouseButtonEvent evt{
                    .button = btn,
                    .action = event::MouseButtonEvent::Action::Release,
                    .position = widget::Point<int32_t>(x, y),
                    .modifiers = mods,
                    .timestamp = static_cast<uint64_t>(GetTickCount64())
                };
                
                window->dispatchEvent(event::Event(evt));
                return 0;
            }

            // ================================================================
            // CRITICAL: WM_MOUSEMOVE
            // ================================================================
            case WM_MOUSEMOVE: {
                if (!pImpl->rootWidget) return 0;
                
                static widget::Point<int32_t> lastPos(0, 0);
                
                int32_t x = GET_X_LPARAM(lp);
                int32_t y = GET_Y_LPARAM(lp);
                uint32_t mods = 0;
                if (GetKeyState(VK_CONTROL) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Control);
                if (GetKeyState(VK_SHIFT) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Shift);
                if (GetKeyState(VK_MENU) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Alt);
                
                event::MouseMoveEvent evt{
                    .position = widget::Point<int32_t>(x, y),
                    .delta = widget::Point<int32_t>(x - lastPos.x, y - lastPos.y),
                    .modifiers = mods,
                    .timestamp = static_cast<uint64_t>(GetTickCount64())
                };
                
                lastPos = evt.position;
                
                window->dispatchEvent(event::Event(evt));
                return 0;
            }

            // ================================================================
            // CRITICAL: WM_MOUSEWHEEL - Scroll Event
            // ================================================================
            case WM_MOUSEWHEEL: {
                if (!pImpl->rootWidget) return 0;
                
                // Extract wheel delta (in units of WHEEL_DELTA = 120)
                int32_t delta = GET_WHEEL_DELTA_WPARAM(wp);
                
                // Get mouse position (in screen coordinates)
                POINT pt;
                pt.x = GET_X_LPARAM(lp);
                pt.y = GET_Y_LPARAM(lp);
                ScreenToClient(hwnd, &pt);
                
                // Get modifiers
                uint32_t mods = 0;
                if (GetKeyState(VK_CONTROL) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Control);
                if (GetKeyState(VK_SHIFT) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Shift);
                if (GetKeyState(VK_MENU) & 0x8000) mods |= static_cast<uint32_t>(event::ModifierKey::Alt);
                
                event::MouseWheelEvent evt{
                    .delta = delta,
                    .position = widget::Point<int32_t>(pt.x, pt.y),
                    .modifiers = mods,
                    .timestamp = static_cast<uint64_t>(GetTickCount64())
                };
                
                window->dispatchEvent(event::Event(evt));
                return 0;
            }

            // ================================================================
            // CRITICAL: WM_DROPFILES - File Drop Event
            // ================================================================
            case WM_DROPFILES: {
                if (!pImpl->rootWidget) return 0;
                
                HDROP hDrop = reinterpret_cast<HDROP>(wp);
                
                // Get drop position
                POINT pt;
                DragQueryPoint(hDrop, &pt);
                
                // Get number of files
                UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
                
                if (fileCount > 0) {
                    std::vector<std::wstring> files;
                    files.reserve(fileCount);
                    
                    // Get each file path
                    for (UINT i = 0; i < fileCount; ++i) {
                        UINT pathLen = DragQueryFileW(hDrop, i, nullptr, 0);
                        if (pathLen > 0) {
                            std::wstring path(pathLen, L'\0');
                            DragQueryFileW(hDrop, i, path.data(), pathLen + 1);
                            files.push_back(std::move(path));
                        }
                    }
                    
                    // Create FileDropEvent
                    event::FileDropEvent evt(
                        widget::Point<int32_t>(pt.x, pt.y),
                        files
                    );
                    
                    // Dispatch to widget tree
                    window->dispatchEvent(event::Event(std::move(evt)));
                }
                
                DragFinish(hDrop);
                return 0;
            }

            default:
                return DefWindowProcW(hwnd, msg, wp, lp);
        }
        
        // Default handling for messages that use 'break'
        return DefWindowProcW(hwnd, msg, wp, lp);
    }

public:
    /**
     * @brief Creates a new Win32 window.
     * 
     * @param title The window title.
     * @param x The x-coordinate of the window's top-left corner.
     * @param y The y-coordinate of the window's top-left corner.
     * @param width The width of the window.
     * @param height The height of the window.
     * @param style The window style flags.
     * @param exStyle The extended window style flags.
     * @param userData A pointer to user data to be associated with the window.
     * @return The handle to the created window, or nullptr on failure.
     */
    static HWND createWindow(
        const wchar_t* title,
        int x, int y,
        int width, int height,
        DWORD style,
        DWORD exStyle,
        void* userData
    ) {
        auto& inst = instance();
        if (!inst.classAtom_) {
            return nullptr;
        }

        return CreateWindowExW(
            exStyle,
            CLASS_NAME,
            title,
            style,
            x, y,
            width, height,
            nullptr,
            nullptr,
            inst.hInstance_,
            userData
        );
    }

    /**
     * @brief Ensures that the window class is registered.
     * 
     * This function can be called to explicitly initialize the singleton
     * and register the window class before creating any windows.
     */
    static void ensureRegistered() {
        instance();
    }
};

// ============================================================================
// WIN32 WINDOW CREATION HELPER
// ============================================================================

/**
 * @brief Creates a native Win32 window based on the provided parameters.
 * 
 * @param params The parameters for creating the window.
 * @param windowPtr A pointer to the `core::Window` that this native window represents.
 * @return The handle to the created native window.
 * @throws std::runtime_error if window creation fails.
 */
HWND createNativeWindow(
    const core::WindowParams& params,
    void* windowPtr
) {
    Win32WindowClass::ensureRegistered();

    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exStyle = WS_EX_APPWINDOW | WS_EX_ACCEPTFILES;  // ✅ CRITICAL: Enable file drop

    if (!params.decorated) {
        style = WS_POPUP | WS_THICKFRAME;
    }

    if (!params.resizable) {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    }

    RECT rect = {0, 0, static_cast<LONG>(params.size.w), static_cast<LONG>(params.size.h)};
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    HWND hwnd = Win32WindowClass::createWindow(
        params.title.c_str(),
        params.position.x,
        params.position.y,
        width,
        height,
        style,
        exStyle,
        windowPtr
    );

    if (!hwnd) {
        throw std::runtime_error("Failed to create native window");
    }

    DragAcceptFiles(hwnd, TRUE);

    if (params.visible) {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }

    return hwnd;
}

} // namespace frqs::platform