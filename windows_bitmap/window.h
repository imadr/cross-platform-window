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

#endif