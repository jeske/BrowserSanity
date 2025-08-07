/**
 * @file NKWindowManager.cpp
 * @brief Implementation of window manager with shared Nuklear context
 */

#include <NKWindowManager.h>
#include <NKWindow.h>
#include <main.h>
#include <debug_log.h>
#include <windowsx.h>
#include <algorithm>
#include <set>

NKWindowManager::NKWindowManager()
    : m_font(nullptr), m_initialized(false), m_focusedWindow(nullptr) {
    memset(&m_ctx, 0, sizeof(m_ctx));
}

NKWindowManager::~NKWindowManager() {
    Cleanup();
}

void NKWindowManager::Initialize() {
    if (m_initialized) return;
    
    InitializeNuklearContext();
    ApplyTheme();
    m_initialized = true;
}

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

void NKWindowManager::ApplyTheme() {
    // Apply light theme colors
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = NK_THEME_TEXT;
    table[NK_COLOR_WINDOW] = NK_THEME_WINDOW;
    table[NK_COLOR_HEADER] = NK_THEME_HEADER;
    table[NK_COLOR_BORDER] = NK_THEME_BORDER;
    table[NK_COLOR_BUTTON] = nk_rgb(220, 220, 220);           // Medium gray - inactive button background
    table[NK_COLOR_BUTTON_HOVER] = nk_rgb(200, 200, 200);     // Darker gray - button hover state
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
    nk_style_from_table(&m_ctx, table);
    
    // Additional style tweaks for better appearance
    m_ctx.style.window.border = 2.0f;                           // 2px borders for visibility
    m_ctx.style.window.rounding = 6.0f;                         // 6px rounded corners
    m_ctx.style.window.border_color = nk_rgb(160, 160, 160);    // Medium-dark gray - window borders
    m_ctx.style.button.border = 2.0f;                           // 2px button borders for visibility
    m_ctx.style.button.rounding = 4.0f;                         // 4px rounded button corners
    m_ctx.style.button.border_color = nk_rgb(160, 160, 160);    // Medium-dark gray - button borders
    m_ctx.style.edit.border = 1.0f;                             // 1px input field borders
    m_ctx.style.edit.rounding = 4.0f;                           // 4px rounded input corners
    m_ctx.style.edit.border_color = NK_THEME_BORDER;            // Use theme border color
}

void NKWindowManager::Cleanup() {
    if (m_initialized) {
        nk_free(&m_ctx);
        m_initialized = false;
    }
}

void NKWindowManager::RegisterWindow(NKWindow* window) {
    if (window && std::find(m_windows.begin(), m_windows.end(), window) == m_windows.end()) {
        m_windows.push_back(window);
    }
}

void NKWindowManager::UnregisterWindow(NKWindow* window) {
    auto it = std::find(m_windows.begin(), m_windows.end(), window);
    if (it != m_windows.end()) {
        m_windows.erase(it);
    }
}

