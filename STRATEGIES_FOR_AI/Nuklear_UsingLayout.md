# Nuklear Layouts Guide

Nuklear is an immediate mode GUI library that provides flexible layout systems for organizing UI elements. This guide covers the essential layout types and how to use them effectively.

## Overview

Nuklear layouts work on a container-based system where you define layout regions and then place widgets within them. All layout functions must be called between `nk_begin()` and `nk_end()` calls.

## Basic Layout Types

### 1. Dynamic Layouts

Dynamic layouts automatically distribute available space among widgets.

#### Row Layout (Dynamic)
```c
// Create a dynamic row with 2 columns
nk_layout_row_dynamic(ctx, 30, 2);
nk_button_label(ctx, "Button 1");
nk_button_label(ctx, "Button 2");

// Single widget taking full width
nk_layout_row_dynamic(ctx, 25, 1);
nk_label(ctx, "Full width label", NK_TEXT_LEFT);
```

#### Column Layout (Dynamic)
```c
// Begin dynamic columns
nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 3);
{
    nk_layout_row_push(ctx, 0.5f);  // 50% width
    nk_button_label(ctx, "Wide");
    
    nk_layout_row_push(ctx, 0.25f); // 25% width
    nk_button_label(ctx, "Medium");
    
    nk_layout_row_push(ctx, 0.25f); // 25% width
    nk_button_label(ctx, "Small");
}
nk_layout_row_end(ctx);
```

### 2. Static Layouts

Static layouts use fixed pixel widths.

#### Row Layout (Static)
```c
// Create a static row with fixed widths
nk_layout_row_static(ctx, 30, 80, 3);
nk_button_label(ctx, "Fixed 1");
nk_button_label(ctx, "Fixed 2");
nk_button_label(ctx, "Fixed 3");
```

#### Mixed Static Layout
```c
nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
{
    nk_layout_row_push(ctx, 100);  // 100 pixels
    nk_button_label(ctx, "100px");
    
    nk_layout_row_push(ctx, 200);  // 200 pixels
    nk_button_label(ctx, "200px");
}
nk_layout_row_end(ctx);
```

### 3. Template Layouts

Templates provide fine-grained control over widget positioning.

```c
// Define column widths as percentages or fixed values
float ratio[] = {60, -150, -1}; // 60px, 150px from right, fill remaining
nk_layout_row(ctx, NK_DYNAMIC, 30, 3, ratio);
nk_label(ctx, "Fixed 60px", NK_TEXT_LEFT);
nk_label(ctx, "150px from right", NK_TEXT_CENTERED);
nk_label(ctx, "Fill remaining", NK_TEXT_RIGHT);
```

## Advanced Layout Features

### Groups

Groups create sub-containers with their own layout systems.

```c
// Create a titled group
if (nk_group_begin(ctx, "My Group", NK_WINDOW_TITLE | NK_WINDOW_BORDER)) {
    nk_layout_row_dynamic(ctx, 25, 1);
    nk_label(ctx, "Inside group", NK_TEXT_LEFT);
    nk_button_label(ctx, "Group Button");
    nk_group_end(ctx);
}

// Scrollable group
if (nk_group_begin(ctx, "Scrollable", NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_SCROLL)) {
    nk_layout_row_dynamic(ctx, 20, 1);
    for (int i = 0; i < 20; i++) {
        nk_labelf(ctx, NK_TEXT_LEFT, "Item %d", i);
    }
    nk_group_end(ctx);
}
```

### Tree Nodes

Tree layouts for hierarchical data display.

```c
if (nk_tree_push(ctx, NK_TREE_TAB, "Root Node", NK_MINIMIZED)) {
    nk_layout_row_dynamic(ctx, 20, 1);
    nk_label(ctx, "Child item 1", NK_TEXT_LEFT);
    
    if (nk_tree_push(ctx, NK_TREE_NODE, "Sub Node", NK_MINIMIZED)) {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Nested item", NK_TEXT_LEFT);
        nk_tree_pop(ctx);
    }
    
    nk_label(ctx, "Child item 2", NK_TEXT_LEFT);
    nk_tree_pop(ctx);
}
```

