
#ifndef MAIN_H
#define MAIN_H
#include <memory>

// Application state
struct BrowserSanityApp {
    HINSTANCE hInstance;
    bool running;
    
    // Window instances
    std::unique_ptr<NKWindow> mainWindow;
    std::unique_ptr<NKWindow> settingsWindow;
    std::unique_ptr<NKWindow> toastWindow;
    
    BrowserSanityApp() : hInstance(nullptr), running(true) {}
};

extern BrowserSanityApp g_app;

#endif MAIN_H