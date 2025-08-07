#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Nuklear implementation
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GDI_IMPLEMENTATION
#include "nuklear.h"

// Simple Win32 GDI Nuklear backend
struct nk_gdi {
    struct nk_context ctx;
    struct nk_buffer cmds;
    HDC window_dc;
    HDC memory_dc;
    HBITMAP bitmap;
    void *bits;
    int width, height;
    struct nk_font *font;
};

// Application window state
struct app_window_state {
    HWND hwnd;
    struct nk_gdi nk_gdi;
    int active;
    int width, height;
    char title[64];
    
    // Window-specific data
    float slider_value;
    int checkbox_state;
    char text_buffer[256];
    struct nk_color bg_color;
};

// Global application state  
struct app_state {
    struct app_window_state main_window;
    struct app_window_state tool_window;
    int running;
    int show_tool_window;
    HINSTANCE hInstance;
} app;

// Window class names
#define MAIN_WINDOW_CLASS L"NuklearMainWindow"
#define TOOL_WINDOW_CLASS L"NuklearToolWindow"

// Forward declarations
static LRESULT CALLBACK main_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static LRESULT CALLBACK tool_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static void setup_nuklear_gdi(struct app_window_state* window);
static void handle_main_window_gui(struct app_window_state* window);
static void handle_tool_window_gui(struct app_window_state* window);
static void render_nuklear_gdi(struct app_window_state* window);
static void cleanup_window(struct app_window_state* window);
static int nk_gdi_handle_event(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, struct nk_context* ctx);
static void nk_gdi_resize(struct nk_gdi* gdi, int width, int height);
static void nk_gdi_render(struct nk_gdi* gdi, struct nk_color bg);

// Initialize window state
static void init_window_state(struct app_window_state* window, const char* title) {
    memset(window, 0, sizeof(*window));
    strncpy(window->title, title, sizeof(window->title) - 1);
    window->width = 500;
    window->height = 400;
    window->slider_value = 50.0f;
    window->checkbox_state = 1;
    strcpy(window->text_buffer, "Hello World");
    window->bg_color = nk_rgb(28, 48, 62);
}

// Handle input events for Nuklear
static int nk_gdi_handle_event(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, struct nk_context* ctx) {
    switch (msg) {
        case WM_KEYDOWN:
        case WM_KEYUP: {
            int down = (msg == WM_KEYDOWN);
            int ctrl = GetKeyState(VK_CONTROL) & 0x8000;
            
            switch (wparam) {
                case VK_SHIFT:
                case VK_LSHIFT:
                case VK_RSHIFT:
                    nk_input_key(ctx, NK_KEY_SHIFT, down);
                    return 1;
                case VK_DELETE:
                    nk_input_key(ctx, NK_KEY_DEL, down);
                    return 1;
                case VK_RETURN:
                    nk_input_key(ctx, NK_KEY_ENTER, down);
                    return 1;
                case VK_TAB:
                    nk_input_key(ctx, NK_KEY_TAB, down);
                    return 1;
                case VK_LEFT:
                    if (ctrl) nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
                    else nk_input_key(ctx, NK_KEY_LEFT, down);
                    return 1;
                case VK_RIGHT:
                    if (ctrl) nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
                    else nk_input_key(ctx, NK_KEY_RIGHT, down);
                    return 1;
                case VK_BACK:
                    nk_input_key(ctx, NK_KEY_BACKSPACE, down);
                    return 1;
                case VK_HOME:
                    nk_input_key(ctx, NK_KEY_TEXT_START, down);
                    return 1;
                case VK_END:
                    nk_input_key(ctx, NK_KEY_TEXT_END, down);
                    return 1;
                case 'A':
                    if (ctrl && down) {
                        nk_input_key(ctx, NK_KEY_TEXT_SELECT_ALL, 1);
                        return 1;
                    }
                    break;
                case 'C':
                    if (ctrl && down) {
                        nk_input_key(ctx, NK_KEY_COPY, 1);
                        return 1;
                    }
                    break;
                case 'V':
                    if (ctrl && down) {
                        nk_input_key(ctx, NK_KEY_PASTE, 1);
                        return 1;
                    }
                    break;
                case 'X':
                    if (ctrl && down) {
                        nk_input_key(ctx, NK_KEY_CUT, 1);
                        return 1;
                    }
                    break;
                case 'Z':
                    if (ctrl && down) {
                        nk_input_key(ctx, NK_KEY_TEXT_UNDO, 1);
                        return 1;
                    }
                    break;
                case 'R':
                    if (ctrl && down) {
                        nk_input_key(ctx, NK_KEY_TEXT_REDO, 1);
                        return 1;
                    }
                    break;
            }
            return 0;
        }
        case WM_CHAR:
            if (wparam >= 32) {
                nk_input_unicode(ctx, (nk_rune)wparam);
                return 1;
            }
            return 0;
        case WM_LBUTTONDOWN:
            SetCapture(hwnd);
            nk_input_button(ctx, NK_BUTTON_LEFT, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), 1);
            return 1;
        case WM_LBUTTONUP:
            ReleaseCapture();
            nk_input_button(ctx, NK_BUTTON_LEFT, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), 0);
            return 1;
        case WM_RBUTTONDOWN:
            SetCapture(hwnd);
            nk_input_button(ctx, NK_BUTTON_RIGHT, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), 1);
            return 1;
        case WM_RBUTTONUP:
            ReleaseCapture();
            nk_input_button(ctx, NK_BUTTON_RIGHT, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), 0);
            return 1;
        case WM_MOUSEMOVE:
            nk_input_motion(ctx, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            return 1;
        case WM_MOUSEWHEEL:
            nk_input_scroll(ctx, nk_vec2(0, (float)(short)HIWORD(wparam) / WHEEL_DELTA));
            return 1;
    }
    return 0;
}

