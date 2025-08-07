# Nuklear Anti-Patterns - What NOT to Do

## Overview
This document contains patterns that have been tested and FAILED. These are approaches that seem logical but cause critical issues in Nuklear applications. Avoid these patterns at all costs.

## ❌ ANTI-PATTERN 1: Multiple Contexts Per Window

### What NOT to Do
```cpp
// DON'T DO THIS - Causes sequence assertion failures
class NKWindow {
    struct nk_context m_ctx;  // ❌ Individual context per window
    NKGdiBackend m_backend;   // ❌ Individual backend per window
    
    void Initialize() {
        nk_init_default(&m_ctx, &font->handle);  // ❌ Multiple contexts
    }
};
```

### Why This Fails
- **Sequence assertion failures**: Nuklear's internal state machine expects single context usage
- **Complex synchronization**: Multiple contexts create race conditions
- **Inconsistent input handling**: Each context processes input independently
- **Memory overhead**: Unnecessary duplication of font atlases and resources
- **Violates Nuklear's design**: Framework is built around single context model

### Symptoms
- Assertion failures with messages like "sequence error"
- Inconsistent rendering between windows
- Input events not working properly
- Crashes during font operations

## ❌ ANTI-PATTERN 2: Tight Message Loop with Immediate UpdateAll()

### What NOT to Do
```cpp
// DON'T DO THIS - Causes blank windows
while (running) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    windowManager.UpdateAll();  // ❌ Called immediately after PeekMessage
}
```

### Why This Fails
- **WM_PAINT starvation**: Low-priority WM_PAINT messages never get processed
- **Blank windows**: Windows appear empty until user interaction forces message processing
- **Poor performance**: Tight loop consumes CPU unnecessarily
- **Message priority issues**: Input messages block painting messages

### Symptoms
- Windows appear blank initially
- Content only appears after clicking or moving mouse
- High CPU usage
- Flickering or delayed rendering

## ❌ ANTI-PATTERN 3: Using NK_INCLUDE_COMMAND_USERDATA for Window Targeting

### What NOT to Do
```cpp
// DON'T DO THIS - nk_draw_list_push_userdata is internal
#define NK_INCLUDE_COMMAND_USERDATA
#include "nuklear.h"

void UpdateAll() {
    for (NKWindow* window : m_windows) {
        // ❌ This function is internal and not part of public API
        nk_draw_list_push_userdata(&m_ctx.draw_list, nk_handle_ptr((void*)window->GetHWND()));
        window->Render();
    }
}
```

### Why This Fails
- **Internal API usage**: `nk_draw_list_push_userdata()` is not part of public API
- **Linker errors**: Function may not be exported or available
- **Fragile implementation**: Internal APIs can change without notice
- **Complex routing logic**: Trying to route commands after generation is error-prone

### Symptoms
- Linker errors: "unresolved external symbol _nk_draw_list_push_userdata"
- Compilation failures
- All windows showing same content (command routing fails)

## ❌ ANTI-PATTERN 4: Trying to Route Draw Commands After Generation

### What NOT to Do
```cpp
// DON'T DO THIS - Commands are already generated for all windows
void ProcessAllCommands() {
    const struct nk_command* cmd;
    nk_foreach(cmd, &m_ctx) {
        // ❌ Trying to figure out which window this command belongs to
        HWND targetWindow = GuessWindowFromCommand(cmd);  // Impossible to determine
        RouteCommandToWindow(targetWindow, cmd);
    }
}
```

### Why This Fails
- **No window information**: Draw commands don't contain target window data
- **Mixed command stream**: All windows' commands are interleaved
- **Impossible routing**: No reliable way to determine command ownership
- **Performance overhead**: Complex routing logic is slow and error-prone

### Symptoms
- All windows showing identical content
- Rendering artifacts
- Poor performance
- Inconsistent UI updates

## ❌ ANTI-PATTERN 5: Individual Font Atlases Per Window

### What NOT to Do
```cpp
// DON'T DO THIS - Wastes memory and causes inconsistencies
class NKWindow {
    struct nk_font_atlas m_atlas;  // ❌ Individual atlas per window
    struct nk_font* m_font;        // ❌ Individual font per window
    
    void InitializeFont() {
        nk_font_atlas_init_default(&m_atlas);  // ❌ Duplicate font baking
        // ... duplicate font setup for each window
    }
};
```

### Why This Fails
- **Memory waste**: Font atlases are large and shouldn't be duplicated
- **Inconsistent rendering**: Different font instances may render differently
- **Complex management**: Multiple atlases need individual cleanup
- **Performance impact**: Multiple font texture uploads

### Symptoms
- High memory usage
- Inconsistent text rendering between windows
- Slow initialization
- Font-related crashes during cleanup

## ❌ ANTI-PATTERN 6: Processing Input Per Window

