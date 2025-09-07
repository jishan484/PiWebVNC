/*
    input.hpp - source code of

    ==================================================
    Inputs [Mouse, Key, Clipboard] for PIwebVNC in C++
    ==================================================

    Free to use, free to modify, free to redistribute.
    Created by : Jishan Ali Mondal

    This is a header-only library.
    created for only PIwebVNC
    * This code was created entirely for the most optimized performance for PIwebVNC *
    * May not be suitable for other projects *
    version 1.0.0
*/

#ifndef INPUT_HPP
#define INPUT_HPP

#include "clipboard.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/Xdamage.h>
#include <iostream>

class XInputs
{
    public:
        XInputs(Display * display);
        void processInputs(char *data, int clinetSD);
        void queueInputs(char *data, int clinetSD);
        void dispatchEvents();
        Clipboard *clipboard;
    private:
        Display *display;
        char events[30][1024] = {0};
        int eventCount = 0;
};

XInputs::XInputs(Display * display)
{
    this->display = display;
}

void XInputs::queueInputs(char *data, int clinetSD)
{
    if (this->eventCount < 30)
    {
        std::cout << data << std::endl;
        strcpy(this->events[this->eventCount], data);
        this->eventCount++;
    }
}

void XInputs::dispatchEvents(){
    for(int i = 0; i < this->eventCount; i++){
        if (events[i][0] != 0)
        {
            processInputs(this->events[i], 0);
            this->events[i][0] = 0;
        }
    }
    this->eventCount = 0;
}

void XInputs::processInputs(char *data, int clientSD)
{
    int len = strlen(data);
    int x = 0, y = 0, i = 1, x2 = 0, y2 = 0;
    if (data[0] == 'C')  //for click to x y pos
    {
        while (data[i] != 32 && i < len)
            x = x * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y = y * 10 + data[i++] - 48;
        if (this->display == 0)
            return;
        XTestFakeMotionEvent(this->display, DefaultScreen(this->display), x, y, CurrentTime);
        XTestFakeButtonEvent(this->display, 1, True, CurrentTime);
        XTestFakeButtonEvent(this->display, 1, False, CurrentTime);
        XFlush(this->display);
    }
    else if (data[0] == 'M')  //only mouse pointer move to x y pos
    {
        while (data[i] != 32 && i < len)
            x = x * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y = y * 10 + data[i++] - 48;
        if (this->display == 0)
            return;
        XTestFakeMotionEvent(this->display,DefaultScreen(this->display), x, y, CurrentTime);
        XFlush(this->display);
    }
    else if (data[0] == 'R') // right click at x y pos
    {
        while (data[i] != 32 && i < len)
            x = x * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y = y * 10 + data[i++] - 48;
        if (this->display == 0)
            return;
        XTestFakeMotionEvent(this->display, DefaultScreen(this->display), x, y, CurrentTime);
        XTestFakeButtonEvent(this->display, 3, True, CurrentTime);
        XTestFakeButtonEvent(this->display, 3, False, CurrentTime);
        XFlush(this->display);
    }
    else if (data[0] == 'D') // drag from x y pos to x2 y2 pos
    {
        while (data[i] != 32 && i < len)
            x = x * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y = y * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            x2 = x2 * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y2 = y2 * 10 + data[i++] - 48;
        if (this->display == 0)
            return;
        XTestFakeMotionEvent(this->display, DefaultScreen(this->display), x, y, CurrentTime);
        XTestFakeButtonEvent(this->display, 1, True, CurrentTime);
        XTestFakeMotionEvent(this->display, DefaultScreen(this->display), x2, y2, CurrentTime);
        XFlush(this->display);
    }
    else if (data[0] == 'S') // mouse scroll
    {
        if (data[1] == 'U') // scroll uo
        {
            XTestFakeButtonEvent(this->display, 4, 1, 0);
            XTestFakeButtonEvent(this->display, 4, 0, 70);
        }
        else //scroll down
        {
            XTestFakeButtonEvent(this->display, 5, 1, 0);
            XTestFakeButtonEvent(this->display, 5, 0, 70);
        }
    }
    else if (data[0] == 'K') // keyboard key press
    {
        if (data[1] == 49) // single key press
        {
            int keycode = XKeysymToKeycode(this->display, XStringToKeysym(data + 2));
            XTestFakeKeyEvent(this->display, keycode, True, CurrentTime);
            XTestFakeKeyEvent(this->display, keycode, False, CurrentTime);
            XFlush(this->display);
        }
        else if (data[1] == 50) // combo key press
        {
            char buffer[15] = {0};
            int i = 2;
            while (data[i] != ' ')
            {
                buffer[i - 2] = data[i];
                i++;
            }
            i++;
            int keycode1 = XKeysymToKeycode(this->display, XStringToKeysym(buffer));
            int keycode2 = XKeysymToKeycode(this->display, XStringToKeysym(data + i));
            XTestFakeKeyEvent(this->display, keycode1, True, 0);
            XTestFakeKeyEvent(this->display, keycode2, True, 0);
            XTestFakeKeyEvent(this->display, keycode2, False, 0);
            XTestFakeKeyEvent(this->display, keycode1, False, 0);
            XFlush(this->display);
        }
    }
    else if (data[0] == 'P') // paste (ctrl + shift + v)
    {
        this->clipboard->setText(&data[1]);
        int keycode = XKeysymToKeycode(this->display, XStringToKeysym("Control_L"));
        int keycode2 = XKeysymToKeycode(this->display, XStringToKeysym("Shift_L"));
        int keycode3 = XKeysymToKeycode(this->display, XStringToKeysym("v"));
        XTestFakeKeyEvent(this->display, keycode, True, CurrentTime);
        XTestFakeKeyEvent(this->display, keycode2, True, CurrentTime);
        XTestFakeKeyEvent(this->display, keycode3, True, CurrentTime);
        XTestFakeKeyEvent(this->display, keycode3, False, CurrentTime);
        XTestFakeKeyEvent(this->display, keycode2, False, CurrentTime);
        XTestFakeKeyEvent(this->display, keycode, False, CurrentTime);
        XFlush(this->display);
    }
    XFlush(this->display);
}

#endif