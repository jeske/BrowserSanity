# Nana C++ GUI Library - How to Use Strategy

## Overview

Nana is a modern C++ GUI library that provides a clean, object-oriented approach to creating cross-platform desktop applications. Unlike Nuklear's immediate mode approach, Nana uses a retained mode GUI system with automatic layout management and event-driven programming.

## Key Architecture Concepts

### 1. **Form-Based Windows**
- **Primary Container**: [`nana::form`](../deps/nana/include/nana/gui/widgets/form.hpp) is the main window class
- **Nested Forms**: [`nana::nested_form`](../deps/nana/include/nana/gui/widgets/form.hpp) for child windows
- **Automatic Management**: Forms handle their own lifecycle and event processing

### 2. **Widget Hierarchy**
- **Base Class**: All widgets inherit from [`nana::widget`](../deps/nana/include/nana/gui/widgets/widget.hpp)
- **Common Widgets**: [`button`](../deps/nana/include/nana/gui/widgets/button.hpp), [`label`](../deps/nana/include/nana/gui/widgets/label.hpp), [`textbox`](../deps/nana/include/nana/gui/widgets/textbox.hpp), etc.
- **Parent-Child**: Widgets are created with a parent window reference

### 3. **Event-Driven Programming**
- **Event Handlers**: Use lambda functions or function objects for event handling
- **Automatic Dispatch**: Events are automatically routed to appropriate handlers
- **Type Safety**: Strong typing prevents common GUI programming errors

### 4. **Layout Management**
- **Place Layout**: [`nana::place`](../deps/nana/include/nana/gui/place.hpp) provides flexible layout management
- **Automatic Sizing**: Widgets can auto-size based on content
- **Responsive Design**: Layouts adapt to window resizing

## Basic Usage Patterns

### Creating a Simple Window
```cpp
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>

int main() {
    // Create main form
    nana::form mainWindow(nana::API::make_center(400, 300));
    mainWindow.caption("Browser Sanity - Main Control");
    
    // Create widgets
    nana::label statusLabel(mainWindow, "Status: Ready");
    nana::button installButton(mainWindow, "Install");
    nana::button exitButton(mainWindow, "Exit");
    
    // Set up event handlers
    installButton.events().click([&]() {
        statusLabel.caption("Installing...");
        // Perform installation logic
    });
    
    exitButton.events().click([&]() {
        mainWindow.close();
    });
    
    // Show window and start event loop
    mainWindow.show();
    nana::exec();
    
    return 0;
}
```

### Layout Management with Place
```cpp
// Create form with place layout
nana::form mainWindow;
nana::place layout(mainWindow);

// Define layout structure
layout.div("vert <status><buttons weight=40>");

// Create widgets
nana::label statusLabel(mainWindow);
nana::panel buttonPanel(mainWindow);

// Add widgets to layout
layout["status"] << statusLabel;
layout["buttons"] << buttonPanel;

// Apply layout
layout.collocate();
```

### Message Boxes and Dialogs
```cpp
#include <nana/gui/msgbox.hpp>

// Simple message box
nana::msgbox msg(mainWindow, "Installation Complete");
msg.icon(nana::msgbox::icon_information);
msg << "Browser Sanity has been successfully installed.";
auto result = msg.show();

// Input dialog
nana::inputbox input(mainWindow, "Enter redirect URL", "Configuration");
nana::inputbox::text urlInput("URL:", "https://www.google.com");

if (input.show(urlInput)) {
    std::string url = urlInput.value();
    // Use the URL
}
```

## Key Advantages Over Nuklear

### 1. **No Multi-Window Complexity**
- **Automatic Management**: Each form manages its own state and events
- **No Context Conflicts**: No shared contexts or assertion failures
- **Independent Windows**: Windows operate completely independently

### 2. **Modern C++ Design**
- **RAII**: Automatic resource management
- **Type Safety**: Compile-time error detection
- **STL Integration**: Works seamlessly with standard library