### Spacing and Alignment

```c
// Add vertical spacing
nk_layout_row_dynamic(ctx, 10, 1);
nk_spacing(ctx, 1);

// Custom spacing
nk_layout_row_dynamic(ctx, 30, 3);
nk_button_label(ctx, "Button");
nk_spacing(ctx, 1); // Empty column
nk_button_label(ctx, "Spaced Button");
```

## Layout Patterns

### Sidebar Layout
```c
nk_layout_row_begin(ctx, NK_STATIC, window_height - 60, 2);
{
    // Sidebar
    nk_layout_row_push(ctx, 200);
    if (nk_group_begin(ctx, "Sidebar", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_button_label(ctx, "Menu Item 1");
        nk_button_label(ctx, "Menu Item 2");
        nk_button_label(ctx, "Menu Item 3");
        nk_group_end(ctx);
    }
    
    // Main content
    nk_layout_row_push(ctx, window_width - 220);
    if (nk_group_begin(ctx, "Content", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "Main content area", NK_TEXT_LEFT);
        nk_group_end(ctx);
    }
}
nk_layout_row_end(ctx);
```

### Form Layout
```c
// Label-input pairs
nk_layout_row_begin(ctx, NK_STATIC, 25, 2);
{
    nk_layout_row_push(ctx, 80);
    nk_label(ctx, "Name:", NK_TEXT_LEFT);
    
    nk_layout_row_push(ctx, 200);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, name_buffer, 
                                   sizeof(name_buffer), nk_filter_default);
}
nk_layout_row_end(ctx);

nk_layout_row_begin(ctx, NK_STATIC, 25, 2);
{
    nk_layout_row_push(ctx, 80);
    nk_label(ctx, "Email:", NK_TEXT_LEFT);
    
    nk_layout_row_push(ctx, 200);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, email_buffer, 
                                   sizeof(email_buffer), nk_filter_default);
}
nk_layout_row_end(ctx);
```

## Best Practices

### 1. Layout Consistency
- Use consistent spacing and alignment throughout your UI
- Define standard heights for similar elements (e.g., all buttons 30px)
- Group related elements using consistent margins

### 2. Responsive Design
```c
// Adapt to window size
float button_width = (window_width - 40) / 3.0f; // 3 buttons with margins
nk_layout_row_static(ctx, 30, button_width, 3);
```

### 3. Memory Management
- Layout calls don't allocate memory, but be mindful of nested groups
- Always match `nk_group_begin()` with `nk_group_end()`
- Same for `nk_tree_push()` and `nk_tree_pop()`

### 4. Performance Tips
- Use static layouts when widget sizes are known and constant
- Minimize deep nesting of groups
- Cache layout calculations when possible

## Common Pitfalls

### 1. Mismatched Begin/End Calls
```c
// WRONG - missing nk_group_end()
if (nk_group_begin(ctx, "Group", NK_WINDOW_BORDER)) {
    nk_button_label(ctx, "Button");
    // Missing nk_group_end()!
}

// CORRECT
if (nk_group_begin(ctx, "Group", NK_WINDOW_BORDER)) {
    nk_button_label(ctx, "Button");
    nk_group_end(ctx);
}
```

### 2. Layout Outside Window Context
```c
// WRONG - layout called outside window
nk_layout_row_dynamic(ctx, 30, 2);

if (nk_begin(ctx, "Window", rect, flags)) {
    nk_button_label(ctx, "Button");
}
nk_end(ctx);

// CORRECT
if (nk_begin(ctx, "Window", rect, flags)) {
    nk_layout_row_dynamic(ctx, 30, 2);
    nk_button_label(ctx, "Button");
}
nk_end(ctx);
```

