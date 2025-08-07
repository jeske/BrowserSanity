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
 * @brief Base class for all Nuklear windows
 * Provides common window management, Nuklear context, and virtual interface
 */
class NKWindow {
    friend LRESULT CALLBACK NKWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    
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
    int HandleInput(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    
    // Virtual interface for derived classes
    virtual void Render() = 0;  // Pure virtual - each dialog implements its own UI
    virtual void OnCreate() {}  // Called when window is created
    virtual void OnDestroy() {} // Called when window is destroyed
    virtual void OnShow() {}    // Called when window is shown
    virtual void OnHide() {}    // Called when window is hidden
    
    // Accessors
    HWND GetHWND() const { return m_hwnd; }
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
    
    // Dynamic sizing members
    int m_minWidth, m_minHeight;
    int m_maxWidth, m_maxHeight;
    
    // Helper methods for derived classes
    void SetBackgroundColor(unsigned char r, unsigned char g, unsigned char b);
    void InvalidateWindow();
    void UpdateWindowSize();  // New method for dynamic sizing
    void SetSizeConstraints(int minW, int minH, int maxW = 1200, int maxH = 800);
};


// C-style window procedure that delegates to C++ class
LRESULT CALLBACK NKWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

// Window procedure lookup - maps HWND to NKWindow instance
void RegisterWindowMapping(HWND hwnd, NKWindow* window);
void UnregisterWindowMapping(HWND hwnd);
NKWindow* GetWindowFromHWND(HWND hwnd);
