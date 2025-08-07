/**
 * @file NKWindow.h
 * @brief C++ Base class for Nuklear-based windows with inheritance support
 */

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>

// Include Nuklear for complete type definitions
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "nk_theme.h"
#include "browser_sanity.h"

// Forward declarations
struct nk_context;
struct nk_font;
class NKWindowManager;

// NKGdiBackend moved to NKWindowManager.h
struct NKGdiBackend;  // Forward declaration

/**
 * 🎯 BREAKTHROUGH SOLUTION: Context-Per-Window Architecture
 *
 * This class implements the ONLY working solution for multi-window Nuklear applications.
 * Each window owns its complete Nuklear context to prevent assertion failures.
 *
 * 🚨 CRITICAL: Shared contexts cause assertion failures due to input window/context mismatches.
 * The solution is complete input isolation - each window processes ONLY its own input events.
 *
 * Key Architecture:
 * - Each window has its own nk_context (m_nuklearContext)
 * - Each window has its own font atlas (m_nuklearFont)
 * - Input events are routed to target window ONLY via ProcessInputEventForWindow()
 * - No shared state prevents Nuklear's internal consistency check failures
 *
 * This prevents assertion failures like:
 * - "GetCapture() == current_hwnd" (input capture window mismatch)
 * - "ctx->input.mouse.grab_window == current_window" (mouse grab state mismatch)
 */
class NKWindow {
    friend LRESULT CALLBACK NKWindowProc(HWND hwndEventSource, UINT msg, WPARAM wparam, LPARAM lparam);
    
public:
    NKWindow(NKWindowManager& windowManager, const std::string& title, int width, int height);
    virtual ~NKWindow();
    
    // Window management
    bool CreateOSWindow(HINSTANCE hInstance, WNDPROC wndProc, const char* className, int x, int y);
    void ShowWindow(int nCmdShow = SW_SHOW);
    void HideWindow();
    void DestroyWindow();
    
    // Nuklear integration
    void BeginFrame();
    void EndFrame();
    void HandleResize(int width, int height);
    int HandleInput(HWND hwndEventSource, UINT msg, WPARAM wparam, LPARAM lparam);
    
    // Virtual interface for derived classes
    virtual void Render() = 0;  // Pure virtual - each dialog implements its own UI
    virtual void OnCreate() {}  // Called when window is created
    virtual void OnDestroy() {} // Called when window is destroyed
    virtual void OnShow() {}    // Called when window is shown
    virtual void OnHide() {}    // Called when window is hidden
    
    // Accessors
    HWND GetHWND() const { return m_hwnd; }
    /**
     * Get THIS window's Nuklear context
     * ✅ Each window has its own context - complete isolation
     */
    struct nk_context* GetContext() const;
    bool IsActive();
    const std::string& GetTitle() const { return m_title; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    
    // Dynamic sizing
    void EnableAutoResize(bool enable = true) { m_autoResize = enable; }
    bool IsAutoResizeEnabled() const { return m_autoResize; }
    
protected:
    // Protected members accessible to derived classes
    NKWindowManager& m_windowManager;
    std::string m_title;
    int m_width, m_height;
    HWND m_hwnd;
    bool m_isDrawing;
    bool m_autoResize;
    struct nk_color m_bgColor;
    
    // ✅ Per-window Nuklear state - complete isolation prevents assertion failures
    struct nk_context m_nuklearContext;    // THIS window's context only
    struct nk_font* m_nuklearFont;         // THIS window's font atlas
    bool m_contextInitialized;
    
    // Dynamic sizing members
    int m_minWidth, m_minHeight;
    int m_maxWidth, m_maxHeight;
    
    // Helper methods for derived classes
    void SetBackgroundColor(unsigned char r, unsigned char g, unsigned char b);
    void InvalidateWindow();
    void UpdateWindowSize();  // New method for dynamic sizing
    void SetSizeConstraints(int minW, int minH, int maxW = 1200, int maxH = 800);
    
    /**
     * Initialize THIS window's Nuklear context with its own font atlas and theme
     * ✅ Complete isolation - no shared state with other windows
     */
    void InitializeNuklearContext();
    
    /**
     * Apply theme to THIS window's context only
     * ✅ Each window can have independent styling
     */
    void ApplyThemeToContext();

public:
    /**
     * 🎯 CRITICAL: Process input events for THIS window ONLY
     * This method ensures input events are isolated to the correct window's context,
     * preventing assertion failures from input window/context mismatches.
     * Called by NKWindowManager with proper input routing.
     */
    void ProcessInputEventForWindow(HWND hwndEventReceiver, UINT msg, WPARAM wparam, LPARAM lparam);
};


// C-style window procedure that delegates to C++ class
LRESULT CALLBACK NKWindowProc(HWND hwndEventSource, UINT msg, WPARAM wparam, LPARAM lparam);

// Window procedure lookup - maps HWND to NKWindow instance
void RegisterWindowMapping(HWND hwnd, NKWindow* window);
void UnregisterWindowMapping(HWND hwnd);
NKWindow* GetWindowFromHWND(HWND hwnd);
