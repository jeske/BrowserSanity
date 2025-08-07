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
    , m_hwnd(nullptr), m_active(false), m_autoResize(false) {
    m_bgColor = NK_THEME_OS_WINDOW_BG; // Use theme background color
    
    // Set default size constraints
    m_minWidth = 200;
    m_minHeight = 150;
    m_maxWidth = 1200;
    m_maxHeight = 800;
    
    // Register with window manager
    m_windowManager.RegisterWindow(this);
}

NKWindow::~NKWindow() {
    if (m_active) {
        DestroyWindow();
    }
    // Unregister from window manager
    m_windowManager.UnregisterWindow(this);
}

bool NKWindow::CreateOSWindow(HINSTANCE hInstance, WNDPROC wndProc, const wchar_t* className, int x, int y) {
    // Convert title from std::string to wide string
    std::wstring wideTitle;
    wideTitle.assign(m_title.begin(), m_title.end());
    
    m_hwnd = CreateWindowExW(
        0,
        className,
        wideTitle.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        x, y, m_width, m_height,
        NULL, NULL, hInstance, NULL
    );
    
    if (m_hwnd) {
        RegisterWindowMapping(m_hwnd, this);
        
        // Create and register GDI backend with window manager
        NKGdiBackend* backend = new NKGdiBackend();
        backend->Initialize(m_hwnd, m_width, m_height);
        m_windowManager.RegisterGdiBackend(m_hwnd, backend);
        
        m_active = true;
        OnCreate();
        return true;
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
        m_active = false;
    }
}

void NKWindow::BeginFrame() {
    // No longer needed - window manager handles input cycle
}

void NKWindow::EndFrame() {
    // No longer needed - window manager handles the drawing cycle
    if (m_active) {
        InvalidateWindow();
    }
}

struct nk_context* NKWindow::GetContext() const {
    return m_windowManager.GetContext();
}

void NKWindow::HandleResize(int width, int height) {
    m_width = width;
    m_height = height;
    if (m_active) {
        NKGdiBackend* backend = m_windowManager.GetGdiBackend(m_hwnd);
        if (backend) {
            backend->Resize(width, height);
        }
    }
}

int NKWindow::HandleInput(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (m_active) {
        // Forward input to window manager for processing
        m_windowManager.ProcessInput(hwnd, msg, wparam, lparam);
        
        NKGdiBackend* backend = m_windowManager.GetGdiBackend(hwnd);
        if (backend) {
            return backend->HandleEvent(hwnd, msg, wparam, lparam);
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
LRESULT CALLBACK NKWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    NKWindow* window = GetWindowFromHWND(hwnd);
    
    if (window && window->HandleInput(hwnd, msg, wparam, lparam)) {
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    
    switch (msg) {
        case WM_SIZE:
            if (window) {
                window->HandleResize(LOWORD(lparam), HIWORD(lparam));
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        case WM_PAINT: {
            DebugLog("WM_PAINT: Starting paint for HWND %p", hwnd);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (window && window->IsActive()) {
                // Get GDI backend from window manager and copy from memory DC to window DC
                NKGdiBackend* backend = window->m_windowManager.GetGdiBackend(hwnd);
                if (backend && backend->memory_dc) {
                    DebugLog("WM_PAINT: BitBlt from memory DC to window DC for HWND %p", hwnd);
                    BitBlt(hdc, 0, 0, window->GetWidth(), window->GetHeight(),
                           backend->memory_dc, 0, 0, SRCCOPY);
                } else {
                    DebugLog("WM_PAINT: No backend or memory_dc for HWND %p", hwnd);
                }
            } else {
                DebugLog("WM_PAINT: Window not active for HWND %p", hwnd);
            }
            EndPaint(hwnd, &ps);
            DebugLog("WM_PAINT: Paint complete for HWND %p", hwnd);
            return 0;
        }
        case WM_CLOSE:
            if (window) {
                window->HideWindow();
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}