#include "platform/win32_safe.hpp"
#include "core/window.hpp"
#include "core/application.hpp"
#include "core/window_impl.hpp"
#include "event/event_types.hpp"
#include <unordered_map>
#include <mutex>

namespace frqs::platform {

// ============================================================================
// WINDOW CLASS REGISTRATION
// ============================================================================

class Win32WindowClass {
private:
    static constexpr const wchar_t* CLASS_NAME = L"FRQS_Window_Class";
    ATOM classAtom_ = 0;
    HINSTANCE hInstance_ = nullptr;
    std::mutex mutex_;
    
    static Win32WindowClass& instance() {
        static Win32WindowClass inst;
        return inst;
    }

    Win32WindowClass() {
        hInstance_ = GetModuleHandleW(nullptr);
        registerClass();
    }

    ~Win32WindowClass() {
        if (classAtom_ && hInstance_) {
            UnregisterClassW(CLASS_NAME, hInstance_);
        }
    }

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

    static LRESULT handleMessage(core::Window* window, HWND hwnd, UINT msg, 
                                WPARAM wp, LPARAM lp) {
        auto* pImpl = window->pImpl_.get();
        
        switch (msg) {
            case WM_GETMINMAXINFO: {
                // ✅ CRITICAL FIX: Remove size constraints
                auto* mmi = reinterpret_cast<MINMAXINFO*>(lp);
                
                // Allow any size from very small to very large
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
                // Get client area position (not window position)
                // This accounts for invisible DWM borders
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

            case WM_KEYDOWN :
            case WM_KEYUP :
            case WM_SYSKEYDOWN :
            case WM_SYSKEYUP :{
                if (!pImpl->rootWidget) return 0;
                
                event::KeyEvent::Action action;
                if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) {
                    // Check if this is a repeat
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
                
                event::Event e = evt;
                bool handled = pImpl->rootWidget->onEvent(e);
                
                // If handled by widget, don't pass to DefWindowProc
                if (handled) return 0;
                break;
            }

            case WM_LBUTTONDOWN : 
            case WM_RBUTTONDOWN : 
            case WM_MBUTTONDOWN : {
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
                
                event::Event e = evt;
                pImpl->rootWidget->onEvent(e);
                return 0;
            }

			case WM_LBUTTONUP : 
			case WM_RBUTTONUP : 
            case WM_MBUTTONUP : {
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
                
                event::Event e = evt;
                pImpl->rootWidget->onEvent(e);
                return 0;
            }

            case WM_MOUSEMOVE : {
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
                
                event::Event e = evt;
                pImpl->rootWidget->onEvent(e);
                return 0;
            }

            case WM_MOUSEWHEEL : 
                // TODO: Dispatch mouse events
                return 0;

            default:
                return DefWindowProcW(hwnd, msg, wp, lp);
        }
    }

public:
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

    static void ensureRegistered() {
        instance();
    }
};

// ============================================================================
// WIN32 WINDOW CREATION HELPER
// ============================================================================

HWND createNativeWindow(
    const core::WindowParams& params,
    void* windowPtr
) {
    Win32WindowClass::ensureRegistered();

    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exStyle = WS_EX_APPWINDOW;

    if (!params.decorated) {
        style = WS_POPUP | WS_THICKFRAME;
    }

    if (!params.resizable) {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    }

    // Calculate window size including borders
    RECT rect = {0, 0, static_cast<LONG>(params.size.w), static_cast<LONG>(params.size.h)};
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create window
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

    // Show window if requested
    if (params.visible) {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }

    return hwnd;
}

} // namespace frqs::platform