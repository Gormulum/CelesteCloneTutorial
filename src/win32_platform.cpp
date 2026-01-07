#include "platform.h"
#include "celeste_lib.h"
#include "glcorearb.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "wglext.h"

// ###################################################################
// Windows Globals
// ###################################################################

static HWND window;
static HDC dc;

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

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

    //fake window to initialize opengl 
    {
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
            SM_ASSERT(false, "Failed to create window");
            return false; 
        } 

        //HDC = device context
        HDC fakeDC = GetDC(window);
        if(!fakeDC)
        {
            SM_ASSERT(false, "Failed to get HDC");
            return false; 
        }

        PIXELFORMATDESCRIPTOR pfd = {0};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.cDepthBits = 24;

        int pixelFormat = ChoosePixelFormat(fakeDC, &pfd);

        if (!pixelFormat)
        {
            SM_ASSERT(false, "Failed to choose pixel format");
            return false; 
        }

        if (!SetPixelFormat(fakeDC, pixelFormat, &pfd))
        {
            SM_ASSERT(false, "Failed to set pixel format");
            return false; 
        }

        //RC = rendering context
        //Creates a handle to a fake OpenGL rendering context
        HGLRC fakeRC = wglCreateContext(fakeDC);
        if (!fakeRC)
        {
            SM_ASSERT(false, "Failed to create Render context");
            return false; 
        }

        if(!wglMakeCurrent(fakeDC, fakeRC))
        {
            SM_ASSERT(false, "Failed to make context current");
            return false; 
        }

        wglChoosePixelFormatARB = 
            (PFNWGLCHOOSEPIXELFORMATARBPROC)platform_load_gl_function("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB = 
            (PFNWGLCREATECONTEXTATTRIBSARBPROC)platform_load_gl_function("wglCreateContextAttribsARB");

        if (!wglCreateContextAttribsARB || !wglChoosePixelFormatARB)
        {
            SM_ASSERT(false, "Failed to load OpenGL functions");
            return false; 
        }

        //clean up fake window and context

        wglMakeCurrent(fakeDC, 0);
        wglDeleteContext(fakeRC);
        ReleaseDC(window, fakeDC);

        //Can't reuse the same (Device)context,
        //because its already called "SetPixelFormat"
        DestroyWindow(window);
    }

    // Actual OpenGL initialization 
    {
        //border size
        {
            RECT borderRect = {};
            AdjustWindowRectEx(&borderRect, dwStyle, 0, 0);

            width += borderRect.right - borderRect.left;
            height += borderRect.bottom - borderRect.top;
        }

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
            SM_ASSERT(false, "Failed to create window");
            return false; 
        } 

        //HDC = device context
        HDC fakeDC = GetDC(window);
        if(!fakeDC)
        {
            SM_ASSERT(false, "Failed to get HDC");
            return false; 
        }

        const int pixelAttribs[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,    
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_SWAP_METHOD_ARB, WGL_SWAP_COPY_ARB,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_ALPHA_BITS_ARB, 8,
            WGL_DEPTH_BITS_ARB, 24,
            0 // terminate with this 0, otherwise opengl will throw an error
        };

        UINT numPixelFormats;
        int pixelFormat = 0;
        if (!wglChoosePixelFormatARB(
            dc,
            pixelAttribs,
            0, //float list
            1, //max formats
            &pixelFormat,
            &numPixelFormats))
        {
            SM_ASSERT(false, "Failed to wglChoosePixelFormatARB");
            return false; 
        }

        PIXELFORMATDESCRIPTOR pfd = {0};
        DescribePixelFormat(fakeDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

        if (!SetPixelFormat(fakeDC, pixelFormat, &pfd))
        {
            SM_ASSERT(0, "Failed to SetPixelFormat");
            return false; 
        }

        const int contextAttribs[] = 
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 5,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
            0 // terminate with this 0, otherwise opengl will throw an error
        };

        HGLRC rc = wglCreateContextAttribsARB(dc, 0, contextAttribs);
        if (!rc)
        {
            SM_ASSERT(0, "Failed to create render context for OpenGL");
            return false; 
        }

        if (!wglMakeCurrent(dc, rc))
        {
            SM_ASSERT(0, "Failed to wglMakeCurrent");
            return false; 
        }
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

void* platform_load_gl_function(const char* funName)
{
    PROC proc = wglGetProcAddress(funName);
    if(!proc)
    {
        static HMODULE openglDLL = LoadLibraryA("opengl32.dll"); 
        proc = GetProcAddress(openglDLL, funName);
    
        if(!proc)
        {
            SM_ASSERT(false, "Failed to load OpenGL function: glCreateProgram");
            return nullptr;
        }
    }

    return (void*)proc;
}
