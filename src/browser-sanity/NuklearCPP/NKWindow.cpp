/**
 * @file NKWindow.cpp
 * @brief Implementation of C++ NKWindow base class with shared Nuklear context
 */

#include <NKWindow.h>
#include <NKWindowManager.h>
#include <nk_theme.h>
#include <main.h>
#include <debug_log.h>
#include <windowsx.h>
#include <map>
#include <algorithm>
#include <cmath>

// Static window mapping for HWND -> NKWindow lookup
static std::map<HWND, NKWindow*> g_windowMap;

// NKGdiBackend implementation moved to NKWindowManager.cpp

// NKWindow Implementation
NKWindow::NKWindow(NKWindowManager& windowManager, const std::string& title, int width, int height)
    : m_windowManager(windowManager), m_title(title), m_width(width), m_height(height)
    , m_hwnd(nullptr), m_isDrawing(false), m_autoResize(false), m_nuklearFont(nullptr), m_contextInitialized(false) {
    m_bgColor = NK_THEME_OS_WINDOW_BG; // Use theme background color
    
    // Set default size constraints
    m_minWidth = 200;
    m_minHeight = 150;
    m_maxWidth = 1200;
    m_maxHeight = 800;
    
    // Initialize per-window Nuklear context
    InitializeNuklearContext();
    
    // Register with window manager
    m_windowManager.RegisterWindow(this);
}

bool NKWindow::IsActive() {
    return (m_windowManager.GetActiveWindow() == this);
}

NKWindow::~NKWindow() {
    if (m_isDrawing) {
        DestroyWindow();
    }
    
    // Cleanup per-window Nuklear context
    if (m_contextInitialized) {
        nk_free(&m_nuklearContext);
        m_contextInitialized = false;
    }
    
    // Unregister from window manager
    m_windowManager.UnregisterWindow(this);
}

bool NKWindow::CreateOSWindow(HINSTANCE hInstance, WNDPROC wndProc, const char* className, int x, int y) {
    // Use the window's actual title (ANSI string to match project's MultiByte character set)
    const char* ansiTitle = m_title.c_str();
    
    DebugLog("CreateOSWindow: Creating window with title: %s", ansiTitle);
    
    m_hwnd = CreateWindowExA(
        0,
        className,
        ansiTitle,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        x, y, m_width, m_height,
        NULL, NULL, hInstance, NULL
    );
    
    if (m_hwnd) {
        DebugLog("CreateOSWindow: Window created successfully, HWND: %p", m_hwnd);
        
        // Force set the window title again after creation
        SetWindowTextA(m_hwnd, ansiTitle);
        DebugLog("CreateOSWindow: Window title set again via SetWindowTextA");
        
        RegisterWindowMapping(m_hwnd, this);
        
        // Create and register GDI backend with window manager
        NKGdiBackend* backend = new NKGdiBackend();
        backend->Initialize(m_hwnd, m_width, m_height);
        m_windowManager.RegisterGdiBackend(m_hwnd, backend);
        
        m_isDrawing = true;
        OnCreate();
        return true;
    } else {
        DebugLogError("CreateOSWindow: Failed to create window, error: %lu", GetLastError());
    }
    return false;
}

void NKWindow::ShowWindow(int nCmdShow) {
    if (m_hwnd) {
        ::ShowWindow(m_hwnd, nCmdShow);
        OnShow();
    }
}

void NKWindow::HideWindow() {
    if (m_hwnd) {
        ::ShowWindow(m_hwnd, SW_HIDE);
        OnHide();
    }
}

void NKWindow::DestroyWindow() {
    if (m_hwnd) {
        OnDestroy();
        UnregisterWindowMapping(m_hwnd);
        
        // Unregister GDI backend from window manager
        m_windowManager.UnregisterGdiBackend(m_hwnd);
        
        ::DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
        m_isDrawing = false;
    }
}

void NKWindow::BeginFrame() {
    // No longer needed - window manager handles input cycle
}

void NKWindow::EndFrame() {
    // No longer needed - window manager handles the drawing cycle
    if (m_isDrawing) {
        InvalidateWindow();
    }
}

struct nk_context* NKWindow::GetContext() const {
    return const_cast<struct nk_context*>(&m_nuklearContext);
}

void NKWindow::HandleResize(int width, int height) {
    m_width = width;
    m_height = height;
    if (m_isDrawing) {
        NKGdiBackend* backend = m_windowManager.GetGdiBackend(m_hwnd);
        if (backend) {
            backend->Resize(width, height);
        }
    }
}