void NKWindowManager::UpdateAll() {
    if (!m_initialized) {
        DebugLog("UpdateAll: Not initialized, skipping");
        return;
    }
    
    DebugLog("UpdateAll: Starting update cycle");
    
    // Step 1: Begin input processing with smart routing
    nk_input_begin(&m_ctx);
    // Process queued input events with smart routing - events are now targeted
    DebugLog("UpdateAll: Processing %d queued input events", (int)m_inputEvents.size());
    nk_input_end(&m_ctx);
    DebugLog("UpdateAll: Input cycle completed");
    
    // Step 2: Process each window individually with targeted input injection
    int activeWindows = 0;
    std::set<HWND> windowsNeedingPaint;
    
    for (NKWindow* window : m_windows) {
        if (window && window->IsActive()) {
            activeWindows++;
            HWND hwnd = window->GetHWND();
            
            // Set font for this window
            nk_style_set_font(&m_ctx, &m_font->handle);
            
            DebugLog("UpdateAll: Processing window HWND %p", (void*)hwnd);
            
            // Process input events targeted for this specific window
            std::queue<InputEvent> remainingEvents;
            while (!m_inputEvents.empty()) {
                InputEvent event = m_inputEvents.front();
                m_inputEvents.pop();
                
                if (ShouldReceiveInput(event.target_hwnd, hwnd, event.msg)) {
                    // This event is for the current window - process it
                    ProcessInputEventForWindow(hwnd, event);
                    DebugLog("UpdateAll: Processed input event for window %p", (void*)hwnd);
                } else {
                    // This event is for a different window - keep it for later
                    remainingEvents.push(event);
                }
            }
            // Restore remaining events for other windows
            m_inputEvents = remainingEvents;
            
            // Let window build its UI using existing Render() method
            window->Render();
            
            // Immediately process draw commands for THIS window only
            NKGdiBackend* backend = GetGdiBackend(hwnd);
            if (backend && backend->memory_dc) {
                // Clear the window's background
                RECT rect = {0, 0, backend->width, backend->height};
                HBRUSH bg_brush = CreateSolidBrush(RGB(240, 240, 240)); // Light gray background
                FillRect(backend->memory_dc, &rect, bg_brush);
                DeleteObject(bg_brush);
                
                // Process all draw commands for this window
                const struct nk_command* cmd;
                int commandCount = 0;
                nk_foreach(cmd, &m_ctx) {
                    commandCount++;
                    ProcessDrawCommandForWindow(backend, cmd);
                }
                
                DebugLog("UpdateAll: Processed %d commands for window HWND %p", commandCount, (void*)hwnd);
                
                // Mark this window as needing paint
                windowsNeedingPaint.insert(hwnd);
            }
            
            // Clear the context after processing this window's commands
            nk_clear(&m_ctx);
        }
    }
    
    DebugLog("UpdateAll: Processed %d active windows", activeWindows);
    
    // Step 3: Trigger WM_PAINT for all windows that had drawing
    for (HWND hwnd : windowsNeedingPaint) {
        InvalidateRect(hwnd, NULL, FALSE);
        DebugLog("UpdateAll: InvalidateRect called for HWND %p", hwnd);
    }
    
    DebugLog("UpdateAll: Update cycle complete");
}

void NKWindowManager::BeginInput() {
    nk_input_begin(&m_ctx);
}

void NKWindowManager::EndInput() {
    nk_input_end(&m_ctx);
}

void NKWindowManager::ProcessInput(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    // Determine target window for this input event
    HWND targetWindow = nullptr;
    
    switch (msg) {
        // Keyboard events go to focused window
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
            targetWindow = m_focusedWindow;
            break;
            
        // Mouse events go to window under cursor
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
        case WM_MOUSEWHEEL:
            targetWindow = GetWindowUnderCursor();
            break;
            
        default:
            targetWindow = hwnd; // Default to the window that received the message
            break;
    }
    
    // Queue the event with its target window
    if (targetWindow) {
        m_inputEvents.push(InputEvent(targetWindow, msg, wparam, lparam));
        DebugLog("ProcessInput: Queued %s event for window %p",
                 (msg >= WM_KEYFIRST && msg <= WM_KEYLAST) ? "keyboard" : "mouse",
                 (void*)targetWindow);
    }
}

void NKWindowManager::ProcessInputEventForWindow(HWND targetWindow, const InputEvent& event) {
    // Only process the event if it's targeted for the current window
    UINT msg = event.msg;
    WPARAM wparam = event.wparam;
    LPARAM lparam = event.lparam;
    
    switch (msg) {
        case WM_KEYDOWN:
        case WM_KEYUP: {
            int down = (msg == WM_KEYDOWN);
            int ctrl = GetKeyState(VK_CONTROL) & 0x8000;
            
            switch (wparam) {
                case VK_SHIFT:
                case VK_LSHIFT:
                case VK_RSHIFT:
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
                case VK_HOME:
                    nk_input_key(&m_ctx, NK_KEY_TEXT_START, down);
                    break;
                case VK_END:
                    nk_input_key(&m_ctx, NK_KEY_TEXT_END, down);
                    break;
                case 'A':
                    if (ctrl && down) {
                        nk_input_key(&m_ctx, NK_KEY_TEXT_SELECT_ALL, 1);
                    }
                    break;
                case 'C':
                    if (ctrl && down) {
                        nk_input_key(&m_ctx, NK_KEY_COPY, 1);
                    }
                    break;
                case 'V':
                    if (ctrl && down) {
                        nk_input_key(&m_ctx, NK_KEY_PASTE, 1);
                    }
                    break;
                case 'X':
                    if (ctrl && down) {
                        nk_input_key(&m_ctx, NK_KEY_CUT, 1);
                    }
                    break;
            }
            break;
        }
        case WM_CHAR:
            if (wparam >= 32) {
                nk_input_unicode(&m_ctx, (nk_rune)wparam);
            }
            break;
        case WM_LBUTTONDOWN:
            SetCapture(targetWindow);
            nk_input_button(&m_ctx, NK_BUTTON_LEFT, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), 1);
            break;
        case WM_LBUTTONUP:
            ReleaseCapture();
            nk_input_button(&m_ctx, NK_BUTTON_LEFT, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), 0);
            break;
        case WM_RBUTTONDOWN:
            SetCapture(targetWindow);
            nk_input_button(&m_ctx, NK_BUTTON_RIGHT, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), 1);
            break;
        case WM_RBUTTONUP:
            ReleaseCapture();
            nk_input_button(&m_ctx, NK_BUTTON_RIGHT, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), 0);
            break;
        case WM_MOUSEMOVE:
            nk_input_motion(&m_ctx, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            break;
        case WM_MOUSEWHEEL:
            nk_input_scroll(&m_ctx, nk_vec2(0, (float)(short)HIWORD(wparam) / WHEEL_DELTA));
            break;
    }
}

