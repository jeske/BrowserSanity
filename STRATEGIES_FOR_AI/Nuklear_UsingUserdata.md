# Nuklear Userdata: Proper Usage Guide

## Overview

Nuklear's userdata mechanism allows you to attach custom application data to the GUI context, making it accessible throughout your UI code. This provides a clean way to connect your Nuklear interface to your application's state and logic without relying on global variables.

## Basic Usage

### Setting Userdata

```c
// Your application state structure
typedef struct {
    float settings[10];
    char filename[256];
    int current_tool;
    // ... other app data
} app_state_t;

app_state_t app_state = {0};

// Set userdata on the context
nk_set_user_data(ctx, nk_handle_ptr(&app_state));
```

### Accessing Userdata

```c
// In your UI code, retrieve the userdata
app_state_t* app = (app_state_t*)nk_get_user_data(ctx).ptr;

// Now you can access/modify your application state
if (nk_button_label(ctx, "Reset")) {
    memset(app->settings, 0, sizeof(app->settings));
}

nk_slider_float(ctx, 0.0f, &app->settings[0], 100.0f, 1.0f);
```

## Common Design Patterns

### 1. Application Context Pattern

Store all major application subsystems in a single context structure:

```c
typedef struct {
    // Application state
    game_state_t game;
    renderer_t renderer;
    audio_system_t audio;
    
    // UI state
    int selected_tab;
    float ui_scale;
    int show_debug_panel;
    
    // Resources
    texture_t* icons;
    font_t* fonts;
    asset_manager_t* assets;
} app_context_t;

app_context_t ctx_data = {0};
init_app_context(&ctx_data);

nk_set_user_data(gui_ctx, nk_handle_ptr(&ctx_data));
```

### 2. Callback Data Pattern

Useful for complex interactions and event handling:

```c
// For custom widgets or complex interactions
typedef struct {
    void (*on_file_selected)(const char* filename);
    void (*on_color_changed)(nk_color color);
    void (*on_settings_changed)(void);
    
    app_state_t* app_state;
    resource_manager_t* resources;
} ui_callbacks_t;

ui_callbacks_t callbacks = {
    .on_file_selected = handle_file_selection,
    .on_color_changed = handle_color_change,
    .on_settings_changed = save_settings_to_file,
    .app_state = &app_state,
    .resources = &resource_manager
};

nk_set_user_data(ctx, nk_handle_ptr(&callbacks));

// Usage in UI code
ui_callbacks_t* cb = (ui_callbacks_t*)nk_get_user_data(ctx).ptr;
if (nk_button_label(ctx, "Load File")) {
    char* filename = open_file_dialog();
    if (filename && cb->on_file_selected) {
        cb->on_file_selected(filename);
    }
}
```

### 3. Widget State Management

Create helper functions for clean, type-safe access:

```c
// Helper function to get typed userdata
static app_state_t* get_app_state(struct nk_context* ctx) {
    return (app_state_t*)nk_get_user_data(ctx).ptr;
}

// Use in modular UI functions
void draw_settings_panel(struct nk_context* ctx) {
    app_state_t* app = get_app_state(ctx);
    
    if (nk_begin(ctx, "Settings", nk_rect(10, 10, 300, 400), 
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "Volume:", NK_TEXT_LEFT);
        nk_slider_float(ctx, 0, &app->volume, 1.0f, 0.01f);
        
        nk_layout_row_dynamic(ctx, 25, 2);
        if (nk_button_label(ctx, "Save Settings")) {
            save_settings_to_file(app);
        }
        if (nk_button_label(ctx, "Reset to Default")) {
            reset_settings_to_default(app);
        }
    }
    nk_end(ctx);
}

void draw_game_hud(struct nk_context* ctx) {
    app_state_t* app = get_app_state(ctx);
    
    if (nk_begin(ctx, "HUD", nk_rect(10, 10, 200, 100),
                 NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_labelf(ctx, NK_TEXT_LEFT, "Health: %.0f/%.0f", 
                 app->player.health, app->player.max_health);
        nk_labelf(ctx, NK_TEXT_LEFT, "Score: %d", app->game.score);
        nk_labelf(ctx, NK_TEXT_LEFT, "Level: %d", app->game.level);
    }
    nk_end(ctx);
}
```

## Advanced Usage Patterns

### 1. Multiple Data Sources

Store references to different subsystems for better organization:

