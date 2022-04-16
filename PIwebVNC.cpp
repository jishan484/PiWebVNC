#include <iostream>
#include <unistd.h>
#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/Xdamage.h>
#include <signal.h>
#include <lz4.h>
#include "libs/sha1.hpp"
#include "libs/base64.hpp"
#include "libs/websocket.hpp"
#include "libs/display.hpp"
#include "libs/appConfigs.hpp"

using namespace std;

Display *display;            // Global display variable
bool sendFirstScreen = false; // to send first frame [for new connection only]
int socketId = 0;             // socket id for recent connection
int frameCount = 0;           // frame counter to track mismatches in client buffer [not implemented] [not needed]
char config[400] = {0};       // 400byte not too much

/**
 * setup the config string and send it to the client as first message
 */
void setConfig(XImage *img)
{
    string data = "{ 'bytePerLine':" + to_string(img->bytes_per_line) +
                  ",'redMask':" + to_string(img->red_mask) +
                  ",'greenMask':" + to_string(img->green_mask) +
                  ",'blueMask':" + to_string(img->blue_mask) +
                  ",'width':" + to_string(img->width) +
                  ",'height':" + to_string(img->height) +
                  ",'depth':" + to_string(img->depth) + " }";
    strcpy(config, data.c_str());
}

/**
 * main loop to send frames to the client
 */
