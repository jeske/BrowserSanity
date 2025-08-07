/**
 * @file NKWindowManager.h
 * @brief PROVEN WORKING PATTERN: Single Context Multi-Window Nuklear Architecture
 *
 * CRITICAL SUCCESS PATTERN: Per-Window Rendering with Shared Context
 *
 * This architecture has been tested and proven to work reliably for multi-window
 * Nuklear applications. It solves the critical window targeting problem through
 * individual window processing rather than command routing.
 *
 * WORKING APPROACH: Single shared nk_context with per-window rendering
 * - One NKWindowManager owns the single nk_context (Nuklear's intended design)
 * - Each window processed individually: Render() → ProcessCommands() → nk_clear()
 * - Immediate draw command processing prevents command mixing between windows
 * - Centralized GDI backend management (one backend per HWND)
 * - Single input processing cycle for all windows
 * - Dependency injection pattern for clean architecture
 *
 * KEY SUCCESS PRINCIPLES:
 * 1. Single nk_context shared across ALL windows (never multiple contexts)
 * 2. Process each window individually with immediate command processing
 * 3. Call nk_clear() after each window to prevent command mixing
 * 4. Process ALL Windows messages before UpdateAll() to prevent WM_PAINT starvation
 * 5. Centralized input processing (one BeginInput/EndInput cycle)
 * 6. One GDI backend per HWND, managed centrally
 * 7. Dependency injection for window manager access
 *
 * This pattern eliminates:
 * - Blank window issues (proper message loop)
 * - All windows showing same content (per-window processing)
 * - Sequence assertion failures (single context)
 * - Input handling conflicts (centralized input)
 */

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <map>
#include <set>

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
    void Render(struct nk_color bg_color, struct nk_context* ctx);
    void Cleanup();
    int HandleEvent(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
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
    struct nk_context* GetContext() { return &m_ctx; }
    struct nk_font* GetFont() { return m_font; }
    
    // Window management
    void RegisterWindow(NKWindow* window);
    void UnregisterWindow(NKWindow* window);
    void UpdateAll();  // Update all active windows with shared input cycle
    void CleanupAll(); // Cleanup all windows
    
    // Input handling - single cycle for all windows
    void BeginInput();
    void EndInput();
    void ProcessInput(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    
    // Draw command processing
    void ProcessDrawCommand(std::set<HWND>& windowsNeedingPaint, const struct nk_command* cmd);
    void ProcessDrawCommandForWindow(NKGdiBackend* backend, const struct nk_command* cmd);
    
    // GDI backend management
    NKGdiBackend* GetGdiBackend(HWND hwnd);
    void RegisterGdiBackend(HWND hwnd, NKGdiBackend* backend);
    void UnregisterGdiBackend(HWND hwnd);
    
private:
    struct nk_context m_ctx;
    struct nk_font* m_font;
    bool m_initialized;
    std::vector<NKWindow*> m_windows;
    
    // Centralized GDI backends for all windows
    std::map<HWND, NKGdiBackend*> m_gdiBackends;
    
    void InitializeNuklearContext();
    void ApplyTheme();
};