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
    : m_initialized(false), m_focusedWindow(nullptr), m_activeWindow(nullptr) {
    // ✅ NO SHARED CONTEXT - each window has its own
}

NKWindowManager::~NKWindowManager() {
    Cleanup();
}

void NKWindowManager::Initialize() {
    if (m_initialized) return;
    
    // ✅ NO SHARED CONTEXT INITIALIZATION - each window initializes its own
    m_initialized = true;
}

// ✅ REMOVED: Shared context initialization moved to individual windows

// ✅ REMOVED: Theme application moved to individual windows

void NKWindowManager::Cleanup() {
    if (m_initialized) {
        // ✅ NO SHARED CONTEXT TO CLEANUP - each window cleans up its own
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

// ✅ REMOVED: Input processing moved to individual windows

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

// 🚨 CRITICAL BUG FIX: This function was using the wrong context!
// This function is now DEPRECATED - input processing moved to NKWindow::ProcessInputEventForWindow()
// to ensure input goes to the correct window's context
void NKWindowManager::ProcessInputEventForWindow(HWND hwndNuklearReceiver, const InputEvent& event) {
    DebugLog("ERROR: ProcessInputEventForWindow in NKWindowManager should not be called - use NKWindow::ProcessInputEventForWindow instead");
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