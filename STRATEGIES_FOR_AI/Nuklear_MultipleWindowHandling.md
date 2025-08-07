# Two Window Nuklear Window Manager (basic c-style version)

This is a complete Win32 GDI implementation that's truly header-only and dependency-free. Here are the key benefits:
Key Features
Pure GDI Rendering: Uses CreateCompatibleDC, CreateDIBSection, and standard GDI functions like Rectangle, FillRect, DrawText - no OpenGL required.
Zero External Dependencies: Only needs:

nuklear.h (header-only)
Windows SDK (which you already have)
Standard C runtime

Multiple Independent Contexts: Each window has its own:

HWND (window handle)
HDC (device context)
Memory DC for double buffering
Nuklear context and state

Architecture
Double Buffering: Each window renders to a memory bitmap, then BitBlt copies to the actual window during WM_PAINT.
Message Pump: Handles all Windows messages in the main loop, routing input to the appropriate Nuklear context.
Dynamic Window Management: Tool window is created/destroyed on demand while maintaining separate state.
Compilation
This will compile with just:
batchcl /EHsc nuklear_win32.c user32.lib gdi32.lib
No need to build external libraries, link complex dependencies, or set up build systems. The GDI backend handles all the rendering primitives that Nuklear needs:

Lines and rectangles for borders/backgrounds
Text rendering with DrawText
Clipping regions for proper UI rendering
Color management through GDI brushes and pens

This approach is perfect for tools, utilities, or applications where you want a lightweight GUI without the complexity of modern graphics APIs or heavy GUI frameworks.

For two window example code, see: STRATEGIES_FOR_AI\Nuklear_MultipleWindow_Examplecode.c
