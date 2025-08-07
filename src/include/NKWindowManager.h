/**
 * @file NKWindowManager.h
 * @brief 🎯 BREAKTHROUGH: Context-Per-Window Architecture
 *
 * This file implements the ONLY working solution for multi-window Nuklear applications.
 * Each window owns its complete Nuklear context to prevent assertion failures.
 *
 * 🚨 CRITICAL: This architecture prevents assertion failures that occur when input
 * events are processed in the wrong window's context. The key insight is that
 * Nuklear's internal consistency checks will detect and assert on mismatches between:
 * - Input event window (HWND that received the Windows message)
 * - Nuklear context window (window currently being processed)
 *
 * WORKING APPROACH: Context-Per-Window with Input Isolation
 * - Each NKWindow owns its own nk_context (complete isolation)
 * - Input events routed to target window ONLY (no broadcasting)
 * - Each window processes: Input → Render → Draw → Clear (isolated cycle)
 * - Centralized GDI backend management (one backend per HWND)
 * - Independent font and theme management per window
 *
 * KEY SUCCESS PRINCIPLES:
 * 1. Context-Per-Window: Each window owns its complete Nuklear state
 * 2. Input Routing: Events go ONLY to target window, never broadcast
 * 3. Complete Isolation: No shared state prevents assertion failures
 * 4. Independent Management: Each window has its own font, theme, and rendering
 * 5. Proper Message Routing: Windows messages target specific windows only
 * 6. Assertion Prevention: Input window always matches context window
 *
 * This architecture eliminates:
 * - Assertion failures from input window/context mismatches
 * - Input event leaking between windows
 * - Global state conflicts in Nuklear's ctx->last_widget_state
 * - Button click detection failures
 */

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <map>
#include <set>
#include <queue>

// Include Nuklear for complete type definitions
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "nk_theme.h"

// Forward declarations
class NKWindow;

/**
 * @brief Input event with target window information for smart routing
 */
struct InputEvent {
    HWND target_hwnd;
    UINT msg;
    WPARAM wparam;
    LPARAM lparam;
    
    InputEvent(HWND hwnd, UINT m, WPARAM wp, LPARAM lp)
        : target_hwnd(hwnd), msg(m), wparam(wp), lparam(lp) {}
};

/**
 * @brief GDI backend for Nuklear rendering - centralized in window manager
 */
struct NKGdiBackend {
    HDC window_dc;
    HDC memory_dc;
    HBITMAP bitmap;
    void* bits;
    int width, height;
    
    NKGdiBackend();
    ~NKGdiBackend();
    
    void Initialize(HWND hwnd, int w, int h);
    void Resize(int w, int h);
    void Render(struct nk_color backgroundColor, struct nk_context* nuklearContext);
    void Cleanup();
    int HandleEvent(HWND eventSourceWindow, UINT msg, WPARAM wparam, LPARAM lparam);
};

/**
 * @brief Window manager class that owns the shared Nuklear context
 */
class NKWindowManager {
public:
    NKWindowManager();
    ~NKWindowManager();
    
    // Context management
    void Initialize();
    void Cleanup();
    // ✅ NO SHARED CONTEXT - each window has its own
    // GetContext() removed - use window->GetContext() instead
    
    // Window management
    void RegisterWindow(NKWindow* window);
    void UnregisterWindow(NKWindow* window);
    void UpdateAll();  // Update all active windows with shared input cycle
    void CleanupAll(); // Cleanup all windows
    
    // ✅ Input handling - routed to target window only
    void ProcessInput(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    
    // Focus and window tracking for smart input routing
    void SetFocusedWindow(HWND hwnd) { m_focusedWindow = hwnd; }
    HWND GetFocusedWindow() const { return m_focusedWindow; }
    
    // Active window management
    void SetActiveWindow(NKWindow* window) { m_activeWindow = window; }
    NKWindow* GetActiveWindow() const { return m_activeWindow; }
    
    // Draw command processing
    void ProcessDrawCommand(std::set<HWND>& windowsNeedingPaint, const struct nk_command* cmd);
    void ProcessDrawCommandForWindow(NKGdiBackend* backend, const struct nk_command* drawCommand);
    
    // GDI backend management
    NKGdiBackend* GetGdiBackend(HWND hwnd);
    void RegisterGdiBackend(HWND hwnd, NKGdiBackend* backend);
    void UnregisterGdiBackend(HWND hwnd);
    
private:
    // ✅ NO SHARED CONTEXT - removed to prevent misuse
    bool m_initialized;
    std::vector<NKWindow*> m_windows;
    
    // Centralized GDI backends for all windows
    std::map<HWND, NKGdiBackend*> m_gdiBackends;
    
    // Smart input routing components
    std::queue<InputEvent> m_inputEvents;
    HWND m_focusedWindow;
    
    // Active window tracking
    NKWindow* m_activeWindow;
    
    void InitializeNuklearContext();
    void ApplyTheme();
    
    // Input routing helpers
    HWND GetWindowUnderCursor();
    bool ShouldReceiveInput(HWND targetWindow, HWND currentWindow, UINT msg);
    void ProcessInputEventForWindow(HWND targetWindow, const InputEvent& event);
};