### What NOT to Do
```cpp
// DON'T DO THIS - Causes input conflicts
class NKWindow {
    void ProcessWindowInput(UINT msg, WPARAM wparam, LPARAM lparam) {
        nk_input_begin(&m_windowManager.GetContext());  // ❌ Multiple begin/end cycles
        
        // Process input for this window only
        switch (msg) {
            case WM_LBUTTONDOWN:
                nk_input_button(&m_windowManager.GetContext(), NK_BUTTON_LEFT, x, y, 1);
                break;
        }
        
        nk_input_end(&m_windowManager.GetContext());  // ❌ Conflicts with other windows
    }
};
```

### Why This Fails
- **Input conflicts**: Multiple begin/end cycles interfere with each other
- **Lost input events**: Events processed by one window aren't available to others
- **State corruption**: Nuklear's input state gets confused
- **Focus issues**: No clear input focus management

### Symptoms
- Input events not working reliably
- Some windows not responding to input
- Keyboard input going to wrong windows
- Mouse events being lost

## ❌ ANTI-PATTERN 7: Calling nk_clear() After Each Window

### What NOT to Do
```cpp
// DON'T DO THIS - Clears commands before other windows can use them
void UpdateAll() {
    for (NKWindow* window : m_windows) {
        window->Render();  // Generates commands
        nk_clear(&m_ctx);  // ❌ Clears commands immediately
    }
}
```

### Why This Fails
- **Lost commands**: Commands are cleared before other windows can process them
- **Incomplete rendering**: Only the last window gets rendered
- **Wasted work**: Previous windows' UI generation is discarded

### Symptoms
- Only one window renders correctly
- Other windows appear blank or corrupted
- UI elements disappearing

## ❌ ANTI-PATTERN 8: Using GetMessage() Instead of PeekMessage()

### What NOT to Do
```cpp
// DON'T DO THIS - Blocks the render loop
while (running) {
    GetMessage(&msg, NULL, 0, 0);  // ❌ Blocks until message arrives
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    
    windowManager.UpdateAll();  // ❌ Only called when messages arrive
}
```

### Why This Fails
- **Blocking behavior**: GetMessage() blocks until a message arrives
- **No continuous rendering**: UI only updates when Windows messages are received
- **Poor responsiveness**: Application appears frozen when no input occurs
- **Animation issues**: Smooth animations are impossible

### Symptoms
- Application appears to freeze
- UI only updates when user interacts
- No smooth animations
- Poor user experience

## ❌ ANTI-PATTERN 9: Ignoring DPI in Font Sizes

### What NOT to Do
```cpp
// DON'T DO THIS - Fixed font sizes don't scale with DPI
void InitializeFont() {
    m_font = nk_font_atlas_add_default(&atlas, 13, 0);  // ❌ Fixed 13pt font
}
```

### Why This Fails
- **Tiny fonts on high-DPI**: 13pt becomes unreadable on 4K displays
- **Inconsistent sizing**: UI elements don't scale properly
- **Poor user experience**: Text is too small to read comfortably

### Symptoms
- Very small, hard-to-read text on high-DPI displays
- UI elements appearing too small
- User complaints about readability

## ❌ ANTI-PATTERN 10: Manual DPI Scaling Calculations

### What NOT to Do
```cpp
// DON'T DO THIS - Manual DPI scaling is complex and error-prone
void ScaleForDPI() {
    int dpi = GetDpiForWindow(hwnd);  // ❌ Manual DPI detection
    float scale = dpi / 96.0f;        // ❌ Manual scaling calculations
    
    // ❌ Manually scale all UI elements
    int scaledWidth = (int)(originalWidth * scale);
    int scaledHeight = (int)(originalHeight * scale);
}
```

### Why This Fails
- **Complex calculations**: DPI scaling involves many edge cases
- **Rounding errors**: Manual scaling introduces visual artifacts
- **Maintenance burden**: Every UI element needs manual scaling
- **Inconsistent results**: Different scaling approaches cause inconsistencies

### Symptoms
- Blurry or misaligned UI elements
- Inconsistent scaling between different parts of the UI
- Complex, hard-to-maintain code
- Visual artifacts and rounding errors

## Summary of What Works Instead

1. **Single Context**: Use one `nk_context` shared by all windows
2. **Per-Window Rendering**: Process each window individually with immediate command processing
3. **Proper Message Loop**: Process ALL messages before UpdateAll()
4. **Centralized Management**: Window manager owns context and backends
5. **Single Input Cycle**: One input begin/end cycle for all windows
6. **Clear After All Windows**: Only call `nk_clear()` after processing all windows
7. **PeekMessage Loop**: Use PeekMessage() for non-blocking message processing
8. **Appropriate Font Sizes**: Use 16pt or larger fonts for high-DPI compatibility
9. **Let Nuklear Handle DPI**: Use dynamic layouts and let Nuklear scale automatically

These anti-patterns were discovered through extensive debugging and testing. Following the working patterns in `Nuklear_UI_BASICS.md` instead will save you significant development time and frustration.