```c
// Store a pointer to a structure containing multiple pointers
typedef struct {
    game_world_t* world;
    player_t* player;
    ui_state_t* ui;
    resource_manager_t* resources;
    input_manager_t* input;
    config_t* config;
} data_refs_t;

data_refs_t refs = {
    .world = &game_world,
    .player = &player,
    .ui = &ui_state,
    .resources = &res_manager,
    .input = &input_mgr,
    .config = &app_config
};

nk_set_user_data(ctx, nk_handle_ptr(&refs));

// Access specific subsystems cleanly
void draw_inventory_ui(struct nk_context* ctx) {
    data_refs_t* refs = (data_refs_t*)nk_get_user_data(ctx).ptr;
    
    if (nk_begin(ctx, "Inventory", nk_rect(100, 100, 300, 400),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE)) {
        
        // Access player inventory through refs
        for (int i = 0; i < refs->player->inventory.item_count; i++) {
            item_t* item = &refs->player->inventory.items[i];
            
            if (nk_button_label(ctx, item->name)) {
                use_item(refs->player, item, refs->world);
            }
        }
        
        if (nk_button_label(ctx, "Heal Player")) {
            refs->player->health = refs->player->max_health;
            play_sound(refs->resources, "heal_sound");
        }
    }
    nk_end(ctx);
}
```

### 2. Dynamic Userdata Updates

Change userdata during runtime for different application states:

```c
typedef enum {
    APP_STATE_MENU,
    APP_STATE_GAME,
    APP_STATE_EDITOR,
    APP_STATE_SETTINGS
} app_state_type_t;

typedef struct {
    app_state_type_t type;
    void* data;
} dynamic_context_t;

// Switch between different contexts
void switch_to_game_mode(struct nk_context* ctx) {
    static dynamic_context_t game_ctx = {0};
    game_ctx.type = APP_STATE_GAME;
    game_ctx.data = create_game_context();
    
    nk_set_user_data(ctx, nk_handle_ptr(&game_ctx));
}

void switch_to_menu_mode(struct nk_context* ctx) {
    static dynamic_context_t menu_ctx = {0};
    menu_ctx.type = APP_STATE_MENU;
    menu_ctx.data = create_menu_context();
    
    nk_set_user_data(ctx, nk_handle_ptr(&menu_ctx));
}

void switch_to_editor_mode(struct nk_context* ctx) {
    static dynamic_context_t editor_ctx = {0};
    editor_ctx.type = APP_STATE_EDITOR;
    editor_ctx.data = create_editor_context();
    
    nk_set_user_data(ctx, nk_handle_ptr(&editor_ctx));
}

// Generic UI drawing based on current context
void draw_ui(struct nk_context* ctx) {
    dynamic_context_t* dyn_ctx = (dynamic_context_t*)nk_get_user_data(ctx).ptr;
    
    switch (dyn_ctx->type) {
        case APP_STATE_MENU:
            draw_main_menu(ctx, (menu_context_t*)dyn_ctx->data);
            break;
        case APP_STATE_GAME:
            draw_game_ui(ctx, (game_context_t*)dyn_ctx->data);
            break;
        case APP_STATE_EDITOR:
            draw_editor_ui(ctx, (editor_context_t*)dyn_ctx->data);
            break;
        case APP_STATE_SETTINGS:
            draw_settings_ui(ctx, (settings_context_t*)dyn_ctx->data);
            break;
    }
}
```

### 3. Type-Safe Helper Macros

Create macros for cleaner, more maintainable code:

```c
// Create type-safe accessors
#define GET_APP_DATA(ctx) ((app_state_t*)nk_get_user_data(ctx).ptr)
#define GET_RENDERER(ctx) (&GET_APP_DATA(ctx)->renderer)
#define GET_SETTINGS(ctx) (&GET_APP_DATA(ctx)->settings)
#define GET_PLAYER(ctx) (&GET_APP_DATA(ctx)->player)
#define GET_RESOURCES(ctx) (GET_APP_DATA(ctx)->resources)

// Usage becomes much cleaner
void draw_graphics_settings(struct nk_context* ctx) {
    settings_t* settings = GET_SETTINGS(ctx);
    renderer_t* renderer = GET_RENDERER(ctx);
    
    if (nk_begin(ctx, "Graphics", nk_rect(50, 50, 400, 300),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        
        nk_layout_row_dynamic(ctx, 25, 2);
        
        nk_label(ctx, "Resolution:", NK_TEXT_LEFT);
        if (nk_combo_begin_label(ctx, settings->resolution_name, 
                                nk_vec2(200, 100))) {
            nk_layout_row_dynamic(ctx, 25, 1);
            for (int i = 0; i < RESOLUTION_COUNT; i++) {
                if (nk_combo_item_label(ctx, resolution_names[i], NK_TEXT_LEFT)) {
                    settings->resolution_index = i;
                    renderer_set_resolution(renderer, &resolutions[i]);
                }
            }
            nk_combo_end(ctx);
        }
        
        if (nk_checkbox_label(ctx, "VSync", &settings->vsync)) {
            renderer_set_vsync(renderer, settings->vsync);
        }
        
        if (nk_checkbox_label(ctx, "Fullscreen", &settings->fullscreen)) {
            renderer_set_fullscreen(renderer, settings->fullscreen);
        }
    }
    nk_end(ctx);
}

void draw_audio_settings(struct nk_context* ctx) {
    settings_t* settings = GET_SETTINGS(ctx);
    
    if (nk_begin(ctx, "Audio", nk_rect(100, 100, 350, 250),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        
        nk_layout_row_dynamic(ctx, 25, 1);
        
        nk_label(ctx, "Master Volume:", NK_TEXT_LEFT);
        nk_slider_float(ctx, 0, &settings->master_volume, 1.0f, 0.01f);
        
        nk_label(ctx, "Music Volume:", NK_TEXT_LEFT);
        nk_slider_float(ctx, 0, &settings->music_volume, 1.0f, 0.01f);
        
        nk_label(ctx, "SFX Volume:", NK_TEXT_LEFT);
        nk_slider_float(ctx, 0, &settings->sfx_volume, 1.0f, 0.01f);
        
        nk_layout_row_dynamic(ctx, 25, 2);
        if (nk_button_label(ctx, "Test Sound")) {
            audio_play_test_sound(GET_RESOURCES(ctx), settings->sfx_volume);
        }
        
        if (nk_button_label(ctx, "Apply")) {
            audio_apply_settings(GET_RESOURCES(ctx), settings);
        }
    }
    nk_end(ctx);
}
```

## Best Practices

### 1. Keep Data Structures Simple

Avoid overly complex nested structures:

```c
// Good: Simple, focused data structure
typedef struct {
    // Core application state
    player_t player;
    game_world_t world;
    
    // Settings
    graphics_settings_t graphics;
    audio_settings_t audio;
    
    // UI state
    ui_state_t ui;
} app_userdata_t;

// Avoid: Overly complex nested structures
/*
typedef struct {
    struct {
        struct {
            struct {
                // Too deeply nested - hard to access and maintain
            } level4;
        } level3;
    } level2;
} complex_userdata_t;
*/
```

### 2. Proper Memory Management

Ensure userdata lifetime matches or exceeds context lifetime:

```c
// Static allocation (simplest)
static app_state_t app_state = {0};
nk_set_user_data(ctx, nk_handle_ptr(&app_state));

// Dynamic allocation
app_state_t* app_state = malloc(sizeof(app_state_t));
if (!app_state) {
    // Handle allocation failure
    return -1;
}

init_app_state(app_state);
nk_set_user_data(ctx, nk_handle_ptr(app_state));

// ... use GUI throughout application lifetime

// Clean up when done (before destroying context)
app_state_t* app = (app_state_t*)nk_get_user_data(ctx).ptr;
if (app) {
    cleanup_app_state(app);
    free(app);
}

// Clear userdata to avoid dangling pointers
nk_set_user_data(ctx, nk_handle_ptr(NULL));
```

### 3. Null Safety and Defensive Programming

Always validate userdata before use:

```c
static app_state_t* safe_get_app_state(struct nk_context* ctx) {
    if (!ctx) {
        // Log error or handle gracefully
        return NULL;
    }
    
    nk_handle handle = nk_get_user_data(ctx);
    if (!handle.ptr) {
        // No userdata set - this might be intentional
        return NULL;
    }
    
    return (app_state_t*)handle.ptr;
}

// Use the safe accessor everywhere
void some_ui_function(struct nk_context* ctx) {
    app_state_t* app = safe_get_app_state(ctx);
    if (!app) {
        // Handle gracefully - maybe show error message or return
        nk_label(ctx, "Application data not available", NK_TEXT_CENTERED);
        return;
    }
    
    // Safe to use app now
    if (nk_button_label(ctx, "Save Game")) {
        save_game_state(app);
    }
}
```

