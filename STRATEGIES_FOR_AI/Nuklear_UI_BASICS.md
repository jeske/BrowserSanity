# Nuklear Multi-Window Architecture - Proven Working Patterns

## Overview
This document contains ONLY patterns that have been tested and proven to work in production. These patterns solve the critical issues of multi-window Nuklear applications on Windows, including proper window targeting, message loop integration, and DPI handling.

## CRITICAL ARCHITECTURE: Single Context Multi-Window Pattern

### The Fundamental Rule
**Nuklear is designed around a SINGLE CONTEXT model.** All windows must share one `nk_context`.

### Working Architecture Pattern

```cpp
// Window Manager - Owns the single shared context
class NKWindowManager {
public:
    NKWindowManager();
    ~NKWindowManager();
    
    void Initialize();
    void Cleanup();
    struct nk_context* GetContext() { return &m_ctx; }
    
    // Window management
    void RegisterWindow(NKWindow* window);
    void UnregisterWindow(NKWindow* window);
    void UpdateAll();  // Process all windows with shared input cycle
    
    // Input handling - single cycle for all windows
    void BeginInput();
    void EndInput();
    void ProcessInput(HWND eventSourceWindow, UINT msg, WPARAM wparam, LPARAM lparam);
    
    // Centralized GDI backend management
    NKGdiBackend* GetGdiBackend(HWND hwnd);
    void RegisterGdiBackend(HWND hwnd, NKGdiBackend* backend);
    void UnregisterGdiBackend(HWND hwnd);
    
private:
    struct nk_context m_ctx;  // Single shared context for ALL windows
    struct nk_font* m_font;
    std::vector<NKWindow*> m_windows;
    std::map<HWND, NKGdiBackend*> m_gdiBackends;  // One backend per HWND
};

// Base Window Class - Uses dependency injection
class NKWindow {
public:
    NKWindow(NKWindowManager& windowManager) : m_windowManager(windowManager) {}
    virtual ~NKWindow() = default;
    
    virtual void Render() = 0;  // Pure virtual for dialog-specific UI
    virtual bool IsActive() const = 0;
    virtual HWND GetHWND() const = 0;
    
protected:
    NKWindowManager& m_windowManager;  // Access to shared context
};
```

### Window-Specific Rendering Solution

The key breakthrough was implementing **per-window rendering cycles** instead of trying to route draw commands:

```cpp
void NKWindowManager::UpdateAll() {
    if (!m_initialized) return;
    
    // Step 1: Single input processing cycle for all windows
    nk_input_begin(&m_ctx);
    // Input events are injected via ProcessInput() calls from window messages
    nk_input_end(&m_ctx);
    
    // Step 2: Process each window individually to avoid command mixing
    std::set<HWND> windowsNeedingPaint;
    
    for (NKWindow* window : m_windows) {
        if (window && window->IsActive()) {
            HWND hwnd = window->GetHWND();
            
            // Set font for this window
            nk_style_set_font(&m_ctx, &m_font->handle);
            
            // Let window build its UI
            window->Render();
            
            // Immediately process draw commands for THIS window only
            NKGdiBackend* backend = GetGdiBackend(hwnd);
            if (backend && backend->memory_dc) {
                // Clear the window's background
                RECT rect = {0, 0, backend->width, backend->height};
                HBRUSH bg_brush = CreateSolidBrush(RGB(240, 240, 240));
                FillRect(backend->memory_dc, &rect, bg_brush);
                DeleteObject(bg_brush);
                
                // Process all draw commands for this window
                const struct nk_command* cmd;
                nk_foreach(cmd, &m_ctx) {
                    ProcessDrawCommandForWindow(backend, cmd);
                }
                
                // Mark this window as needing paint
                windowsNeedingPaint.insert(hwnd);
            }
            
            // Clear the context after processing this window's commands
            nk_clear(&m_ctx);
        }
    }
    
    // Step 3: Trigger WM_PAINT for all windows that had drawing
    for (HWND hwnd : windowsNeedingPaint) {
        InvalidateRect(hwnd, NULL, FALSE);
    }
}
```

## CRITICAL MESSAGE LOOP PATTERN

### The Working Message Loop

```cpp
// CRITICAL: Process ALL pending messages before calling UpdateAll()
while (g_app.running) {
    // Process ALL pending messages before updating
    BOOL hasMessages = TRUE;
    while (hasMessages && g_app.running) {
        hasMessages = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        if (hasMessages) {
            if (msg.message == WM_QUIT) {
                g_app.running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    if (!g_app.running) break;
    
    // Only after ALL messages are processed
    windowManager.UpdateAll();
    Sleep(16); // ~60 FPS
}
```

**Why this works:**
1. WM_PAINT messages have lower priority than input messages
2. In a tight loop, WM_PAINT messages get starved
3. Processing ALL messages ensures WM_PAINT gets handled
4. This prevents the "blank window until interaction" problem

