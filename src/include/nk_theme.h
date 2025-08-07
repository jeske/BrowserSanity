/**
 * @file nk_theme.h
 * @brief Application color theme definitions for Nuklear UI
 */

#pragma once

#include "nuklear.h"

// Application color theme palette
#define NK_THEME_TEXT                nk_rgb(0, 0, 0)                    // Pure black - main text color
#define NK_THEME_WINDOW              nk_rgb(255, 255, 255)              // Pure white - window background
#define NK_THEME_HEADER              nk_rgb(245, 245, 245)              // Very light gray - header backgrounds
#define NK_THEME_BORDER              nk_rgb(180, 180, 180)              // Medium gray - visible borders
#define NK_THEME_BUTTON              nk_rgb(255, 255, 255)              // Pure white - button background
#define NK_THEME_BUTTON_HOVER        nk_rgb(240, 240, 240)              // Light gray - button hover state
#define NK_THEME_BUTTON_ACTIVE       nk_rgb(0, 122, 255)                // Blue - pressed buttons
#define NK_THEME_TOGGLE              nk_rgb(255, 255, 255)              // Pure white - toggle background
#define NK_THEME_TOGGLE_HOVER        nk_rgb(240, 240, 240)              // Light gray - toggle hover
#define NK_THEME_TOGGLE_CURSOR       nk_rgb(0, 122, 255)                // Blue - toggle indicator
#define NK_THEME_SELECT              nk_rgb(255, 255, 255)              // Pure white - select background
#define NK_THEME_SELECT_ACTIVE       nk_rgb(0, 122, 255)                // Blue - selected items
#define NK_THEME_SLIDER              nk_rgb(220, 220, 220)              // Light gray - slider track
#define NK_THEME_SLIDER_CURSOR       nk_rgb(0, 122, 255)                // Blue - slider handle
#define NK_THEME_SLIDER_CURSOR_HOVER nk_rgb(30, 144, 255)               // Bright blue - slider hover
#define NK_THEME_SLIDER_CURSOR_ACTIVE nk_rgb(0, 100, 200)               // Dark blue - slider active
#define NK_THEME_PROPERTY            nk_rgb(255, 255, 255)              // Pure white - property fields
#define NK_THEME_EDIT                nk_rgb(255, 255, 255)              // Pure white - text input fields
#define NK_THEME_EDIT_CURSOR         nk_rgb(0, 0, 0)                    // Pure black - text cursor
#define NK_THEME_COMBO               nk_rgb(255, 255, 255)              // Pure white - dropdown background
#define NK_THEME_CHART               nk_rgb(245, 245, 245)              // Very light gray - chart background
#define NK_THEME_CHART_COLOR         nk_rgb(0, 122, 255)                // Blue - chart elements
#define NK_THEME_CHART_COLOR_HIGHLIGHT nk_rgb(30, 144, 255)             // Bright blue - chart highlights
#define NK_THEME_SCROLLBAR           nk_rgb(235, 235, 235)              // Light gray - scrollbar track
#define NK_THEME_SCROLLBAR_CURSOR    nk_rgb(180, 180, 180)              // Medium gray - scrollbar thumb
#define NK_THEME_SCROLLBAR_CURSOR_HOVER nk_rgb(160, 160, 160)           // Darker gray - scrollbar hover
#define NK_THEME_SCROLLBAR_CURSOR_ACTIVE nk_rgb(140, 140, 140)          // Dark gray - scrollbar active
#define NK_THEME_TAB_HEADER          nk_rgb(235, 235, 235)              // Light gray - tab headers

// Window background color for OS-level windows
#define NK_THEME_OS_WINDOW_BG        nk_rgb(248, 248, 248)              // Light gray - OS window background