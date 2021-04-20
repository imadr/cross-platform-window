#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int window_width, window_height;
uint32_t* buffer;

void init_buffer(int width, int height){
    buffer = malloc(width*height*sizeof(uint32_t));
}

#ifdef _WIN32

#include <windows.h>

HWND window_handle;
BITMAPINFO bitmap_info = {0};

void close_window(){
    UnregisterClass("WindowClass", GetModuleHandle(0));
    DestroyWindow(window_handle);
}

void draw_buffer(HDC device_context){
    StretchDIBits(device_context, 0, 0, window_width, window_height,
                    0, 0, window_width, window_height, buffer, &bitmap_info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK window_procedure(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param){
    switch(message){
        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window_handle, &paint);
            draw_buffer(device_context);
            EndPaint(window_handle, &paint);
            break;
        }
        case WM_CLOSE:
            close_window();
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(window_handle, message, w_param, l_param);
    }
    return 0;
}

void print_last_error(char* msg){
    wchar_t buff[256];
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_MAX_WIDTH_MASK,
        NULL, GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buff, (sizeof(buff)/sizeof(wchar_t)), NULL);
    fprintf(stderr, "%s, %S\n", msg, buff);
}

int create_window(char* title, int width, int height){
    window_width = width;
    window_height = height;

    const char* window_class_name = "WindowClass";
    WNDCLASS window_class = {0};
    window_class.lpfnWndProc = window_procedure;
    window_class.hInstance = GetModuleHandle(0);
    window_class.lpszClassName = window_class_name;

    if(!RegisterClass(&window_class)){
        print_last_error("RegisterClass: Error when creating window class");
        return 1;
    }

    DWORD window_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    RECT window_rect = {0, 0, window_width, window_height};
    AdjustWindowRect(&window_rect, window_style, FALSE);

    window_handle = CreateWindowEx(0,
        window_class_name,
        title,
        window_style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        window_rect.right-window_rect.left,
        window_rect.bottom-window_rect.top,
        NULL, NULL, GetModuleHandle(0), NULL);

    if(!window_handle){
        print_last_error("CreateWindowEx: Error when creating window");
        return 1;
    }

    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = window_width;
    bitmap_info.bmiHeader.biHeight = -window_height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    ShowWindow(window_handle, 5);

    return 0;
}

int event_loop(){
    MSG message;
    int ret = GetMessage(&message, NULL, 0, 0);
    TranslateMessage(&message);
    DispatchMessage(&message);
    return ret;
}

#elif __linux__

#ifdef X11

#include <X11/Xlib.h>
#include <X11/Xutil.h>

Display* display;
Window window;
Atom delete_window_atom;
XVisualInfo visual_info;
GC graphic_context;

int create_window(char* title, int width, int height){
    window_width = width;
    window_height = height;

    display = XOpenDisplay(0);

    if(display == NULL){
        fprintf(stderr, "XOpenDisplay: Error opening display\n");
        return 1;
    }

    XMatchVisualInfo(display, DefaultScreen(display), 32, TrueColor, &visual_info);

    XSetWindowAttributes window_attributes;
    window_attributes.colormap = XCreateColormap(display, DefaultRootWindow(display), visual_info.visual, AllocNone);
    window_attributes.border_pixel = 0;
    window_attributes.background_pixel = 0;

    window = XCreateWindow(display, DefaultRootWindow(display),
        0, 0, width, height, 0,
        visual_info.depth,
        InputOutput,
        visual_info.visual,
        CWColormap | CWBorderPixel | CWBackPixel,
        &window_attributes);

    XSetStandardProperties(display, window, title, title, None, NULL, 0, NULL);

    graphic_context = XCreateGC(display, window, 0, 0);

    XSizeHints size_hints;
    size_hints.flags = PMinSize | PMaxSize;
    size_hints.min_width = window_width;
    size_hints.min_height = window_height;
    size_hints.max_width = window_width;
    size_hints.max_height = window_height;
    XSetWMNormalHints(display, window, &size_hints);

    XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask);

    delete_window_atom = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, window, &delete_window_atom, 1);

    XMapWindow(display, window);
    return 0;
}

void draw_buffer(){
    XImage* image = XCreateImage(display, visual_info.visual, 32, ZPixmap, 0,
        (char*)buffer, window_width, window_height, 32, 0);

    if(image == NULL){
        fprintf(stderr, "XCreateImage: Error creating image\n");
        return;
    }

    XPutImage(display, window, graphic_context, image, 0, 0, 0, 0, window_width, window_height);
}

int event_loop(){
    XEvent event;
    XNextEvent(display, &event);

    if(event.type == ClientMessage){
        if(event.xclient.data.l[0] == delete_window_atom){
            return 0;
        }
    }

    draw_buffer();

    return 1;
}

void close_window(){
    XFreeGC(display, graphic_context);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

#endif

#endif