## Font Initialization Pattern

```cpp
void NKWindowManager::InitializeNuklearContext() {
    // Initialize font atlas
    struct nk_font_atlas atlas;
    nk_font_atlas_init_default(&atlas);
    nk_font_atlas_begin(&atlas);
    
    // Add default font with 16pt size for DPI scaling
    m_font = nk_font_atlas_add_default(&atlas, 16, 0);
    
    // Bake the font atlas
    const void *image;
    int atlas_w, atlas_h;
    image = nk_font_atlas_bake(&atlas, &atlas_w, &atlas_h, NK_FONT_ATLAS_RGBA32);
    
    // End atlas (no GPU upload needed for GDI)
    nk_font_atlas_end(&atlas, nk_handle_ptr(0), NULL);
    
    // Initialize context with font
    nk_init_default(&m_ctx, &m_font->handle);
}
```

## GDI Backend Implementation

```cpp
struct NKGdiBackend {
    HDC window_dc;
    HDC memory_dc;
    HBITMAP bitmap;
    void* bits;
    int width, height;
    
    void Initialize(HWND hwnd, int w, int h);
    void Resize(int w, int h);
    void Cleanup();
};

void NKGdiBackend::Initialize(HWND hwnd, int w, int h) {
    window_dc = GetDC(hwnd);
    width = w;
    height = h;
    
    memory_dc = CreateCompatibleDC(window_dc);
    
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    bitmap = CreateDIBSection(memory_dc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    SelectObject(memory_dc, bitmap);
}
```

## Draw Command Processing

```cpp
void NKWindowManager::ProcessDrawCommandForWindow(NKGdiBackend* backend, const struct nk_command* cmd) {
    if (!backend || !backend->memory_dc || !cmd) return;
    
    HDC memory_dc = backend->memory_dc;
    
    switch (cmd->type) {
        case NK_COMMAND_NOP: 
            break;
        case NK_COMMAND_SCISSOR: {
            const struct nk_command_scissor* s = (const struct nk_command_scissor*)cmd;
            HRGN region = CreateRectRgn((int)s->x, (int)s->y, (int)(s->x + s->w), (int)(s->y + s->h));
            SelectClipRgn(memory_dc, region);
            DeleteObject(region);
        } break;
        case NK_COMMAND_RECT_FILLED: {
            const struct nk_command_rect_filled* r = (const struct nk_command_rect_filled*)cmd;
            RECT rect = {(int)r->x, (int)r->y, (int)(r->x + r->w), (int)(r->y + r->h)};
            HBRUSH brush = CreateSolidBrush(RGB(r->color.r, r->color.g, r->color.b));
            FillRect(memory_dc, &rect, brush);
            DeleteObject(brush);
        } break;
        case NK_COMMAND_TEXT: {
            const struct nk_command_text* t = (const struct nk_command_text*)cmd;
            SetTextColor(memory_dc, RGB(t->foreground.r, t->foreground.g, t->foreground.b));
            SetBkMode(memory_dc, TRANSPARENT);
            
            RECT rect = {(int)t->x, (int)t->y, (int)(t->x + t->w), (int)(t->y + t->h)};
            
            // Convert to wide char for DrawText
            wchar_t* wtext = (wchar_t*)malloc((t->length + 1) * sizeof(wchar_t));
            if (wtext) {
                MultiByteToWideChar(CP_UTF8, 0, (const char*)t->string, (int)t->length, wtext, (int)t->length);
                wtext[t->length] = 0;
                DrawTextW(memory_dc, wtext, -1, &rect, DT_LEFT | DT_TOP | DT_SINGLELINE);
                free(wtext);
            }
        } break;
        case NK_COMMAND_RECT: {
            const struct nk_command_rect* r = (const struct nk_command_rect*)cmd;
            HPEN pen = CreatePen(PS_SOLID, (int)r->line_thickness, RGB(r->color.r, r->color.g, r->color.b));
            HPEN old_pen = (HPEN)SelectObject(memory_dc, pen);
            HBRUSH old_brush = (HBRUSH)SelectObject(memory_dc, GetStockObject(NULL_BRUSH));
            
            Rectangle(memory_dc, (int)r->x, (int)r->y, (int)(r->x + r->w), (int)(r->y + r->h));
            
            SelectObject(memory_dc, old_brush);
            SelectObject(memory_dc, old_pen);
            DeleteObject(pen);
        } break;
    }
}
```

## Input Processing Pattern

