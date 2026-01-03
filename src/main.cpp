#include "celeste_lib.h"
// ###################################################################
// Platform Globals
// ###################################################################

static bool running = true;

// ###################################################################
// Platform Functions
// ###################################################################

bool platform_create_window(int width, int height, const char* title);
void platform_update_window();

// ###################################################################
// Windows Plaform
// ###################################################################

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// ###################################################################
// Windows Globals
// ###################################################################

static HWND window;

// ###################################################################
// Platform Implementations
// ###################################################################
LRESULT CALLBACK windows_window_callback(HWND window, UINT msg, 
                                        WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch(msg)
    {
        case WM_CLOSE:
        {
            running = false; 
            break;
        }

        default:
        {
            //Windows default input handling
            result = DefWindowProcA(window, msg, wParam, lParam); // Default window procedure
        }
    }

    return result; // Return the result of the message processing
}



bool platform_create_window(int width, int height, const char* title)
{
    HINSTANCE instance = GetModuleHandleA(0);

    WNDCLASSA wc = {};
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);   // This means we decide the look of the cursor
    wc.lpszClassName = title;                   // This is not the window title, just a unique identifier (ID)
    wc.lpfnWndProc = windows_window_callback;   // Callback for input into the window

    if (!RegisterClassA(&wc))
    {
        return false; 
    }

    // WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME
    int dwStyle = WS_OVERLAPPEDWINDOW;

    window = CreateWindowExA(
        0, // Extended styles (drag and drop, display in taskbar, etc.)
        title, // Class name
        title, // Window title
        dwStyle, // Style
        100, 100, width, height, // Position and size
        NULL, // Parent window
        NULL, // Menu
        instance, // Instance handle
        NULL // Additional data     
    );

    if (window == NULL)
    {
        return false; 
    }

    ShowWindow(window, SW_SHOW); // Show the window

    return true;
}

void platform_update_window()
{
    MSG msg;

    while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg); //Calls to the callback specified when creating the window
    }
}


#endif



int main()
{

    platform_create_window(800, 600, "Window");
    while(running)
    {
        //update
        platform_update_window();

        SM_TRACE("test");
        SM_WARN("test");
        SM_ERROR("test");
    } 

    return 0; 
}