### 3. Incorrect Column Count
```c
// WRONG - 3 widgets but only 2 columns defined
nk_layout_row_dynamic(ctx, 30, 2);
nk_button_label(ctx, "Button 1");
nk_button_label(ctx, "Button 2");
nk_button_label(ctx, "Button 3"); // This will appear on next row

// CORRECT
nk_layout_row_dynamic(ctx, 30, 3);
nk_button_label(ctx, "Button 1");
nk_button_label(ctx, "Button 2");
nk_button_label(ctx, "Button 3");
```

## Dynamic Sizing Solutions

### 1. Font-Based Sizing

Calculate heights based on font metrics for scalable layouts:

```c
// Get font height
float font_height = ctx->style.font->height;

// Use font-based measurements
float row_height = font_height + 10;  // Font + padding
float button_height = font_height + 16; // Font + button padding
float input_height = font_height + 12;  // Font + input padding

nk_layout_row_dynamic(ctx, row_height, 2);
nk_button_label(ctx, "Auto-sized Button");
nk_button_label(ctx, "Scales with Font");
```

### 2. Style-Based Sizing

Use Nuklear's style system for consistent measurements:

```c
// Access style properties
struct nk_style *style = &ctx->style;
float padding = style->window.padding.y;
float spacing = style->window.spacing.y;
float button_padding = style->button.padding.y * 2;

// Calculate dynamic sizes
float item_height = style->font->height + button_padding;
float spaced_height = item_height + spacing;

nk_layout_row_dynamic(ctx, item_height, 1);
nk_button_label(ctx, "Style-aware Button");
```

### 3. Configuration-Based Approach

Create a theming system with predefined sizes:

```c
typedef struct {
    float font_scale;
    float base_height;
    float button_height;
    float input_height;
    float spacing;
    float padding;
} ui_theme_t;

// Initialize theme
ui_theme_t theme = {
    .font_scale = 1.0f,
    .base_height = 0,  // Will be calculated
    .spacing = 8.0f,
    .padding = 4.0f
};

// Calculate base measurements
void ui_theme_calculate(ui_theme_t *theme, struct nk_context *ctx) {
    float font_height = ctx->style.font->height * theme->font_scale;
    theme->base_height = font_height;
    theme->button_height = font_height + theme->padding * 4;
    theme->input_height = font_height + theme->padding * 3;
}

// Use in layouts
nk_layout_row_dynamic(ctx, theme.button_height, 2);
nk_button_label(ctx, "Themed Button");
nk_button_label(ctx, "Consistent Size");
```

### 4. Automatic Content Sizing

Let Nuklear calculate heights automatically when possible:

```c
// Use 0 for automatic height calculation in some contexts
nk_layout_row_template_begin(ctx, 0); // Auto height
nk_layout_row_template_push_dynamic(ctx);
nk_layout_row_template_push_static(ctx, 100);
nk_layout_row_template_end(ctx);

// Content determines the height
nk_label_wrap(ctx, "This text can be multiple lines and the row height will adjust automatically to fit the content.");
nk_button_label(ctx, "Fixed Width");
```

### 5. Responsive Sizing Functions

Create helper functions for common sizing patterns:

```c
// Helper functions
float ui_row_height(struct nk_context *ctx, float multiplier) {
    return ctx->style.font->height * multiplier + ctx->style.button.padding.y * 2;
}

float ui_text_height(struct nk_context *ctx) {
    return ctx->style.font->height + 4;
}

float ui_button_height(struct nk_context *ctx) {
    return ctx->style.font->height + ctx->style.button.padding.y * 2;
}

float ui_input_height(struct nk_context *ctx) {
    return ctx->style.font->height + ctx->style.edit.padding.y * 2;
}

// Usage
nk_layout_row_dynamic(ctx, ui_button_height(ctx), 3);
nk_button_label(ctx, "Auto Button");
nk_button_label(ctx, "Auto Button");
nk_button_label(ctx, "Auto Button");

nk_layout_row_dynamic(ctx, ui_text_height(ctx), 1);
nk_label(ctx, "Auto-sized text", NK_TEXT_LEFT);
```

