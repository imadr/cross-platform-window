#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int window_width, window_height;

#ifdef __linux__

#ifdef X11

#include <X11/Xlib.h>
#include <X11/Xutil.h>

Display* display;
Window window;
Atom delete_window_atom;

int create_window(char* title, int width, int height){
    window_width = width;
    window_height = height;

    display = XOpenDisplay(0);

    if(display == NULL){
        fprintf(stderr, "XOpenDisplay: Error opening display\n");
        return 1;
    }

    XVisualInfo visual_info;
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

int event_loop(){
    XEvent event;
    XNextEvent(display, &event);

    if(event.type == ClientMessage){
        if(event.xclient.data.l[0] == delete_window_atom){
            return 0;
        }
    }

    return 1;
}

void close_window(){
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

#endif

#endif