int NKWindow::HandleInput(HWND hwndEventSource, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (m_isDrawing && m_contextInitialized) {
        // Process input directly with this window's own context
        ProcessInputEventForWindow(hwndEventSource, msg, wparam, lparam);
        
        NKGdiBackend* backend = m_windowManager.GetGdiBackend(hwndEventSource);
        if (backend) {
            return backend->HandleEvent(hwndEventSource, msg, wparam, lparam);
        }
    }
    return 0;
}

void NKWindow::SetBackgroundColor(unsigned char r, unsigned char g, unsigned char b) {
    m_bgColor = nk_rgb(r, g, b);
}

void NKWindow::InvalidateWindow() {
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

void NKWindow::UpdateWindowSize() {
    if (!m_autoResize || !m_hwnd) return;
    
    // Get the current Nuklear window bounds after rendering
    struct nk_context* ctx = GetContext();
    if (!ctx) return;
    
    // Calculate required size based on content
    // This is a simplified approach - in a full implementation you'd measure actual content
    int contentWidth = m_width;
    int contentHeight = m_height;
    
    // Apply size constraints
    if (contentWidth < m_minWidth) contentWidth = m_minWidth;
    if (contentWidth > m_maxWidth) contentWidth = m_maxWidth;
    if (contentHeight < m_minHeight) contentHeight = m_minHeight;
    if (contentHeight > m_maxHeight) contentHeight = m_maxHeight;
    
    // Only resize if size changed significantly
    if (abs(contentWidth - m_width) > 10 || abs(contentHeight - m_height) > 10) {
        m_width = contentWidth;
        m_height = contentHeight;
        
        // Resize the OS window
        SetWindowPos(m_hwnd, NULL, 0, 0, m_width, m_height,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        
        // Update the GDI backend
        NKGdiBackend* backend = m_windowManager.GetGdiBackend(m_hwnd);
        if (backend) {
            backend->Resize(m_width, m_height);
        }
    }
}

void NKWindow::SetSizeConstraints(int minW, int minH, int maxW, int maxH) {
    m_minWidth = minW;
    m_minHeight = minH;
    m_maxWidth = maxW;
    m_maxHeight = maxH;
}

void NKWindow::InitializeNuklearContext() {
    // Initialize font atlas (copied from NKWindowManager)
    struct nk_font_atlas atlas;
    nk_font_atlas_init_default(&atlas);
    nk_font_atlas_begin(&atlas);
    
    // Add default font with 16pt size for DPI scaling
    m_nuklearFont = nk_font_atlas_add_default(&atlas, 16, 0);
    
    // Bake the font atlas
    const void *image;
    int atlas_w, atlas_h;
    image = nk_font_atlas_bake(&atlas, &atlas_w, &atlas_h, NK_FONT_ATLAS_RGBA32);
    
    // End atlas (no GPU upload needed for GDI)
    nk_font_atlas_end(&atlas, nk_handle_ptr(0), NULL);
    
    // Initialize context with font
    nk_init_default(&m_nuklearContext, &m_nuklearFont->handle);
    
    // Apply theme (copied from NKWindowManager)
    ApplyThemeToContext();
    
    m_contextInitialized = true;
    DebugLog("InitializeNuklearContext: Per-window context initialized for window: %s", m_title.c_str());
}

void NKWindow::ApplyThemeToContext() {
    // Apply light theme colors (copied from NKWindowManager)
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = NK_THEME_TEXT;
    table[NK_COLOR_WINDOW] = NK_THEME_WINDOW;
    table[NK_COLOR_HEADER] = NK_THEME_HEADER;
    table[NK_COLOR_BORDER] = NK_THEME_BORDER;
    table[NK_COLOR_BUTTON] = nk_rgb(220, 220, 220);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgb(200, 200, 200);
    table[NK_COLOR_BUTTON_ACTIVE] = NK_THEME_BUTTON_ACTIVE;
    table[NK_COLOR_TOGGLE] = NK_THEME_TOGGLE;
    table[NK_COLOR_TOGGLE_HOVER] = NK_THEME_TOGGLE_HOVER;
    table[NK_COLOR_TOGGLE_CURSOR] = NK_THEME_TOGGLE_CURSOR;
    table[NK_COLOR_SELECT] = NK_THEME_SELECT;
    table[NK_COLOR_SELECT_ACTIVE] = NK_THEME_SELECT_ACTIVE;
    table[NK_COLOR_SLIDER] = NK_THEME_SLIDER;
    table[NK_COLOR_SLIDER_CURSOR] = NK_THEME_SLIDER_CURSOR;
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = NK_THEME_SLIDER_CURSOR_HOVER;
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = NK_THEME_SLIDER_CURSOR_ACTIVE;
    table[NK_COLOR_PROPERTY] = NK_THEME_PROPERTY;
    table[NK_COLOR_EDIT] = NK_THEME_EDIT;
    table[NK_COLOR_EDIT_CURSOR] = NK_THEME_EDIT_CURSOR;
    table[NK_COLOR_COMBO] = NK_THEME_COMBO;
    table[NK_COLOR_CHART] = NK_THEME_CHART;
    table[NK_COLOR_CHART_COLOR] = NK_THEME_CHART_COLOR;
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = NK_THEME_CHART_COLOR_HIGHLIGHT;
    table[NK_COLOR_SCROLLBAR] = NK_THEME_SCROLLBAR;
    table[NK_COLOR_SCROLLBAR_CURSOR] = NK_THEME_SCROLLBAR_CURSOR;
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = NK_THEME_SCROLLBAR_CURSOR_HOVER;
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = NK_THEME_SCROLLBAR_CURSOR_ACTIVE;
    table[NK_COLOR_TAB_HEADER] = NK_THEME_TAB_HEADER;
    
    // Apply the color table to the context
    nk_style_from_table(&m_nuklearContext, table);
    
    // Additional style tweaks for better appearance
    m_nuklearContext.style.window.border = 2.0f;
    m_nuklearContext.style.window.rounding = 6.0f;
    m_nuklearContext.style.window.border_color = nk_rgb(160, 160, 160);
    m_nuklearContext.style.button.border = 2.0f;
    m_nuklearContext.style.button.rounding = 4.0f;
    m_nuklearContext.style.button.border_color = nk_rgb(160, 160, 160);
    m_nuklearContext.style.edit.border = 1.0f;
    m_nuklearContext.style.edit.rounding = 4.0f;
    m_nuklearContext.style.edit.border_color = NK_THEME_BORDER;
}

void NKWindow::ProcessInputEventForWindow(HWND hwndEventReceiver, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (!m_contextInitialized) return;
    
    // Process input event directly with this window's own context
    // This ensures complete input isolation between windows
    switch (msg) {
        case WM_KEYDOWN:
        case WM_KEYUP: {
            int down = (msg == WM_KEYDOWN);
            int ctrl = GetKeyState(VK_CONTROL) & 0x8000;
            
            switch (wparam) {
                case VK_SHIFT:
                case VK_LSHIFT:
                case VK_RSHIFT:
                    nk_input_key(&m_nuklearContext, NK_KEY_SHIFT, down);
                    break;
                case VK_DELETE:
                    nk_input_key(&m_nuklearContext, NK_KEY_DEL, down);
                    break;
                case VK_RETURN:
                    nk_input_key(&m_nuklearContext, NK_KEY_ENTER, down);
                    break;
                case VK_TAB:
                    nk_input_key(&m_nuklearContext, NK_KEY_TAB, down);
                    break;
                case VK_LEFT:
                    if (ctrl) nk_input_key(&m_nuklearContext, NK_KEY_TEXT_WORD_LEFT, down);
                    else nk_input_key(&m_nuklearContext, NK_KEY_LEFT, down);
                    break;
                case VK_RIGHT:
                    if (ctrl) nk_input_key(&m_nuklearContext, NK_KEY_TEXT_WORD_RIGHT, down);
                    else nk_input_key(&m_nuklearContext, NK_KEY_RIGHT, down);
                    break;
                case VK_BACK:
                    nk_input_key(&m_nuklearContext, NK_KEY_BACKSPACE, down);
                    break;
                case VK_HOME:
                    nk_input_key(&m_nuklearContext, NK_KEY_TEXT_START, down);
                    break;
                case VK_END:
                    nk_input_key(&m_nuklearContext, NK_KEY_TEXT_END, down);
                    break;
                case 'A':
                    if (ctrl && down) {
                        nk_input_key(&m_nuklearContext, NK_KEY_TEXT_SELECT_ALL, 1);
                    }
                    break;
                case 'C':
                    if (ctrl && down) {
                        nk_input_key(&m_nuklearContext, NK_KEY_COPY, 1);
                    }
                    break;
                case 'V':
                    if (ctrl && down) {
                        nk_input_key(&m_nuklearContext, NK_KEY_PASTE, 1);
                    }
                    break;
                case 'X':
                    if (ctrl && down) {
                        nk_input_key(&m_nuklearContext, NK_KEY_CUT, 1);
                    }
                    break;
            }
            break;
        }
        case WM_CHAR:
            if (wparam >= 32) {
                nk_input_unicode(&m_nuklearContext, (nk_rune)wparam);
            }
            break;
        case WM_LBUTTONDOWN: {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            // Left mouse button down - input isolated to this window
            SetCapture(hwndEventReceiver);
            nk_input_button(&m_nuklearContext, NK_BUTTON_LEFT, x, y, 1);
            break;
        }
        case WM_LBUTTONUP: {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            // Left mouse button up - input isolated to this window
            ReleaseCapture();
            nk_input_button(&m_nuklearContext, NK_BUTTON_LEFT, x, y, 0);
            break;
        }
        case WM_RBUTTONDOWN: {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            // Right mouse button down - input isolated to this window
            SetCapture(hwndEventReceiver);
            nk_input_button(&m_nuklearContext, NK_BUTTON_RIGHT, x, y, 1);
            break;
        }
        case WM_RBUTTONUP: {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            // Right mouse button up - input isolated to this window
            ReleaseCapture();
            nk_input_button(&m_nuklearContext, NK_BUTTON_RIGHT, x, y, 0);
            break;
        }
        case WM_MOUSEMOVE:
            nk_input_motion(&m_nuklearContext, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            break;
        case WM_MOUSEWHEEL:
            nk_input_scroll(&m_nuklearContext, nk_vec2(0, (float)(short)HIWORD(wparam) / WHEEL_DELTA));
            break;
    }
}

// Window mapping functions
void RegisterWindowMapping(HWND hwnd, NKWindow* window) {
    g_windowMap[hwnd] = window;
}

void UnregisterWindowMapping(HWND hwnd) {
    g_windowMap.erase(hwnd);
}

NKWindow* GetWindowFromHWND(HWND hwnd) {
    auto it = g_windowMap.find(hwnd);
    return (it != g_windowMap.end()) ? it->second : nullptr;
}

// Generic window procedure
LRESULT CALLBACK NKWindowProc(HWND hwndEventSource, UINT msg, WPARAM wparam, LPARAM lparam) {
    NKWindow* eventSourceWindow = GetWindowFromHWND(hwndEventSource);
    
    if (eventSourceWindow && eventSourceWindow->HandleInput(hwndEventSource, msg, wparam, lparam)) {
        InvalidateRect(hwndEventSource, NULL, FALSE);
        return 0;
    }
    
    switch (msg) {
        case WM_SETFOCUS:
            if (eventSourceWindow) {
                // Update the window manager's focused window
                eventSourceWindow->m_windowManager.SetFocusedWindow(hwndEventSource);
                // Also set this window as the active window for keyboard input
                eventSourceWindow->m_windowManager.SetActiveWindow(eventSourceWindow);
                DebugLog("WM_SETFOCUS: Window %p gained focus and became active", hwndEventSource);
            }
            return 0;
        case WM_KILLFOCUS:
            if (eventSourceWindow) {
                // Clear focus if this window is losing it
                if (eventSourceWindow->m_windowManager.GetFocusedWindow() == hwndEventSource) {
                    eventSourceWindow->m_windowManager.SetFocusedWindow(nullptr);
                    DebugLog("WM_KILLFOCUS: Window %p lost focus", hwndEventSource);
                }
            }
            return 0;
        case WM_SIZE:
            if (eventSourceWindow) {
                eventSourceWindow->HandleResize(LOWORD(lparam), HIWORD(lparam));
                InvalidateRect(hwndEventSource, NULL, FALSE);
            }
            return 0;
        case WM_PAINT: {
            DebugLogDraw("WM_PAINT: Starting paint for HWND %p", hwndEventSource);
            PAINTSTRUCT ps;
            HDC windowDeviceContext = BeginPaint(hwndEventSource, &ps);
            if (eventSourceWindow) {
                // Get GDI backend from window manager and copy from memory DC to window DC
                NKGdiBackend* backend = eventSourceWindow->m_windowManager.GetGdiBackend(hwndEventSource);
                if (backend && backend->memory_dc) {
                    DebugLogDraw("WM_PAINT: BitBlt from memory DC to window DC for HWND %p", hwndEventSource);
                    BitBlt(windowDeviceContext, 0, 0, eventSourceWindow->GetWidth(), eventSourceWindow->GetHeight(),
                           backend->memory_dc, 0, 0, SRCCOPY);
                } else {
                    DebugLogDraw("WM_PAINT: No backend or memory_dc for HWND %p", hwndEventSource);
                }
            } else {
                DebugLogDraw("WM_PAINT: Window not active for HWND %p", hwndEventSource);
            }
            EndPaint(hwndEventSource, &ps);
            DebugLogDraw("WM_PAINT: Paint complete for HWND %p", hwndEventSource);
            return 0;
        }
        case WM_CLOSE:
            if (eventSourceWindow) {
                eventSourceWindow->HideWindow();
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwndEventSource, msg, wparam, lparam);
}