### 6. HiDPI and Scaling Considerations

HiDPI displays require special handling because Nuklear works in logical pixels while the display uses physical pixels.

#### Understanding the HiDPI Problem
```c
// On a 2x HiDPI display:
// - Logical pixels: what Nuklear sees
// - Physical pixels: actual screen pixels (2x more)
// - Without scaling: UI appears tiny
// - With scaling: UI scales but may be blurry if not handled properly
```

#### Win32 DPI Detection and Setup

**DPI Awareness Declaration:**
```c
#include <windows.h>
#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")

// In your application manifest or startup code
void set_dpi_awareness(void) {
    // Method 1: Use manifest (recommended)
    // <dpiAware>true</dpiAware> in manifest
    
    // Method 2: Programmatic (fallback)
    SetProcessDPIAware();
    
    // Method 3: Per-monitor V2 (Windows 10 1703+)
    // SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
}
```

**DPI Detection Functions:**
```c
float get_window_dpi_scale(HWND hwnd) {
    // Windows 10 1607+ (recommended)
    UINT dpi = GetDpiForWindow(hwnd);
    return (float)dpi / 96.0f;
}

float get_system_dpi_scale(void) {
    HDC hdc = GetDC(NULL);
    int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);
    return (float)dpi / 96.0f;
}

// Monitor-specific DPI (for multi-monitor setups)
float get_monitor_dpi_scale(HWND hwnd) {
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    UINT dpi_x, dpi_y;
    
    if (GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y) == S_OK) {
        return (float)dpi_x / 96.0f;
    }
    
    return get_system_dpi_scale(); // Fallback
}

// Handle DPI change messages
LRESULT handle_dpi_change(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    UINT new_dpi = HIWORD(wParam);
    RECT* suggested_rect = (RECT*)lParam;
    
    // Update your UI scaling here
    float new_scale = (float)new_dpi / 96.0f;
    
    // Resize window to suggested size
    SetWindowPos(hwnd, NULL, 
                 suggested_rect->left, suggested_rect->top,
                 suggested_rect->right - suggested_rect->left,
                 suggested_rect->bottom - suggested_rect->top,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    
    return 0;
}
```

#### Comprehensive HiDPI UI System

```c
typedef struct {
    float content_scale;      // From platform (1.0, 1.5, 2.0, etc.)
    float user_scale;         // User preference multiplier
    float font_scale;         // Font-specific scaling
    struct nk_context *ctx;
    
    // Cached calculated values
    float effective_scale;
    float base_font_height;
    struct nk_user_font *scaled_font;
} ui_hidpi_context_t;

void ui_hidpi_init(ui_hidpi_context_t *ui, struct nk_context *ctx, float content_scale) {
    ui->ctx = ctx;
    ui->content_scale = content_scale;
    ui->user_scale = 1.0f;
    ui->font_scale = 1.0f;
    ui_hidpi_update_scale(ui);
}

void ui_hidpi_update_scale(ui_hidpi_context_t *ui) {
    ui->effective_scale = ui->content_scale * ui->user_scale * ui->font_scale;
    ui->base_font_height = ui->ctx->style.font->height;
    
    // You might need to reload fonts at different sizes here
    // ui_load_scaled_font(ui, ui->effective_scale);
}

float ui_hidpi_scale(ui_hidpi_context_t *ui, float size) {
    return size * ui->effective_scale;
}

// Specialized scaling for different UI elements
float ui_hidpi_button_height(ui_hidpi_context_t *ui) {
    float base_height = ui->base_font_height + ui->ctx->style.button.padding.y * 2;
    return ui_hidpi_scale(ui, base_height);
}

float ui_hidpi_input_height(ui_hidpi_context_t *ui) {
    float base_height = ui->base_font_height + ui->ctx->style.edit.padding.y * 2;
    return ui_hidpi_scale(ui, base_height);
}

float ui_hidpi_spacing(ui_hidpi_context_t *ui) {
    return ui_hidpi_scale(ui, ui->ctx->style.window.spacing.y);
}
```

