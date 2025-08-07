/**
 * @file NKWindowManager.cpp
 * @brief 🎯 BREAKTHROUGH: Context-Per-Window Architecture Implementation
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
 * Key Architecture Features:
 * - Each window has its own nk_context via NKWindow::m_nuklearContext
 * - Input events are routed to target window ONLY (no broadcasting)
 * - Complete state isolation prevents global state conflicts
 * - Independent font and theme management per window
 */

#include <NKWindowManager.h>
#include <NKWindow.h>
#include <main.h>
#include <debug_log.h>
#include <windowsx.h>
#include <algorithm>
#include <set>

NKWindowManager::NKWindowManager()
    : m_font(nullptr), m_initialized(false), m_focusedWindow(nullptr), m_activeWindow(nullptr) {
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
    
    
    // ✅ CONTEXT-PER-WINDOW ARCHITECTURE: Complete input isolation prevents assertion failures
    // Each window processes ONLY its own input events with its own Nuklear context
    int activeWindowCount = 0;
    std::set<HWND> windowsNeedingPaint;
    
    for (NKWindow* window : m_windows) {
        if (window) {
            activeWindowCount++;
            HWND hwndBeingProcessed = window->GetHWND();
            struct nk_context* windowContext = window->GetContext();
            
            if (!windowContext) {
                DebugLog("UpdateAll: Window %p has no context, skipping", (void*)hwndBeingProcessed);
                continue;
            }
            
            // ✅ CRITICAL: Each window uses its OWN context - prevents assertion failures
            // This ensures input window always matches context window
            nk_input_begin(windowContext);
            
            // Process input events targeted for this specific window
            std::queue<InputEvent> eventsForThisWindow;
            std::queue<InputEvent> remainingEvents;
            
            while (!m_inputEvents.empty()) {
                InputEvent event = m_inputEvents.front();
                m_inputEvents.pop();
                
                if (event.target_hwnd == hwndBeingProcessed) {
                    eventsForThisWindow.push(event);
                } else {
                    remainingEvents.push(event);
                }
            }
            
            // Restore events not for this window
            m_inputEvents = remainingEvents;
            
            // Process events for this window with its own context
            int inputEventCount = 0;
            while (!eventsForThisWindow.empty()) {
                InputEvent event = eventsForThisWindow.front();
                eventsForThisWindow.pop();
                window->ProcessInputEventForWindow(hwndBeingProcessed, event.msg, event.wparam, event.lparam);
                inputEventCount++;
            }
            
            nk_input_end(windowContext);
            
            // Input events processed successfully with isolated context
            
            // ✅ Let window build its UI with its OWN isolated context - no shared state
            window->Render();
            
            // Process draw commands for THIS window only
            NKGdiBackend* backend = GetGdiBackend(hwndBeingProcessed);
            if (backend && backend->memory_dc) {
                // Clear the window's background
                RECT backgroundRect = {0, 0, backend->width, backend->height};
                HBRUSH backgroundBrush = CreateSolidBrush(RGB(240, 240, 240)); // Light gray background
                FillRect(backend->memory_dc, &backgroundRect, backgroundBrush);
                DeleteObject(backgroundBrush);
                
                // ✅ Process draw commands for THIS window's context only - complete isolation
                const struct nk_command* drawCommand;
                int commandCount = 0;
                nk_foreach(drawCommand, windowContext) {
                    commandCount++;
                    ProcessDrawCommandForWindow(backend, drawCommand);
                }
                
                
                // Mark this window as needing paint
                windowsNeedingPaint.insert(hwndBeingProcessed);
            }
            
            // ✅ Clear THIS window's context after processing - maintains isolation
            nk_clear(windowContext);
        }
    }
    
    
    // Step 2: Trigger WM_PAINT for all windows that had drawing
    for (HWND hwndNeedingPaint : windowsNeedingPaint) {
        InvalidateRect(hwndNeedingPaint, NULL, FALSE);
        // DebugLog("UpdateAll: InvalidateRect called for HWND %p", hwndNeedingPaint);
    }
    
    // DebugLog("UpdateAll: Context-per-window update cycle complete");
}

