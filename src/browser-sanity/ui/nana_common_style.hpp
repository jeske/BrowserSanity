/**
 * @file nana_common_style.hpp
 * @brief Common styling for all Nana UI components
 * 
 * This file provides consistent styling, colors, and UI constants
 * for all Browser Sanity UI components.
 */

#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/paint/image.hpp>

namespace BrowserSanity {
namespace Style {

// Color scheme
const nana::color PRIMARY_COLOR = nana::colors::blue;
const nana::color SECONDARY_COLOR = nana::color(0, 120, 215); // Windows 10 blue
const nana::color ACCENT_COLOR = nana::color(255, 185, 0);    // Amber accent
const nana::color SUCCESS_COLOR = nana::color(92, 184, 92);   // Green
const nana::color WARNING_COLOR = nana::color(240, 173, 78);  // Orange
const nana::color DANGER_COLOR = nana::color(217, 83, 79);    // Red
const nana::color INFO_COLOR = nana::color(91, 192, 222);     // Light blue
const nana::color BACKGROUND_COLOR = nana::colors::white;
const nana::color TEXT_COLOR = nana::color(51, 51, 51);       // Dark gray
const nana::color LIGHT_TEXT_COLOR = nana::color(119, 119, 119); // Medium gray

// Typography
const int TITLE_FONT_SIZE = 16;
const int SUBTITLE_FONT_SIZE = 14;
const int NORMAL_FONT_SIZE = 12;
const int SMALL_FONT_SIZE = 10;

// Spacing
const int MARGIN = 20;
const int GAP = 15;
const int SMALL_GAP = 8;

// Common window sizes
const int SMALL_WINDOW_WIDTH = 400;
const int SMALL_WINDOW_HEIGHT = 300;
const int MEDIUM_WINDOW_WIDTH = 500;
const int MEDIUM_WINDOW_HEIGHT = 400;
const int LARGE_WINDOW_WIDTH = 600;
const int LARGE_WINDOW_HEIGHT = 500;

/**
 * @brief Apply common styling to a form
 * @param form The form to style
 */
inline void ApplyFormStyle(nana::form& form) {
    form.bgcolor(BACKGROUND_COLOR);
}

/**
 * @brief Create a title font (large, bold)
 * @return A font object for titles
 */
inline nana::paint::font TitleFont() {
    return nana::paint::font("", TITLE_FONT_SIZE, true); // Bold
}

/**
 * @brief Create a subtitle font (medium, bold)
 * @return A font object for subtitles
 */
inline nana::paint::font SubtitleFont() {
    return nana::paint::font("", SUBTITLE_FONT_SIZE, true); // Bold
}

/**
 * @brief Create a normal font
 * @return A font object for normal text
 */
inline nana::paint::font NormalFont() {
    return nana::paint::font("", NORMAL_FONT_SIZE, false); // Not bold
}

/**
 * @brief Create a small font
 * @return A font object for small text
 */
inline nana::paint::font SmallFont() {
    return nana::paint::font("", SMALL_FONT_SIZE, false); // Not bold
}

/**
 * @brief Style a title label
 * @param label The label to style
 * @param text The text for the label
 */
inline void StyleTitleLabel(nana::label& label, const std::string& text) {
    label.caption(text);
    label.text_align(nana::align::center);
    label.typeface(TitleFont());
    label.fgcolor(TEXT_COLOR);
}

/**
 * @brief Style a subtitle label
 * @param label The label to style
 * @param text The text for the label
 */
inline void StyleSubtitleLabel(nana::label& label, const std::string& text) {
    label.caption(text);
    label.text_align(nana::align::center);
    label.typeface(SubtitleFont());
    label.fgcolor(TEXT_COLOR);
}

/**
 * @brief Style a normal label
 * @param label The label to style
 * @param text The text for the label
 */
inline void StyleNormalLabel(nana::label& label, const std::string& text) {
    label.caption(text);
    label.typeface(NormalFont());
    label.fgcolor(TEXT_COLOR);
}

/**
 * @brief Style a small label
 * @param label The label to style
 * @param text The text for the label
 */
inline void StyleSmallLabel(nana::label& label, const std::string& text) {
    label.caption(text);
    label.typeface(SmallFont());
    label.fgcolor(LIGHT_TEXT_COLOR);
}

/**
 * @brief Style a primary button
 * @param button The button to style
 * @param text The text for the button
 */
inline void StylePrimaryButton(nana::button& button, const std::string& text) {
    button.caption(text);
    button.typeface(NormalFont());
    button.bgcolor(PRIMARY_COLOR);
    button.fgcolor(nana::colors::white);
    button.enable_focus_color(true);
}

/**
 * @brief Style a secondary button
 * @param button The button to style
 * @param text The text for the button
 */
inline void StyleSecondaryButton(nana::button& button, const std::string& text) {
    button.caption(text);
    button.typeface(NormalFont());
    button.enable_focus_color(true);
}

/**
 * @brief Style a danger button (e.g., uninstall, delete)
 * @param button The button to style
 * @param text The text for the button
 */
inline void StyleDangerButton(nana::button& button, const std::string& text) {
    button.caption(text);
    button.typeface(NormalFont());
    button.bgcolor(DANGER_COLOR);
    button.fgcolor(nana::colors::white);
    button.enable_focus_color(true);
}

/**
 * @brief Create a standard place layout string with consistent margins and gaps
 * @return A layout string for nana::place
 */
inline std::string StandardLayoutString() {
    return "vert margin=" + std::to_string(MARGIN) + " gap=" + std::to_string(GAP);
}

/**
 * @brief Load application icon
 * @param hInstance The application instance handle
 * @return An image object containing the icon
 */
inline nana::paint::image LoadAppIcon(HINSTANCE hInstance) {
    nana::paint::image img;
    
    // Try to load from resources
    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
    if (hIcon) {
        // Convert HICON to nana::paint::image
        // This is a placeholder - in a real implementation, you would
        // need to convert the HICON to a format that nana::paint::image can use
    } else {
        // Fallback to default application icon
        hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    
    return img;
}

} // namespace Style
} // namespace BrowserSanity