#### Win32 GDI Font Handling for HiDPI

```c
typedef struct {
    struct nk_user_font nk_font;
    HFONT hfont;
    HDC hdc;
    int font_height;
    float scale;
} win32_font_t;

win32_font_t* create_scaled_font(HDC hdc, const char* font_name, 
                                int base_size, float dpi_scale) {
    win32_font_t* font = malloc(sizeof(win32_font_t));
    
    // Calculate scaled font size
    int scaled_size = (int)(base_size * dpi_scale);
    
    // Create GDI font
    font->hfont = CreateFontA(
        -scaled_size,              // Height (negative for character height)
        0,                         // Width (0 = default)
        0,                         // Escapement
        0,                         // Orientation  
        FW_NORMAL,                 // Weight
        FALSE,                     // Italic
        FALSE,                     // Underline
        FALSE,                     // StrikeOut
        DEFAULT_CHARSET,           // CharSet
        OUT_DEFAULT_PRECIS,        // OutPrecision
        CLIP_DEFAULT_PRECIS,       // ClipPrecision
        CLEARTYPE_QUALITY,         // Quality (use ClearType on modern systems)
        DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
        font_name                  // FaceName
    );
    
    if (!font->hfont) {
        free(font);
        return NULL;
    }
    
    // Set up device context
    font->hdc = CreateCompatibleDC(hdc);
    SelectObject(font->hdc, font->hfont);
    
    // Get actual font metrics
    TEXTMETRIC tm;
    GetTextMetrics(font->hdc, &tm);
    font->font_height = tm.tmHeight;
    font->scale = dpi_scale;
    
    // Set up Nuklear font callbacks
    font->nk_font.userdata.ptr = font;
    font->nk_font.height = (float)font->font_height;
    font->nk_font.width = win32_font_get_text_width;
    
    return font;
}

// Font width callback for Nuklear
float win32_font_get_text_width(nk_handle handle, float height, const char *text, int len) {
    win32_font_t* font = (win32_font_t*)handle.ptr;
    SIZE size;
    
    if (GetTextExtentPoint32A(font->hdc, text, len, &size)) {
        return (float)size.cx;
    }
    return 0;
}

void destroy_font(win32_font_t* font) {
    if (font) {
        DeleteObject(font->hfont);
        DeleteDC(font->hdc);
        free(font);
    }
}

// Apply font to Nuklear context
void apply_win32_font(struct nk_context* ctx, win32_font_t* font) {
    nk_style_set_font(ctx, &font->nk_font);
}
```

#### Complete Win32 HiDPI Example