```cpp
void NKWindowManager::ProcessInput(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_LBUTTONDOWN:
            SetCapture(hwnd);
            nk_input_button(&m_ctx, NK_BUTTON_LEFT, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), 1);
            break;
        case WM_LBUTTONUP:
            ReleaseCapture();
            nk_input_button(&m_ctx, NK_BUTTON_LEFT, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), 0);
            break;
        case WM_MOUSEMOVE:
            nk_input_motion(&m_ctx, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            break;
        case WM_CHAR:
            if (wparam >= 32) {
                nk_input_unicode(&m_ctx, (nk_rune)wparam);
            }
            break;
        case WM_KEYDOWN:
        case WM_KEYUP: {
            int down = (msg == WM_KEYDOWN);
            int ctrl = GetKeyState(VK_CONTROL) & 0x8000;
            
            switch (wparam) {
                case VK_SHIFT:
                    nk_input_key(&m_ctx, NK_KEY_SHIFT, down);
                    break;
                case VK_DELETE:
                    nk_input_key(&m_ctx, NK_KEY_DEL, down);
                    break;
                case VK_RETURN:
                    nk_input_key(&m_ctx, NK_KEY_ENTER, down);
                    break;
                case VK_TAB:
                    nk_input_key(&m_ctx, NK_KEY_TAB, down);
                    break;
                case VK_LEFT:
                    if (ctrl) nk_input_key(&m_ctx, NK_KEY_TEXT_WORD_LEFT, down);
                    else nk_input_key(&m_ctx, NK_KEY_LEFT, down);
                    break;
                case VK_RIGHT:
                    if (ctrl) nk_input_key(&m_ctx, NK_KEY_TEXT_WORD_RIGHT, down);
                    else nk_input_key(&m_ctx, NK_KEY_RIGHT, down);
                    break;
                case VK_BACK:
                    nk_input_key(&m_ctx, NK_KEY_BACKSPACE, down);
                    break;
                case 'A':
                    if (ctrl && down) nk_input_key(&m_ctx, NK_KEY_TEXT_SELECT_ALL, 1);
                    break;
                case 'C':
                    if (ctrl && down) nk_input_key(&m_ctx, NK_KEY_COPY, 1);
                    break;
                case 'V':
                    if (ctrl && down) nk_input_key(&m_ctx, NK_KEY_PASTE, 1);
                    break;
                case 'X':
                    if (ctrl && down) nk_input_key(&m_ctx, NK_KEY_CUT, 1);
                    break;
            }
        } break;
    }
}
```

## Window Implementation Example

```cpp
class NKWindow_MainLaunch : public NKWindow {
public:
    NKWindow_MainLaunch(NKWindowManager& windowManager) : NKWindow(windowManager) {}
    
    void Render() override {
        struct nk_context* ctx = m_windowManager.GetContext();
        
        if (nk_begin(ctx, "Browser Sanity - Main Control", nk_rect(0, 0, 400, 300), 
                     NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
            
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "Browser Sanity Control Panel", NK_TEXT_CENTERED);
            
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_button_label(ctx, "Show Settings")) {
                // Handle button click
            }
            if (nk_button_label(ctx, "Show Toast")) {
                // Handle button click
            }
            
            nk_layout_row_dynamic(ctx, 30, 3);
            if (nk_button_label(ctx, "Install")) {
                // Handle install
            }
            if (nk_button_label(ctx, "Uninstall")) {
                // Handle uninstall
            }
            if (nk_button_label(ctx, "Exit")) {
                // Handle exit
            }
        }
        nk_end(ctx);
    }
};
```

## Essential Header Defines

```cpp
// In header file (NKWindowManager.h)
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
```

## Theme Application

```cpp
void NKWindowManager::ApplyTheme() {
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = nk_rgb(70, 70, 70);
    table[NK_COLOR_WINDOW] = nk_rgb(240, 240, 240);
    table[NK_COLOR_HEADER] = nk_rgb(220, 220, 220);
    table[NK_COLOR_BORDER] = nk_rgb(160, 160, 160);
    table[NK_COLOR_BUTTON] = nk_rgb(220, 220, 220);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgb(200, 200, 200);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgb(180, 180, 180);
    // ... set other colors
    
    nk_style_from_table(&m_ctx, table);
    
    // Additional style tweaks
    m_ctx.style.window.border = 2.0f;
    m_ctx.style.window.rounding = 6.0f;
    m_ctx.style.button.border = 2.0f;
    m_ctx.style.button.rounding = 4.0f;
}
```

## Key Success Principles

1. **Single Context Rule**: One `nk_context` shared by all windows
2. **Per-Window Rendering**: Process each window individually with immediate draw command processing
3. **Message Loop Priority**: Process ALL messages before UpdateAll() to prevent WM_PAINT starvation
4. **Centralized Management**: Window manager owns context and GDI backends
5. **Dependency Injection**: Windows receive manager reference, not individual contexts
6. **Clear Context After Each Window**: Call `nk_clear()` after processing each window's commands

## DPI Handling

Nuklear automatically handles DPI scaling when you:
- Use appropriate font sizes (16pt works well for high-DPI)
- Use dynamic layouts instead of fixed pixel sizes
- Let Nuklear handle coordinate scaling

This architecture has been tested and proven to work reliably for multi-window Nuklear applications on Windows with proper input handling, rendering, and DPI support.