### 4. Avoid Global State

Use userdata instead of global variables for better encapsulation:

```c
// Instead of global variables (BAD)
/*
int g_current_level = 1;
float g_player_health = 100.0f;
char g_player_name[64];
int g_high_score = 0;
*/

// Store in userdata structure (GOOD)
typedef struct {
    int current_level;
    float player_health;
    char player_name[64];
    int high_score;
    
    // Easy to extend with more state
    int lives_remaining;
    float play_time;
    achievement_t unlocked_achievements[MAX_ACHIEVEMENTS];
} game_state_t;

game_state_t game = {
    .current_level = 1,
    .player_health = 100.0f,
    .player_name = "Player",
    .high_score = 0,
    .lives_remaining = 3,
    .play_time = 0.0f
};

nk_set_user_data(ctx, nk_handle_ptr(&game));
```

### 5. Modular UI Design

Use userdata to enable modular, reusable UI components:

```c
// Generic UI components that work with any compatible userdata
void draw_health_bar(struct nk_context* ctx, float health, float max_health) {
    nk_layout_row_dynamic(ctx, 20, 1);
    
    float health_ratio = health / max_health;
    nk_color bar_color = health_ratio > 0.5f ? nk_rgb(0, 255, 0) : 
                        health_ratio > 0.25f ? nk_rgb(255, 255, 0) : 
                                               nk_rgb(255, 0, 0);
    
    char health_text[64];
    snprintf(health_text, sizeof(health_text), "Health: %.0f/%.0f", 
             health, max_health);
    
    nk_label(ctx, health_text, NK_TEXT_CENTERED);
    
    // Draw health bar
    struct nk_rect bar_rect = nk_widget_bounds(ctx);
    bar_rect.h = 10;
    bar_rect.w *= health_ratio;
    
    nk_fill_rect(&ctx->current->buffer, bar_rect, 0, bar_color);
}

void draw_player_stats_panel(struct nk_context* ctx) {
    app_state_t* app = safe_get_app_state(ctx);
    if (!app) return;
    
    if (nk_begin(ctx, "Player Stats", nk_rect(10, 10, 250, 150),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        
        draw_health_bar(ctx, app->player.health, app->player.max_health);
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_labelf(ctx, NK_TEXT_LEFT, "Level: %d", app->player.level);
        nk_labelf(ctx, NK_TEXT_LEFT, "XP: %d/%d", app->player.xp, app->player.xp_to_next);
        nk_labelf(ctx, NK_TEXT_LEFT, "Gold: %d", app->player.gold);
    }
    nk_end(ctx);
}
```

## Common Pitfalls to Avoid

### 1. Forgetting to Initialize Userdata
```c
// BAD: Uninitialized userdata
app_state_t app_state; // Contains garbage data
nk_set_user_data(ctx, nk_handle_ptr(&app_state));

// GOOD: Properly initialized
app_state_t app_state = {0}; // Zero-initialized
// OR
app_state_t app_state;
memset(&app_state, 0, sizeof(app_state));
init_app_state_defaults(&app_state);
nk_set_user_data(ctx, nk_handle_ptr(&app_state));
```

### 2. Casting to Wrong Type
```c
// BAD: Assuming userdata type without verification
player_t* player = (player_t*)nk_get_user_data(ctx).ptr;

// GOOD: Use consistent typing and helper functions
app_state_t* app = safe_get_app_state(ctx);
if (app) {
    player_t* player = &app->player;
}
```

### 3. Memory Lifetime Issues
```c
// BAD: Local variable goes out of scope
void setup_ui() {
    app_state_t local_state = {0};
    nk_set_user_data(ctx, nk_handle_ptr(&local_state));
    // local_state destroyed when function returns!
}

// GOOD: Persistent storage
static app_state_t persistent_state = {0};
void setup_ui() {
    nk_set_user_data(ctx, nk_handle_ptr(&persistent_state));
}
```

## Conclusion

Nuklear's userdata mechanism is a powerful tool for connecting your UI to your application state. By following these patterns and best practices, you can create maintainable, type-safe, and efficient GUI applications that avoid the pitfalls of global state while keeping your code organized and modular.

Key takeaways:
- Use userdata to eliminate global variables
- Create helper functions for type-safe access
- Always validate userdata before use
- Keep data structures simple and focused
- Match userdata lifetime to context lifetime
- Design modular UI components that work with your userdata structure