```c
#include <windows.h>
#include <shellscalingapi.h>

// Global UI context
ui_hidpi_context_t g_ui;
win32_font_t* g_font = NULL;
struct nk_context* g_ctx = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Get initial DPI
            float dpi_scale = get_window_dpi_scale(hwnd);
            
            // Initialize UI context
            ui_hidpi_init(&g_ui, g_ctx, dpi_scale);
            
            // Create scaled font
            HDC hdc = GetDC(hwnd);
            g_font = create_scaled_font(hdc, "Segoe UI", 14, dpi_scale);
            ReleaseDC(hwnd, hdc);
            
            if (g_font) {
                apply_win32_font(g_ctx, g_font);
            }
            
            return 0;
        }
        
        case WM_DPICHANGED: {
            // Handle DPI changes (moving between monitors)
            UINT new_dpi = HIWORD(wParam);
            float new_scale = (float)new_dpi / 96.0f;
            
            printf("DPI changed: %.2f -> %.2f\n", g_ui.content_scale, new_scale);
            
            // Update UI scale
            g_ui.content_scale = new_scale;
            ui_hidpi_update_scale(&g_ui);
            
            // Recreate font at new scale
            if (g_font) {
                destroy_font(g_font);
                
                HDC hdc = GetDC(hwnd);
                g_font = create_scaled_font(hdc, "Segoe UI", 14, new_scale);
                ReleaseDC(hwnd, hdc);
                
                if (g_font) {
                    apply_win32_font(g_ctx, g_font);
                }
            }
            
            // Use suggested window size
            RECT* suggested_rect = (RECT*)lParam;
            SetWindowPos(hwnd, NULL,
                        suggested_rect->left, suggested_rect->top,
                        suggested_rect->right - suggested_rect->left,
                        suggested_rect->bottom - suggested_rect->top,
                        SWP_NOZORDER | SWP_NOACTIVATE);
            
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Your Nuklear rendering here
            render_nuklear_ui(hdc);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_SIZE: {
            // Handle window resize
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void render_nuklear_ui(HDC hdc) {
    // Clear background
    RECT client_rect;
    GetClientRect(WindowFromDC(hdc), &client_rect);
    
    // Begin Nuklear window
    struct nk_rect window_bounds = nk_rect(10, 10, 
                                          client_rect.right - 20, 
                                          client_rect.bottom - 20);
    
    if (nk_begin(g_ctx, "HiDPI Test Window", window_bounds,
                 NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)) {
        
        // Use HiDPI-aware sizing
        nk_layout_row_dynamic(g_ctx, ui_hidpi_button_height(&g_ui), 2);
        nk_button_label(g_ctx, "Scaled Button 1");
        nk_button_label(g_ctx, "Scaled Button 2");
        
        nk_layout_row_dynamic(g_ctx, ui_hidpi_input_height(&g_ui), 1);
        static char text_buffer[256] = "HiDPI Text Input";
        nk_edit_string_zero_terminated(g_ctx, NK_EDIT_FIELD, text_buffer, 256, nk_filter_default);
        
        nk_layout_row_dynamic(g_ctx, ui_hidpi_button_height(&g_ui) * 0.8f, 1);
        nk_labelf(g_ctx, NK_TEXT_LEFT, "Current DPI Scale: %.2fx", g_ui.content_scale);
        nk_labelf(g_ctx, NK_TEXT_LEFT, "Font Height: %.0fpx", g_ui.base_font_height);
        nk_labelf(g_ctx, NK_TEXT_LEFT, "Effective Scale: %.2fx", g_ui.effective_scale);
    }
    nk_end(g_ctx);
    
    // Render Nuklear to GDI (you'll need to implement this based on your renderer)
    nk_gdi_render(hdc, g_ctx);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    
    // Set DPI awareness
    set_dpi_awareness();
    
    // Register window class
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "NuklearHiDPIWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);
    
    // Get initial DPI for window sizing
    float initial_dpi_scale = get_system_dpi_scale();
    
    // Create window with DPI-scaled size
    int window_width = (int)(800 * initial_dpi_scale);
    int window_height = (int)(600 * initial_dpi_scale);
    
    HWND hwnd = CreateWindowEx(
        0,
        "NuklearHiDPIWindow",
        "Nuklear HiDPI Test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        window_width, window_height,
        NULL, NULL, hInstance, NULL
    );
    
    if (!hwnd) {
        return 1;
    }
    
    // Initialize Nuklear context (implementation specific)
    g_ctx = nk_gdi_init(hwnd); // Your GDI backend initialization
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Message loop
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    if (g_font) {
        destroy_font(g_font);
    }
    
    nk_gdi_shutdown(); // Your cleanup
    
    return (int)msg.wParam;
}
```

#### Key HiDPI Principles