void NKWindowManager::BeginInput() {
    nk_input_begin(&m_ctx);
}

void NKWindowManager::EndInput() {
    nk_input_end(&m_ctx);
}

void NKWindowManager::ProcessInput(HWND hwndMessageReceiver, UINT msg, WPARAM wparam, LPARAM lparam) {
    // Determine target window for this input event
    HWND hwndEventTarget = nullptr;
    
    switch (msg) {
        
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
            // Keyboard events only to the active window 
            if (m_activeWindow) {
                hwndEventTarget = m_activeWindow->GetHWND();
            }
            break;
            
        // Mouse events go to the window that received the message (Windows already did the bounds testing)
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP: {
            // Log click events with desktop coordinates
            POINT cursor;
            GetCursorPos(&cursor);
            hwndEventTarget = hwndMessageReceiver; // Use the window that received the message
            DebugLog("CLICK EVENT: %s at desktop coords (%d,%d) -> message receiver %p -> event target %p",
                     (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) ? "LEFT" : "RIGHT",
                     cursor.x, cursor.y, (void*)hwndMessageReceiver, (void*)hwndEventTarget);
            break;
        }
        case WM_MOUSEMOVE:
        case WM_MOUSEWHEEL:
            hwndEventTarget = hwndMessageReceiver; // Use the window that received the message
            break;
            
        default:
            hwndEventTarget = hwndMessageReceiver; // Default to the window that received the message
            break;
    }
    
    // Queue the event with its target window
    if (hwndEventTarget) {
        m_inputEvents.push(InputEvent(hwndEventTarget, msg, wparam, lparam));
        // Only log click events to reduce noise
        if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP || msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) {
            DebugLog("CLICK QUEUED: %s event from receiver %p for target %p",
                     (msg >= WM_KEYFIRST && msg <= WM_KEYLAST) ? "keyboard" : "mouse",
                     (void*)hwndMessageReceiver, (void*)hwndEventTarget);
        }
    }
}

void NKWindowManager::ProcessInputEventForWindow(HWND hwndNuklearReceiver, const InputEvent& event) {    
    UINT msg = event.msg;
    WPARAM wparam = event.wparam;
    LPARAM lparam = event.lparam;

    // Events are already filtered by ShouldReceiveInput() in UpdateAll() before reaching here

    
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
        case WM_LBUTTONDOWN: {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            // Left mouse button down processed
            SetCapture(hwndNuklearReceiver);
            nk_input_button(&m_ctx, NK_BUTTON_LEFT, x, y, 1);
            break;
        }
        case WM_LBUTTONUP: {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            // Left mouse button up processed
            ReleaseCapture();
            nk_input_button(&m_ctx, NK_BUTTON_LEFT, x, y, 0);
            break;
        }
        case WM_RBUTTONDOWN: {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            // Right mouse button down processed
            SetCapture(hwndNuklearReceiver);
            nk_input_button(&m_ctx, NK_BUTTON_RIGHT, x, y, 1);
            break;
        }
        case WM_RBUTTONUP: {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            // Right mouse button up processed
            ReleaseCapture();
            nk_input_button(&m_ctx, NK_BUTTON_RIGHT, x, y, 0);
            break;
        }
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

bool NKWindowManager::ShouldReceiveInput(HWND hwndEventTarget, HWND hwndBeingProcessed, UINT msg) {
    // Keyboard events: only active window receives them
    if (msg >= WM_KEYFIRST && msg <= WM_KEYLAST) {
        // hwndEventTarget was already set to active window in ProcessInput(), so just check if it matches
        bool shouldReceive = (hwndEventTarget == hwndBeingProcessed);
        // DebugLog("ShouldReceiveInput: Keyboard event target=%p being_processed=%p active=%p -> %s",
        //          (void*)hwndEventTarget, (void*)hwndBeingProcessed,
        //          m_activeWindow ? (void*)m_activeWindow->GetHWND() : nullptr,
        //          shouldReceive ? "YES" : "NO");
        return shouldReceive;
    }
    
    // Mouse events: only the target window receives them (already filtered by bounds)
    // Only log click event filtering to reduce noise
    if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP || msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) {
        DebugLog("CLICK FILTER: target=%p being_processed=%p -> %s",
                 (void*)hwndEventTarget, (void*)hwndBeingProcessed,
                 (hwndEventTarget == hwndBeingProcessed) ? "YES" : "NO");
    }
    return (hwndEventTarget == hwndBeingProcessed);
}