void frameLoop(Websocket ws)
{
    int damage_event, damage_error, test;
    Window root = DefaultRootWindow(display);
    XWindowAttributes attributes;
    XGetWindowAttributes(display, root, &attributes);
    int Width = attributes.width;
    int Height = attributes.height;
    test = XDamageQueryExtension(display, &damage_event, &damage_error);
    Damage damage = XDamageCreate(display, root, XDamageReportNonEmpty);
    XImage *img = XGetImage(display, root, 0, 0, Width, Height, AllPlanes, ZPixmap);
    int bufferSize = ((Height - 1) * img->bytes_per_line + (Width - 1));
    char buffer[bufferSize] = {0};
    setConfig(img);
    XDestroyImage(img);
    while (1)
    {
        usleep(1000000/FPS);
        if (ws.clients > 0)
        {
            // send refresh screen after eveny 30 frames
            // if(frameCount == 5) { sendFirstScreen = true; } frameCount++;
            if (sendFirstScreen)
            {
                img = XGetImage(display, root, 0, 0, Width, Height, AllPlanes, ZPixmap);
                int frameSize = ((Height - 1) * img->bytes_per_line + (Width - 1));
                int compressedSize = LZ4_compress_default(img->data, buffer, frameSize, bufferSize);
                string data = "UPD" + to_string(0) + " " + to_string(0) + " " + to_string(Width) + " " + to_string(Height) + " " + to_string(img->bytes_per_line) + " " + to_string(compressedSize) + " \n";
                char *info = (char *)data.c_str();
                int infoSize = strlen(info);
                ws.sendText(config, socketId);
                ws.sendFrame(info, buffer, infoSize, compressedSize, socketId);
                XDestroyImage(img);
                sendFirstScreen = false;
                usleep(1000000);
            }
            else
            {
                int partCounts = 0;
                XserverRegion xregion = XFixesCreateRegion(display, NULL, 0);
                XDamageSubtract(display, damage, None, xregion);
                XRectangle *rect = XFixesFetchRegion(display, xregion, &partCounts);
                XFixesDestroyRegion(display, xregion);
                for (int i = 0; i < partCounts; i++)
                {
                    img = XGetImage(display, root, rect[i].x, rect[i].y, rect[i].width, rect[i].height, AllPlanes, ZPixmap);
                    int frameSize = (rect[i].height * img->bytes_per_line);
                    int compressedSize = LZ4_compress_default(img->data, buffer, frameSize, bufferSize);
                    string data = "UPD" + to_string(rect[i].x) + " " + to_string(rect[i].y) + " " + to_string(rect[i].width) + " " + to_string(rect[i].height) + " " + to_string(img->bytes_per_line) + " " + to_string(compressedSize) + " \n";
                    char *info = (char *)data.c_str();
                    int infoSize = strlen(info);
                    ws.sendFrame(info, buffer, infoSize, compressedSize);
                    XDestroyImage(img);
                    usleep(30000);
                }
                XFree(rect);
            }
        }
    }
    XCloseDisplay(display);
}
Websocket *wss;
void firstFrame(int sid)
{
    usleep(100000);
    sendFirstScreen = true;
    socketId = sid;
}
void onMessageCLBK(void *arg, int sid)
{
    char *data = (char *)arg;
    int len = strlen(data);
    int x = 0, y = 0, i = 1, x2 = 0, y2 = 0;
    if (data[0] == 'C')
    {
        while (data[i] != 32 && i < len)
            x = x * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y = y * 10 + data[i++] - 48;
        if (display == 0)
            return;
        XTestFakeMotionEvent(display, -1, x, y, CurrentTime);
        XTestFakeButtonEvent(display, 1, True, CurrentTime);
        XTestFakeButtonEvent(display, 1, False, CurrentTime);
        XFlush(display);
    }
    else if (data[0] == 'M')
    {
        while (data[i] != 32 && i < len)
            x = x * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y = y * 10 + data[i++] - 48;
        if (display == 0)
            return;
        XTestFakeMotionEvent(display, -1, x, y, CurrentTime);
        XFlush(display);
    }
    else if (data[0] == 'R')
    {
        while (data[i] != 32 && i < len)
            x = x * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y = y * 10 + data[i++] - 48;
        if (display == 0)
            return;
        XTestFakeMotionEvent(display, -1, x, y, CurrentTime);
        XTestFakeButtonEvent(display, 3, True, CurrentTime);
        XTestFakeButtonEvent(display, 3, False, CurrentTime);
        XFlush(display);
    }
    else if (data[0] == 'D')
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
        if (display == 0)
            return;
        XTestFakeMotionEvent(display, -1, x, y, 0);
        XTestFakeButtonEvent(display, 1, True, 0);
        XTestFakeMotionEvent(display, -1, x2, y2, 0);
        // usleep(1000000);
        // XTestFakeButtonEvent(display, 1, False, 10);
        XFlush(display);
    }
    else if (data[0] == 'S')
    {
        if (data[1] == 'U')
        {
            XTestFakeButtonEvent(display, 4, 1, 0);
            XTestFakeButtonEvent(display, 4, 0, 70);
        }
        else
        {
            XTestFakeButtonEvent(display, 5, 1, 0);
            XTestFakeButtonEvent(display, 5, 0, 70);
        }
    }
    else if (data[0] == 'K')
    {
        if (data[1] == 49)
        {
            int keycode = XKeysymToKeycode(display, XStringToKeysym(data + 2));
            XTestFakeKeyEvent(display, keycode, True, 0);
            XTestFakeKeyEvent(display, keycode, False, 0);
            XFlush(display);
        }
        else if (data[1] == 50)
        {
            char buffer[15] = {0};
            int i = 2;
            while (data[i] != ' ')
            {
                buffer[i - 2] = data[i];
                i++;
            }
            i++;
            int keycode1 = XKeysymToKeycode(display, XStringToKeysym(buffer));
            int keycode2 = XKeysymToKeycode(display, XStringToKeysym(data + i));
            XTestFakeKeyEvent(display, keycode1, True, 0);
            XTestFakeKeyEvent(display, keycode2, True, 0);
            XTestFakeKeyEvent(display, keycode2, False, 0);
            XTestFakeKeyEvent(display, keycode1, False, 0);
            XFlush(display);
        }
    }
    free(data);
}

void handle_sigint(int sig)
{
    printf("Caught signal %d\n", sig);
    if (wss != NULL)
        wss->stop = true;
    
    exit(0);
}

int main(int argc, char *argv[])
{
    XDisplay xDisplay;
    display = xDisplay.getDisplay();
    Websocket ws;
    wss = &ws;
    signal(SIGINT, handle_sigint);
    std::thread t1 = ws.begin(8000);
    std::thread t2(frameLoop, ws);
    ws.onMessage(onMessageCLBK);
    ws.onConnect(firstFrame);
    t1.join();
    t2.join();
    return 0;
}