// Resize GDI bitmap
static void nk_gdi_resize(struct nk_gdi* gdi, int width, int height) {
    if (gdi->memory_dc) {
        if (gdi->bitmap) {
            SelectObject(gdi->memory_dc, NULL);
            DeleteObject(gdi->bitmap);
        }
        DeleteDC(gdi->memory_dc);
    }
    
    gdi->width = width;
    gdi->height = height;
    
    if (width <= 0 || height <= 0) return;
    
    gdi->memory_dc = CreateCompatibleDC(gdi->window_dc);
    
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    gdi->bitmap = CreateDIBSection(gdi->memory_dc, &bmi, DIB_RGB_COLORS, &gdi->bits, NULL, 0);
    SelectObject(gdi->memory_dc, gdi->bitmap);
}

// Simple GDI rendering for Nuklear
static void nk_gdi_render(struct nk_gdi* gdi, struct nk_color bg) {
    const struct nk_command* cmd;
    
    if (!gdi->memory_dc || !gdi->bits) return;
    
    // Clear background
    RECT rect = {0, 0, gdi->width, gdi->height};
    HBRUSH bg_brush = CreateSolidBrush(RGB(bg.r, bg.g, bg.b));
    FillRect(gdi->memory_dc, &rect, bg_brush);
    DeleteObject(bg_brush);
    
    // Render nuklear commands
    nk_foreach(cmd, &gdi->ctx) {
        switch (cmd->type) {
        case NK_COMMAND_NOP: break;
        case NK_COMMAND_SCISSOR: {
            const struct nk_command_scissor* s = (const struct nk_command_scissor*)cmd;
            // Set clipping region (simplified)
            HRGN region = CreateRectRgn((int)s->x, (int)s->y, (int)(s->x + s->w), (int)(s->y + s->h));
            SelectClipRgn(gdi->memory_dc, region);
            DeleteObject(region);
        } break;
        case NK_COMMAND_LINE: {
            const struct nk_command_line* l = (const struct nk_command_line*)cmd;
            HPEN pen = CreatePen(PS_SOLID, (int)l->line_thickness, RGB(l->color.r, l->color.g, l->color.b));
            HPEN old_pen = SelectObject(gdi->memory_dc, pen);
            MoveToEx(gdi->memory_dc, (int)l->begin.x, (int)l->begin.y, NULL);
            LineTo(gdi->memory_dc, (int)l->end.x, (int)l->end.y);
            SelectObject(gdi->memory_dc, old_pen);
            DeleteObject(pen);
        } break;
        case NK_COMMAND_RECT: {
            const struct nk_command_rect* r = (const struct nk_command_rect*)cmd;
            HBRUSH brush = NULL;
            HPEN pen = NULL;
            
            if (r->color.a > 0) {
                brush = CreateSolidBrush(RGB(r->color.r, r->color.g, r->color.b));
            } else {
                brush = (HBRUSH)GetStockObject(NULL_BRUSH);
            }
            
            if (r->line_thickness > 0) {
                pen = CreatePen(PS_SOLID, (int)r->line_thickness, RGB(r->color.r, r->color.g, r->color.b));
            } else {
                pen = (HPEN)GetStockObject(NULL_PEN);
            }
            
            HPEN old_pen = SelectObject(gdi->memory_dc, pen);
            HBRUSH old_brush = SelectObject(gdi->memory_dc, brush);
            
            Rectangle(gdi->memory_dc, (int)r->x, (int)r->y, (int)(r->x + r->w), (int)(r->y + r->h));
            
            SelectObject(gdi->memory_dc, old_pen);
            SelectObject(gdi->memory_dc, old_brush);
            if (r->color.a > 0) DeleteObject(brush);
            if (r->line_thickness > 0) DeleteObject(pen);
        } break;
        case NK_COMMAND_RECT_FILLED: {
            const struct nk_command_rect_filled* r = (const struct nk_command_rect_filled*)cmd;
            RECT rect = {(int)r->x, (int)r->y, (int)(r->x + r->w), (int)(r->y + r->h)};
            HBRUSH brush = CreateSolidBrush(RGB(r->color.r, r->color.g, r->color.b));
            FillRect(gdi->memory_dc, &rect, brush);
            DeleteObject(brush);
        } break;
        case NK_COMMAND_TEXT: {
            const struct nk_command_text* t = (const struct nk_command_text*)cmd;
            SetTextColor(gdi->memory_dc, RGB(t->foreground.r, t->foreground.g, t->foreground.b));
            SetBkMode(gdi->memory_dc, TRANSPARENT);
            
            RECT rect = {(int)t->x, (int)t->y, (int)(t->x + t->w), (int)(t->y + t->h)};
            
            // Convert to wide char for DrawText
            wchar_t* wtext = malloc((t->length + 1) * sizeof(wchar_t));
            MultiByteToWideChar(CP_UTF8, 0, (const char*)t->string, (int)t->length, wtext, (int)t->length);
            wtext[t->length] = 0;
            
            DrawTextW(gdi->memory_dc, wtext, -1, &rect, DT_LEFT | DT_TOP | DT_SINGLELINE);
            free(wtext);
        } break;
        default: break;
        }
    }
    
    // Reset clipping
    SelectClipRgn(gdi->memory_dc, NULL);
}