1. **Separate Logical and Physical Pixels**: Nuklear works in logical pixels, but you need to scale for physical pixels
2. **Font Scaling**: Load fonts at the appropriate physical size, not just scale the logical size
3. **Dynamic Scaling**: Handle scale changes when users move windows between displays
4. **Consistent Scaling**: Apply the same scale factor to all UI elements
5. **Viewport Scaling**: Ensure your OpenGL/rendering viewport accounts for the scale

#### Common Pitfalls

```c
// WRONG: Scaling after font is loaded
nk_layout_row_dynamic(ctx, 30 * content_scale, 2); // Font won't match

// WRONG: Inconsistent scaling
nk_layout_row_dynamic(ctx, ui_hidpi_scale(&ui, 30), 2); // Button scaled
nk_spacing(ctx, 1); // Spacing not scaled - inconsistent

// CORRECT: Consistent scaling throughout
nk_layout_row_dynamic(ctx, ui_hidpi_button_height(&ui), 2);
nk_button_label(ctx, "Button");
nk_layout_row_dynamic(ctx, ui_hidpi_spacing(&ui), 1);
nk_spacing(ctx, 1);
```

The key is to handle HiDPI at the font loading stage and consistently apply scaling to all measurements, not just some of them.

### 7. Content-Aware Layouts

Calculate sizes based on actual content:

```c
// Measure text to determine optimal height
float ui_text_width(struct nk_context *ctx, const char *text) {
    return ctx->style.font->width(ctx->style.font->userdata, 
                                  ctx->style.font->height, text, strlen(text));
}

// Multi-line text height calculation
float ui_multiline_height(struct nk_context *ctx, const char *text, float max_width) {
    // Simple estimation - you might want a more sophisticated word-wrap calculation
    float text_width = ui_text_width(ctx, text);
    int lines = (int)ceilf(text_width / max_width);
    if (lines < 1) lines = 1;
    
    return lines * ctx->style.font->height + lines * 2; // 2px line spacing
}

// Usage
const char *long_text = "This is a very long text that might wrap to multiple lines...";
float content_height = ui_multiline_height(ctx, long_text, 300);

nk_layout_row_dynamic(ctx, content_height, 1);
nk_label_wrap(ctx, long_text);
```

### 8. Complete Theme System Example

```c
typedef enum {
    UI_SIZE_TINY,
    UI_SIZE_SMALL,
    UI_SIZE_NORMAL,
    UI_SIZE_LARGE,
    UI_SIZE_HUGE
} ui_size_t;

typedef struct {
    struct nk_context *ctx;
    float scale_factor;
    float base_font_size;
} ui_manager_t;

float ui_get_height(ui_manager_t *ui, ui_size_t size) {
    float base = ui->ctx->style.font->height * ui->scale_factor;
    
    switch (size) {
        case UI_SIZE_TINY:  return base * 0.7f + 4;
        case UI_SIZE_SMALL: return base * 0.85f + 6;
        case UI_SIZE_NORMAL: return base + 8;
        case UI_SIZE_LARGE: return base * 1.2f + 10;
        case UI_SIZE_HUGE:  return base * 1.5f + 12;
    }
    return base + 8;
}

// Clean API usage
nk_layout_row_dynamic(ctx, ui_get_height(&ui, UI_SIZE_NORMAL), 2);
nk_button_label(ctx, "Normal Button");
nk_button_label(ctx, "Normal Button");

nk_layout_row_dynamic(ctx, ui_get_height(&ui, UI_SIZE_LARGE), 1);
nk_button_label(ctx, "Large Button");
```

## Conclusion

Nuklear's layout system is powerful and flexible once you understand the basic patterns. By using font-based sizing, style system properties, and helper functions, you can create maintainable UIs that scale properly across different fonts, DPI settings, and themes. Start with simple dynamic and static layouts, then gradually incorporate more advanced features like groups and trees as needed. Remember to always maintain proper begin/end pairing and consider the user experience when designing your layouts.