### 3. **Built-in Features**
- **DPI Awareness**: Automatic high-DPI scaling
- **Theme Support**: Built-in theming system
- **Layout Management**: Sophisticated layout engines
- **Event System**: Robust event handling with type safety

### 4. **Cross-Platform**
- **Windows**: Native Windows API integration
- **Linux**: X11 support
- **Consistent API**: Same code works across platforms

## Migration Strategy from Nuklear

### 1. **Window Architecture**
- **Replace**: `NKWindow` classes → `nana::form` classes
- **Simplify**: Remove complex window manager and context management
- **Event-Driven**: Replace immediate mode rendering with event handlers

### 2. **UI Components**
- **Direct Mapping**: Nuklear widgets → Nana widgets
  - `nk_button_label()` → `nana::button`
  - `nk_label()` → `nana::label`
  - `nk_checkbox_label()` → `nana::checkbox`
  - `nk_edit_string()` → `nana::textbox`

### 3. **Layout System**
- **Replace**: Manual Nuklear layouts → `nana::place` automatic layouts
- **Responsive**: Automatic resizing and DPI scaling
- **Declarative**: Define layout structure with strings

### 4. **Event Handling**
- **Replace**: Nuklear's return value checking → Event handler lambdas
- **Type Safe**: Compile-time event binding
- **Automatic**: No manual input processing required

## Build Integration

### Include Paths
```cpp
// Add to project include directories
deps/nana/include/

// Main header
#include <nana/gui.hpp>

// Specific widgets
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/msgbox.hpp>
```

### Linking
```cpp
// Link against Nana library (if using precompiled)
// Or include source files in build
deps/nana/source/gui/*.cpp
deps/nana/source/paint/*.cpp
deps/nana/source/system/*.cpp
```

### Preprocessor Definitions
```cpp
// Windows-specific
#define WIN32_LEAN_AND_MEAN
#define NANA_WINDOWS

// Optional features
#define NANA_ENABLE_AUDIO    // If using audio features
#define NANA_ENABLE_PNG      // If using PNG images
```

## Error Handling and Debugging

### 1. **Exception Safety**
- **RAII**: Automatic cleanup prevents resource leaks
- **Exception Handling**: Use try-catch for error handling
- **Validation**: Built-in parameter validation

### 2. **Debugging Support**
- **Debug Builds**: Enable debug information in Nana
- **Logging**: Use standard logging instead of custom debug system
- **Visual Debugging**: Use Visual Studio's GUI debugging tools

## Performance Considerations

### 1. **Retained Mode Benefits**
- **Efficient Updates**: Only changed widgets are redrawn
- **No Constant Rendering**: No need for continuous render loops
- **Event-Driven**: CPU usage only when needed

### 2. **Memory Management**
- **Automatic**: RAII handles widget lifecycle
- **Smart Pointers**: Use when needed for complex ownership
- **Stack Allocation**: Most widgets can be stack-allocated

## Best Practices

### 1. **Window Design**
- **Single Responsibility**: Each form handles one specific function
- **Composition**: Use member widgets instead of inheritance
- **Event Binding**: Bind events in constructor or initialization

### 2. **Layout Management**
- **Use Place**: Prefer `nana::place` over manual positioning
- **Responsive**: Design for different screen sizes
- **Consistent**: Use consistent spacing and alignment

### 3. **Error Handling**
- **Validate Input**: Check user input before processing
- **User Feedback**: Provide clear error messages
- **Graceful Degradation**: Handle errors without crashing

## Conclusion

Nana provides a much simpler and more robust solution for multi-window GUI applications compared to Nuklear. The retained mode approach eliminates the complex context management issues that plagued the Nuklear implementation, while providing modern C++ features and automatic resource management.

The migration from Nuklear to Nana will result in:
- **Simpler Code**: Less boilerplate and manual management
- **Better Reliability**: No assertion failures or context conflicts
- **Modern Design**: Type-safe, RAII-based architecture
- **Cross-Platform**: Same code works on multiple platforms
- **Maintainability**: Cleaner, more understandable codebase