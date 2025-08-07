# Nuklear Dirty Event Detection and Optimization

## Overview

Nuklear is an immediate mode GUI library, which means it redraws the entire interface every frame by default. However, this can be inefficient for applications that don't need constant updates. Nuklear provides several mechanisms to help detect when actual changes occur, allowing you to implement smart repainting strategies.

## Key Mechanisms

### 1. Widget Return Values

Most Nuklear widgets return values indicating user interaction or state changes:

```c
// Button returns 1 when clicked, 0 otherwise
if (nk_button_label(ctx, "Save")) {
    // Button was clicked - UI is dirty
    needs_repaint = 1;
}

// Sliders return 1 when value changes
if (nk_slider_float(ctx, 0.0f, &value, 100.0f, 1.0f)) {
    // Slider value changed - UI is dirty
    needs_repaint = 1;
}

// Checkboxes return 1 when toggled
if (nk_checkbox_label(ctx, "Enable feature", &checkbox_state)) {
    // Checkbox was toggled - UI is dirty
    needs_repaint = 1;
}
```

### 2. Activity Detection Functions

Nuklear provides functions to detect general UI activity:

```c
// Check if any item is currently active (being interacted with)
if (nk_item_is_any_active(ctx)) {
    needs_repaint = 1;
}

// Check if mouse is over any item
if (nk_widget_is_mouse_clicked(ctx, NK_BUTTON_LEFT)) {
    needs_repaint = 1;
}
```

### 3. Context State Monitoring

Monitor the Nuklear context for changes:

```c
// Store previous frame state
static struct nk_input prev_input;

// Compare current input with previous frame
if (memcmp(&ctx->input, &prev_input, sizeof(struct nk_input)) != 0) {
    needs_repaint = 1;
    prev_input = ctx->input;
}
```

## Implementation Patterns

### Basic Dirty Tracking

```c
int needs_repaint = 0;
static int last_frame_active = 0;

if (nk_begin(ctx, "Main Window", nk_rect(0, 0, 400, 300), 
             NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
    
    // Track widget interactions
    if (nk_button_label(ctx, "Action")) {
        perform_action();
        needs_repaint = 1;
    }
    
    static float slider_val = 50.0f;
    if (nk_slider_float(ctx, 0, &slider_val, 100, 1)) {
        needs_repaint = 1;
    }
    
    static char text_buffer[256];
    static int text_len = 0;
    int old_len = text_len;
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, text_buffer, 
                                   sizeof(text_buffer), nk_filter_default);
    if (strlen(text_buffer) != old_len) {
        needs_repaint = 1;
    }
}
nk_end(ctx);

// Check for any ongoing interactions
int currently_active = nk_item_is_any_active(ctx);
if (currently_active || last_frame_active) {
    needs_repaint = 1;
}
last_frame_active = currently_active;

// Only render if something changed
if (needs_repaint) {
    render_frame();
}
```

### Advanced Dirty Region Tracking

```c
typedef struct {
    int dirty;
    nk_flags changed_regions;
} ui_state_t;

static ui_state_t ui_state = {0};

#define UI_REGION_TOOLBAR  (1 << 0)
#define UI_REGION_SIDEBAR  (1 << 1) 
#define UI_REGION_CONTENT  (1 << 2)

// Toolbar window
if (nk_begin(ctx, "Toolbar", toolbar_rect, NK_WINDOW_NO_SCROLLBAR)) {
    if (nk_button_label(ctx, "New")) {
        ui_state.dirty = 1;
        ui_state.changed_regions |= UI_REGION_TOOLBAR | UI_REGION_CONTENT;
    }
}
nk_end(ctx);

// Only repaint affected regions
if (ui_state.dirty) {
    if (ui_state.changed_regions & UI_REGION_TOOLBAR) {
        render_toolbar();
    }
    if (ui_state.changed_regions & UI_REGION_CONTENT) {
        render_content();
    }
    
    ui_state.dirty = 0;
    ui_state.changed_regions = 0;
}
```

## Best Practices

### 1. Minimize State Comparisons

Only check for changes where necessary:

```c
// Good: Only check return values from widgets that matter
if (nk_button_label(ctx, "Important")) {
    needs_repaint = 1;
}

// Avoid: Checking every possible state every frame
// if (memcmp(&entire_app_state, &prev_state, sizeof(app_state))) // expensive
```

### 2. Use Hierarchical Dirty Flags

Organize dirty detection by UI regions or logical components:

```c
typedef struct {
    int menu_dirty;
    int viewport_dirty; 
    int statusbar_dirty;
} dirty_flags_t;

static dirty_flags_t dirty = {0};

// Update only what's needed
if (dirty.menu_dirty) {
    render_menu();
    dirty.menu_dirty = 0;
}
```

### 3. Handle Animation and Timers

Remember to account for time-based updates:

```c
static float animation_time = 0.0f;
static int animating = 0;

if (animating) {
    animation_time += delta_time;
    needs_repaint = 1;
    
    if (animation_time >= animation_duration) {
        animating = 0;
    }
}
```

### 4. Batch Updates

Group related changes to minimize repaints:

```c
int batch_dirty = 0;

// Process multiple related widgets
if (nk_button_label(ctx, "Apply Changes")) {
    batch_dirty |= update_setting_1();
    batch_dirty |= update_setting_2(); 
    batch_dirty |= update_setting_3();
}

if (batch_dirty) {
    refresh_ui();
    needs_repaint = 1;
}
```

## Performance Considerations

### Frame Rate Optimization

```c
// Limit repaints to specific frame rates when idle
static double last_repaint_time = 0.0;
const double idle_fps_limit = 1.0 / 30.0; // 30 FPS when idle

double current_time = get_time();
int force_repaint = (current_time - last_repaint_time) >= idle_fps_limit;

if (needs_repaint || force_repaint) {
    render_frame();
    last_repaint_time = current_time;
}
```

### Memory Efficiency

```c
// Avoid allocating memory for dirty tracking every frame
static int* widget_dirty_states = NULL;
static int widget_count = 0;

// Initialize once
if (!widget_dirty_states) {
    widget_count = get_widget_count();
    widget_dirty_states = malloc(widget_count * sizeof(int));
    memset(widget_dirty_states, 0, widget_count * sizeof(int));
}
```

## Common Pitfalls

### 1. Forgetting Input Events
Always check for mouse/keyboard activity even if widgets don't change:

```c
// Don't forget about hover states, mouse movement, etc.
if (nk_input_is_mouse_hovering_rect(&ctx->input, window_bounds)) {
    needs_repaint = 1; // For hover effects
}
```

### 2. Missing Window Events
Track window-level changes:

```c
// Check if window was moved, resized, etc.
if (nk_window_is_any_hovered(ctx) || nk_window_is_collapsed(ctx, "Window")) {
    needs_repaint = 1;
}
```

### 3. State Synchronization Issues
Ensure UI state matches application state:

```c
// Always sync critical state
if (app_state.value != ui_slider_value) {
    ui_slider_value = app_state.value;
    needs_repaint = 1;
}
```

## Conclusion

While Nuklear doesn't provide automatic dirty detection like retained mode GUIs, its widget return values and activity detection functions give you the tools needed to implement efficient repainting strategies. The key is to strategically monitor the interactions that actually matter for your application and batch updates appropriately.

Remember that the immediate mode paradigm means you're rebuilding the UI description each frame anyway, so the main optimization is in the actual rendering/presentation step rather than the UI logic itself.