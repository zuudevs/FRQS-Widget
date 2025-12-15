#include "../../include/platform/win32_safe.hpp"
#include "../../include/core/window.hpp"
#include "../../include/core/application.hpp"
#include "../core/window_impl.hpp"
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
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Add CS_OWNDC for better rendering
        wcex.lpfnWndProc = windowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(void*);
        wcex.hInstance = hInstance_;
        wcex.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = nullptr; // We paint everything ourselves
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
        // Get Impl for direct access to renderer
        auto* pImpl = window->pImpl_.get();
        
        switch (msg) {
            case WM_CLOSE:
                window->close();
                return 0;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            case WM_SIZE: {
                UINT width = LOWORD(lp);
                UINT height = HIWORD(lp);
                
                // Update window size (this also resizes renderer buffer)
                window->setSize(widget::Size<uint32_t>(width, height));
                
                // FIX: Immediate redraw after resize
                if (!pImpl->inSizeMove) {
                    window->forceRedraw();
                }
                return 0;
            }

            case WM_MOVE: {
                int x = static_cast<int16_t>(LOWORD(lp));
                int y = static_cast<int16_t>(HIWORD(lp));
                window->setPosition(widget::Point<int32_t>(x, y));
                return 0;
            }

            case WM_ENTERSIZEMOVE:
                // FIX: Mark that we're in resize/move mode
                pImpl->inSizeMove = true;
                return 0;

            case WM_EXITSIZEMOVE:
                // FIX: Exit resize/move mode and force redraw
                pImpl->inSizeMove = false;
                window->forceRedraw();
                return 0;

            case WM_PAINT: {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);
                
                // FIX: Render during WM_PAINT
                if (pImpl->renderer && pImpl->rootWidget && pImpl->visible) {
                    pImpl->render();
                }
                
                EndPaint(hwnd, &ps);
                return 0;
            }
            
            case WM_DISPLAYCHANGE:
                // Handle display settings change (e.g., resolution change)
                window->invalidate();
                return 0;

            case WM_ERASEBKGND:
                // We handle all painting ourselves
                return 1;

            case WM_SETFOCUS:
                pImpl->focused = true;
                return 0;

            case WM_KILLFOCUS:
                pImpl->focused = false;
                return 0;

            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
                // TODO: Dispatch keyboard events
                return 0;

            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MOUSEMOVE:
            case WM_MOUSEWHEEL:
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