HWND NKWindowManager::GetWindowUnderCursor() {
    POINT cursor;
    GetCursorPos(&cursor);
    HWND windowUnderCursor = WindowFromPoint(cursor);
    
    // Check if this window is one of our managed windows
    for (NKWindow* window : m_windows) {
        if (window && window->GetHWND() == windowUnderCursor) {
            return windowUnderCursor;
        }
    }
    
    // If cursor is not over any of our windows, return the focused window as fallback
    return m_focusedWindow;
}

bool NKWindowManager::ShouldReceiveInput(HWND targetWindow, HWND currentWindow, UINT msg) {
    // Only the target window should receive the input
    return (targetWindow == currentWindow);
}

void NKWindowManager::ProcessDrawCommand(std::set<HWND>& windowsNeedingPaint, const struct nk_command* cmd) {
    // This function is now deprecated - we use ProcessDrawCommandForWindow instead
    // Keeping for compatibility but it should not be called in the new architecture
    DebugLog("ProcessDrawCommand: Deprecated function called - should use ProcessDrawCommandForWindow");
}

void NKWindowManager::ProcessDrawCommandForWindow(NKGdiBackend* backend, const struct nk_command* cmd) {
    if (!backend || !backend->memory_dc || !cmd) {
        DebugLog("ProcessDrawCommandForWindow: Invalid parameters");
        return;
    }
    
    HDC memory_dc = backend->memory_dc;
    
    // Process the draw command for the specific window
    switch (cmd->type) {
        case NK_COMMAND_NOP:
            break;
        case NK_COMMAND_SCISSOR: {
            const struct nk_command_scissor* s = (const struct nk_command_scissor*)cmd;
            HRGN region = CreateRectRgn((int)s->x, (int)s->y, (int)(s->x + s->w), (int)(s->y + s->h));
            SelectClipRgn(memory_dc, region);
            DeleteObject(region);
            DebugLog("ProcessDrawCommandForWindow: Applied scissor region");
        } break;
        case NK_COMMAND_RECT_FILLED: {
            const struct nk_command_rect_filled* r = (const struct nk_command_rect_filled*)cmd;
            RECT rect = {(int)r->x, (int)r->y, (int)(r->x + r->w), (int)(r->y + r->h)};
            HBRUSH brush = CreateSolidBrush(RGB(r->color.r, r->color.g, r->color.b));
            FillRect(memory_dc, &rect, brush);
            DeleteObject(brush);
            DebugLog("ProcessDrawCommandForWindow: Drew filled rectangle");
        } break;
        case NK_COMMAND_TEXT: {
            const struct nk_command_text* t = (const struct nk_command_text*)cmd;
            SetTextColor(memory_dc, RGB(t->foreground.r, t->foreground.g, t->foreground.b));
            SetBkMode(memory_dc, TRANSPARENT);
            
            // Give text more vertical space to prevent clipping - add 50% extra height
            int extra_height = (int)(t->h * 0.5f);
            RECT rect = {(int)t->x, (int)t->y - extra_height/2, (int)(t->x + t->w), (int)(t->y + t->h + extra_height/2)};
            
            // Convert to wide char for DrawText
            wchar_t* wtext = (wchar_t*)malloc((t->length + 1) * sizeof(wchar_t));
            if (wtext) {
                MultiByteToWideChar(CP_UTF8, 0, (const char*)t->string, (int)t->length, wtext, (int)t->length);
                wtext[t->length] = 0;
                
                // Use DT_VCENTER to properly center text vertically in the expanded rectangle
                DrawTextW(memory_dc, wtext, -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                free(wtext);
                DebugLog("ProcessDrawCommandForWindow: Drew text with expanded bounds");
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
            DebugLog("ProcessDrawCommandForWindow: Drew rectangle outline");
        } break;
        default:
            DebugLog("ProcessDrawCommandForWindow: Unhandled command type %d", cmd->type);
            break;
    }
}

NKGdiBackend* NKWindowManager::GetGdiBackend(HWND hwnd) {
    auto it = m_gdiBackends.find(hwnd);
    return (it != m_gdiBackends.end()) ? it->second : nullptr;
}

void NKWindowManager::RegisterGdiBackend(HWND hwnd, NKGdiBackend* backend) {
    if (hwnd && backend) {
        m_gdiBackends[hwnd] = backend;
    }
}

void NKWindowManager::UnregisterGdiBackend(HWND hwnd) {
    auto it = m_gdiBackends.find(hwnd);
    if (it != m_gdiBackends.end()) {
        delete it->second;
        m_gdiBackends.erase(it);
    }
}

void NKWindowManager::CleanupAll() {
    // Cleanup all GDI backends
    for (auto& pair : m_gdiBackends) {
        delete pair.second;
    }
    m_gdiBackends.clear();
    
    // Cleanup all windows
    for (NKWindow* window : m_windows) {
        if (window) {
            window->DestroyWindow();
        }
    }
    m_windows.clear();
}

// NKGdiBackend Implementation
NKGdiBackend::NKGdiBackend()
    : window_dc(nullptr), memory_dc(nullptr), bitmap(nullptr), bits(nullptr)
    , width(0), height(0) {
}

NKGdiBackend::~NKGdiBackend() {
    Cleanup();
}

void NKGdiBackend::Initialize(HWND hwnd, int w, int h) {
    window_dc = GetDC(hwnd);
    width = w;
    height = h;
    
    // Create initial bitmap
    Resize(w, h);
}

void NKGdiBackend::Resize(int w, int h) {
    if (memory_dc) {
        if (bitmap) {
            SelectObject(memory_dc, NULL);
            DeleteObject(bitmap);
        }
        DeleteDC(memory_dc);
    }
    
    width = w;
    height = h;
    
    if (width <= 0 || height <= 0) return;
    
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

void NKGdiBackend::Render(struct nk_color bg_color, struct nk_context* ctx) {
    const struct nk_command* cmd;
    
    if (!memory_dc || !bits || !ctx) return;
    
    // Clear background
    RECT rect = {0, 0, width, height};
    HBRUSH bg_brush = CreateSolidBrush(RGB(bg_color.r, bg_color.g, bg_color.b));
    FillRect(memory_dc, &rect, bg_brush);
    DeleteObject(bg_brush);
    
    // Render nuklear commands
    nk_foreach(cmd, ctx) {
        switch (cmd->type) {
        case NK_COMMAND_NOP: break;
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
            
            // Give text more vertical space to prevent clipping - add 50% extra height
            int extra_height = (int)(t->h * 0.5f);
            RECT rect = {(int)t->x, (int)t->y - extra_height/2, (int)(t->x + t->w), (int)(t->y + t->h + extra_height/2)};
            
            // Convert to wide char for DrawText
            wchar_t* wtext = (wchar_t*)malloc((t->length + 1) * sizeof(wchar_t));
            MultiByteToWideChar(CP_UTF8, 0, (const char*)t->string, (int)t->length, wtext, (int)t->length);
            wtext[t->length] = 0;
            
            // Use DT_VCENTER to properly center text vertically in the expanded rectangle
            DrawTextW(memory_dc, wtext, -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            free(wtext);
        } break;
        default: break;
        }
    }
    
    // Reset clipping
    SelectClipRgn(memory_dc, NULL);
}

void NKGdiBackend::Cleanup() {
    if (bitmap) {
        DeleteObject(bitmap);
        bitmap = nullptr;
    }
    if (memory_dc) {
        DeleteDC(memory_dc);
        memory_dc = nullptr;
    }
    if (window_dc) {
        ReleaseDC(GetActiveWindow(), window_dc); // Note: should store HWND
        window_dc = nullptr;
    }
}

int NKGdiBackend::HandleEvent(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_CLOSE:
            // Don't quit the entire app when a single window closes
            // Let the main window procedure handle this
            return 0;
        case WM_DESTROY:
        case WM_QUIT:
            g_app.running = false;
            PostQuitMessage(0);
            return 0;
        default:
            return 0;
    }
}