// Setup Nuklear GDI context
static void setup_nuklear_gdi(struct app_window_state* window) {
    struct nk_gdi* gdi = &window->nk_gdi;
    
    gdi->window_dc = GetDC(window->hwnd);
    
    // Initialize nuklear context with default font
    nk_init_default(&gdi->ctx, 0);
    nk_buffer_init_default(&gdi->cmds);
    
    // Create initial bitmap
    nk_gdi_resize(gdi, window->width, window->height);
    
    window->active = 1;
}

// Main window procedure
static LRESULT CALLBACK main_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    struct app_window_state* window = &app.main_window;
    
    // Handle Nuklear input first
    if (window->active && nk_gdi_handle_event(hwnd, msg, wparam, lparam, &window->nk_gdi.ctx)) {
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    
    switch (msg) {
        case WM_CREATE:
            window->hwnd = hwnd;
            setup_nuklear_gdi(window);
            return 0;
        case WM_SIZE:
            if (window->active) {
                window->width = LOWORD(lparam);
                window->height = HIWORD(lparam);
                nk_gdi_resize(&window->nk_gdi, window->width, window->height);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (window->active && window->nk_gdi.memory_dc) {
                BitBlt(hdc, 0, 0, window->width, window->height,
                       window->nk_gdi.memory_dc, 0, 0, SRCCOPY);
            }
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_CLOSE:
            app.running = 0;
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

// Tool window procedure
static LRESULT CALLBACK tool_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    struct app_window_state* window = &app.tool_window;
    
    // Handle Nuklear input first
    if (window->active && nk_gdi_handle_event(hwnd, msg, wparam, lparam, &window->nk_gdi.ctx)) {
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    
    switch (msg) {
        case WM_CREATE:
            window->hwnd = hwnd;
            setup_nuklear_gdi(window);
            return 0;
        case WM_SIZE:
            if (window->active) {
                window->width = LOWORD(lparam);
                window->height = HIWORD(lparam);
                nk_gdi_resize(&window->nk_gdi, window->width, window->height);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (window->active && window->nk_gdi.memory_dc) {
                BitBlt(hdc, 0, 0, window->width, window->height,
                       window->nk_gdi.memory_dc, 0, 0, SRCCOPY);
            }
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_CLOSE:
            app.show_tool_window = 0;
            return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

// Handle main window GUI
static void handle_main_window_gui(struct app_window_state* window) {
    struct nk_context* ctx = &window->nk_gdi.ctx;
    
    if (nk_begin(ctx, "Main Control Panel", nk_rect(20, 20, 300, 280),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)) {
        
        nk_layout_row_static(ctx, 25, 100, 2);
        nk_label(ctx, "Main Window", NK_TEXT_LEFT);
        
        if (nk_button_label(ctx, "Open Tool")) {
            app.show_tool_window = 1;
        }
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Background Color:", NK_TEXT_LEFT);
        
        nk_layout_row_static(ctx, 25, 50, 3);
        window->bg_color.r = (nk_byte)nk_propertyi(ctx, "#R", 0, window->bg_color.r, 255, 1, 1);
        window->bg_color.g = (nk_byte)nk_propertyi(ctx, "#G", 0, window->bg_color.g, 255, 1, 1);
        window->bg_color.b = (nk_byte)nk_propertyi(ctx, "#B", 0, window->bg_color.b, 255, 1, 1);
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Main Slider:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_slider_float(ctx, 0, &window->slider_value, 100, 1.0f);
        
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_checkbox_label(ctx, "Main Checkbox", &window->checkbox_state);
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Text Input:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, window->text_buffer, 
                                      sizeof(window->text_buffer), nk_filter_default);
        
        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(ctx, "Close Application")) {
            app.running = 0;
        }
    }
    nk_end(ctx);
}

// Handle tool window GUI
static void handle_tool_window_gui(struct app_window_state* window) {
    struct nk_context* ctx = &window->nk_gdi.ctx;
    
    if (nk_begin(ctx, "Tool Window", nk_rect(20, 20, 250, 200),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | 
                 NK_WINDOW_TITLE | NK_WINDOW_CLOSABLE)) {
        
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "Tool Panel", NK_TEXT_LEFT);
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Tool Slider:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_slider_float(ctx, 0, &window->slider_value, 200, 2.0f);
        
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_checkbox_label(ctx, "Tool Option", &window->checkbox_state);
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Tool Input:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, window->text_buffer, 
                                      sizeof(window->text_buffer), nk_filter_default);
        
        nk_layout_row_dynamic(ctx, 25, 2);
        if (nk_button_label(ctx, "Process")) {
            char msg[256];
            sprintf(msg, "Processing: %.2f", window->slider_value);
            MessageBoxA(window->hwnd, msg, "Tool", MB_OK);
        }
        
        if (nk_button_label(ctx, "Reset")) {
            window->slider_value = 100.0f;
            window->checkbox_state = 0;
            strcpy(window->text_buffer, "Reset");
        }
        
    } else {
        // Window was closed via X button
        app.show_tool_window = 0;
    }
    nk_end(ctx);
}

// Render window with GDI
static void render_nuklear_gdi(struct app_window_state* window) {
    if (!window->active) return;
    
    struct nk_gdi* gdi = &window->nk_gdi;
    
    // Render nuklear to memory DC
    nk_gdi_render(gdi, window->bg_color);
    
    // Trigger paint message to copy to window
    InvalidateRect(window->hwnd, NULL, FALSE);
}

// Cleanup window
static void cleanup_window(struct app_window_state* window) {
    if (!window->active) return;
    
    struct nk_gdi* gdi = &window->nk_gdi;
    
    if (gdi->bitmap) {
        DeleteObject(gdi->bitmap);
    }
    if (gdi->memory_dc) {
        DeleteDC(gdi->memory_dc);
    }
    if (gdi->window_dc) {
        ReleaseDC(window->hwnd, gdi->window_dc);
    }
    
    nk_buffer_free(&gdi->cmds);
    nk_free(&gdi->ctx);
    
    if (window->hwnd) {
        DestroyWindow(window->hwnd);
    }
    
    window->active = 0;
}

// Create window
static int create_window(struct app_window_state* window, WNDPROC proc, LPCWSTR class_name, int x, int y) {
    window->hwnd = CreateWindowExW(
        0,
        class_name,
        L"Nuklear Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        x, y, window->width, window->height,
        NULL, NULL, app.hInstance, NULL
    );
    
    return window->hwnd != NULL;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    MSG msg;
    
    app.hInstance = hInstance;
    app.running = 1;
    app.show_tool_window = 0;
    
    // Initialize window states
    init_window_state(&app.main_window, "Main Window");
    init_window_state(&app.tool_window, "Tool Window");
    
    // Register window classes
    WNDCLASSEXW main_wc = {0};
    main_wc.cbSize = sizeof(WNDCLASSEXW);
    main_wc.style = CS_HREDRAW | CS_VREDRAW;
    main_wc.lpfnWndProc = main_window_proc;
    main_wc.hInstance = hInstance;
    main_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    main_wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    main_wc.lpszClassName = MAIN_WINDOW_CLASS;
    RegisterClassExW(&main_wc);
    
    WNDCLASSEXW tool_wc = main_wc;
    tool_wc.lpfnWndProc = tool_window_proc;
    tool_wc.lpszClassName = TOOL_WINDOW_CLASS;
    RegisterClassExW(&tool_wc);
    
    // Create main window
    if (!create_window(&app.main_window, main_window_proc, MAIN_WINDOW_CLASS, 100, 100)) {
        return -1;
    }
    
    // Main message loop
    while (app.running) {
        // Process Windows messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        if (!app.running) break;
        
        // Handle main window GUI
        if (app.main_window.active) {
            nk_input_begin(&app.main_window.nk_gdi.ctx);
            nk_input_end(&app.main_window.nk_gdi.ctx);
            handle_main_window_gui(&app.main_window);
            render_nuklear_gdi(&app.main_window);
            nk_clear(&app.main_window.nk_gdi.ctx);
        }
        
        // Handle tool window creation/destruction
        if (app.show_tool_window && !app.tool_window.active) {
            if (create_window(&app.tool_window, tool_window_proc, TOOL_WINDOW_CLASS, 650, 150)) {
                // Window setup handled in WM_CREATE
            } else {
                app.show_tool_window = 0;
            }
        } else if (!app.show_tool_window && app.tool_window.active) {
            cleanup_window(&app.tool_window);
        }
        
        // Handle tool window GUI if active
        if (app.tool_window.active) {
            nk_input_begin(&app.tool_window.nk_gdi.ctx);
            nk_input_end(&app.tool_window.nk_gdi.ctx);
            handle_tool_window_gui(&app.tool_window);
            render_nuklear_gdi(&app.tool_window);
            nk_clear(&app.tool_window.nk_gdi.ctx);
        }
        
        Sleep(16); // ~60 FPS
    }
    
    // Cleanup
    cleanup_window(&app.main_window);
    cleanup_window(&app.tool_window);
    
    return 0;
}