void NKWindowManager::ProcessDrawCommand(std::set<HWND>& windowsNeedingPaint, const struct nk_command* cmd) {
    // This function is deprecated in context-per-window architecture
    // Use ProcessDrawCommandForWindow instead for proper window isolation
}

void NKWindowManager::ProcessDrawCommandForWindow(NKGdiBackend* backend, const struct nk_command* drawCommand) {
    if (!backend || !backend->memory_dc || !drawCommand) {
        DebugLogDraw("ProcessDrawCommandForWindow: Invalid parameters");
        return;
    }
    
    HDC memoryDeviceContext = backend->memory_dc;
    
    // Process the draw command for the specific window
    switch (drawCommand->type) {
        case NK_COMMAND_NOP:
            break;
        case NK_COMMAND_SCISSOR: {
            const struct nk_command_scissor* scissorCmd = (const struct nk_command_scissor*)drawCommand;
            HRGN clipRegion = CreateRectRgn((int)scissorCmd->x, (int)scissorCmd->y, (int)(scissorCmd->x + scissorCmd->w), (int)(scissorCmd->y + scissorCmd->h));
            SelectClipRgn(memoryDeviceContext, clipRegion);
            DeleteObject(clipRegion);
            DebugLogDraw("ProcessDrawCommandForWindow: Applied scissor region");
        } break;
        case NK_COMMAND_RECT_FILLED: {
            const struct nk_command_rect_filled* rectCmd = (const struct nk_command_rect_filled*)drawCommand;
            RECT fillRect = {(int)rectCmd->x, (int)rectCmd->y, (int)(rectCmd->x + rectCmd->w), (int)(rectCmd->y + rectCmd->h)};
            HBRUSH fillBrush = CreateSolidBrush(RGB(rectCmd->color.r, rectCmd->color.g, rectCmd->color.b));
            FillRect(memoryDeviceContext, &fillRect, fillBrush);
            DeleteObject(fillBrush);
            DebugLogDraw("ProcessDrawCommandForWindow: Drew filled rectangle");
        } break;
        case NK_COMMAND_TEXT: {
            const struct nk_command_text* textCmd = (const struct nk_command_text*)drawCommand;
            SetTextColor(memoryDeviceContext, RGB(textCmd->foreground.r, textCmd->foreground.g, textCmd->foreground.b));
            SetBkMode(memoryDeviceContext, TRANSPARENT);
            
            // Give text more vertical space to prevent clipping - add 50% extra height
            int extraHeight = (int)(textCmd->h * 0.5f);
            RECT textRect = {(int)textCmd->x, (int)textCmd->y - extraHeight/2, (int)(textCmd->x + textCmd->w), (int)(textCmd->y + textCmd->h + extraHeight/2)};
            
            // Convert to wide char for DrawText
            wchar_t* wideText = (wchar_t*)malloc((textCmd->length + 1) * sizeof(wchar_t));
            if (wideText) {
                MultiByteToWideChar(CP_UTF8, 0, (const char*)textCmd->string, (int)textCmd->length, wideText, (int)textCmd->length);
                wideText[textCmd->length] = 0;
                
                // Use DT_VCENTER to properly center text vertically in the expanded rectangle
                DrawTextW(memoryDeviceContext, wideText, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                free(wideText);
                DebugLogDraw("ProcessDrawCommandForWindow: Drew text with expanded bounds");
            }
        } break;
        case NK_COMMAND_RECT: {
            const struct nk_command_rect* outlineCmd = (const struct nk_command_rect*)drawCommand;
            HPEN outlinePen = CreatePen(PS_SOLID, (int)outlineCmd->line_thickness, RGB(outlineCmd->color.r, outlineCmd->color.g, outlineCmd->color.b));
            HPEN previousPen = (HPEN)SelectObject(memoryDeviceContext, outlinePen);
            HBRUSH previousBrush = (HBRUSH)SelectObject(memoryDeviceContext, GetStockObject(NULL_BRUSH));
            
            Rectangle(memoryDeviceContext, (int)outlineCmd->x, (int)outlineCmd->y, (int)(outlineCmd->x + outlineCmd->w), (int)(outlineCmd->y + outlineCmd->h));
            
            SelectObject(memoryDeviceContext, previousBrush);
            SelectObject(memoryDeviceContext, previousPen);
            DeleteObject(outlinePen);
            DebugLogDraw("ProcessDrawCommandForWindow: Drew rectangle outline");
        } break;
        default:
            DebugLogDraw("ProcessDrawCommandForWindow: Unhandled command type %d", drawCommand->type);
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

void NKGdiBackend::Render(struct nk_color backgroundColor, struct nk_context* nuklearContext) {
    const struct nk_command* renderCommand;
    
    if (!memory_dc || !bits || !nuklearContext) return;
    
    // Clear background
    RECT backgroundRect = {0, 0, width, height};
    HBRUSH backgroundBrush = CreateSolidBrush(RGB(backgroundColor.r, backgroundColor.g, backgroundColor.b));
    FillRect(memory_dc, &backgroundRect, backgroundBrush);
    DeleteObject(backgroundBrush);
    
    // Render nuklear commands
    nk_foreach(renderCommand, nuklearContext) {
        switch (renderCommand->type) {
        case NK_COMMAND_NOP: break;
        case NK_COMMAND_SCISSOR: {
            const struct nk_command_scissor* scissorCmd = (const struct nk_command_scissor*)renderCommand;
            HRGN clipRegion = CreateRectRgn((int)scissorCmd->x, (int)scissorCmd->y, (int)(scissorCmd->x + scissorCmd->w), (int)(scissorCmd->y + scissorCmd->h));
            SelectClipRgn(memory_dc, clipRegion);
            DeleteObject(clipRegion);
        } break;
        case NK_COMMAND_RECT_FILLED: {
            const struct nk_command_rect_filled* rectCmd = (const struct nk_command_rect_filled*)renderCommand;
            RECT fillRect = {(int)rectCmd->x, (int)rectCmd->y, (int)(rectCmd->x + rectCmd->w), (int)(rectCmd->y + rectCmd->h)};
            HBRUSH fillBrush = CreateSolidBrush(RGB(rectCmd->color.r, rectCmd->color.g, rectCmd->color.b));
            FillRect(memory_dc, &fillRect, fillBrush);
            DeleteObject(fillBrush);
        } break;
        case NK_COMMAND_TEXT: {
            const struct nk_command_text* textCmd = (const struct nk_command_text*)renderCommand;
            SetTextColor(memory_dc, RGB(textCmd->foreground.r, textCmd->foreground.g, textCmd->foreground.b));
            SetBkMode(memory_dc, TRANSPARENT);
            
            // Give text more vertical space to prevent clipping - add 50% extra height
            int extraHeight = (int)(textCmd->h * 0.5f);
            RECT textRect = {(int)textCmd->x, (int)textCmd->y - extraHeight/2, (int)(textCmd->x + textCmd->w), (int)(textCmd->y + textCmd->h + extraHeight/2)};
            
            // Convert to wide char for DrawText
            wchar_t* wideText = (wchar_t*)malloc((textCmd->length + 1) * sizeof(wchar_t));
            MultiByteToWideChar(CP_UTF8, 0, (const char*)textCmd->string, (int)textCmd->length, wideText, (int)textCmd->length);
            wideText[textCmd->length] = 0;
            
            // Use DT_VCENTER to properly center text vertically in the expanded rectangle
            DrawTextW(memory_dc, wideText, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            free(wideText);
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

int NKGdiBackend::HandleEvent(HWND eventSourceWindow, UINT msg, WPARAM wparam, LPARAM lparam) {
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