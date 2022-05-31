/*
    vncserver.hpp - source code

    =================================
      vnc class for PIwebVNC in C++
    =================================

    Free to use, free to modify, free to redistribute.
    Created by : Jishan Ali Mondal

    This is a header-only library.
    created for only PIwebVNC
    * This code was created entirely for the most optimized performance for PIwebVNC *
    * May not be suitable for other projects *
    version 1.0.1
*/

#ifndef WEBVNC_HPP
#define WEBVNC_HPP

#include <iostream>
#include <unistd.h>
#include <string>
#include <lz4.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/XTest.h>

#include "display.hpp"
#include "input.hpp"

class VNCServer
{
    public:
        XDisplay xdisplay;
        XInputs *inputs;
        VNCServer();
        ~VNCServer();
        void start_service(Websocket &ws);
        void stop_service();
        void send_first_frame(int sd); // send the first frame to the client
    private:
        void threadSleep();
        Display * display;
        Damage damage;
        ScreenInfo screenInfo;
        char config[500] = {0}; // 500 byte not too much
        bool isRunning = true;
        bool sendFirstFrame = false;
        int clientSD = 0;
        int sleepDelay = 1000000 / FPS;
        int sleepLoop = 1000000 / FPS / 100000;
};

VNCServer::VNCServer()
{
    int damage_event, damage_error, test;   
    this->display = xdisplay.getDisplay();
    this->screenInfo = xdisplay.getScreenInfo();
    strcpy(config,xdisplay.getDisplayConfig().c_str());

    test = XDamageQueryExtension(display, &damage_event, &damage_error);
    this->damage = XDamageCreate(display, this->screenInfo.root, XDamageReportNonEmpty);
}

VNCServer::~VNCServer()
{
    XDamageDestroy(display, damage);
    xdisplay.close();
}

void VNCServer::stop_service()
{
    this->isRunning = false;
}
void VNCServer::send_first_frame(int sd)
{
    this->clientSD = sd;
    this->sendFirstFrame = true;
}
void VNCServer::start_service(Websocket &ws)
{
    int bufferSize = (this->screenInfo.height * xdisplay.getBitPerLine()) + (2 * this->screenInfo.width);
    char buffer[bufferSize] = {0};
    XImage *image;
    const XserverRegion xregion = XFixesCreateRegion(this->display, NULL, 0);
    while(isRunning)
    {
        threadSleep();
        if (ws.clients > 0)
        {
            if (this->sendFirstFrame)
            {
                image = XGetImage(this->display, this->screenInfo.root, 0, 0, this->screenInfo.width
                    , this->screenInfo.height, AllPlanes, ZPixmap);
                int frameSize = ((this->screenInfo.height - 1) * image->bytes_per_line + (this->screenInfo.width - 1));
                int compressedSize = LZ4_compress_default(image->data, buffer, frameSize, bufferSize);
                std::string data = "UPD" + std::to_string(0) + " " + std::to_string(0) + " " 
                    + std::to_string(this->screenInfo.width) + " " + std::to_string(this->screenInfo.height) + " " 
                    + std::to_string(image->bytes_per_line) + " " + std::to_string(compressedSize) + " \n";
                char *info = (char *)data.c_str();
                int infoSize = strlen(info);
                XDestroyImage(image);
                ws.sendText(config, this->clientSD);
                ws.sendFrame(info, buffer, infoSize, compressedSize, this->clientSD);
                this->sendFirstFrame = false;
                usleep(100000);
            }
            else
            {
                int partCounts = 0;
                XDamageSubtract(this->display, this->damage, None, xregion);
                XRectangle *rect = XFixesFetchRegion(this->display, xregion, &partCounts);
                for (int i = 0; i < partCounts; i++)
                {
                    image = XGetImage(display, this->screenInfo.root, rect[i].x, rect[i].y, rect[i].width, rect[i].height, AllPlanes, ZPixmap);
                    int frameSize = (rect[i].height * image->bytes_per_line);
                    int compressedSize = LZ4_compress_default(image->data, buffer, frameSize, bufferSize);
                    std::string data = "UPD" + std::to_string(rect[i].x) + " " + std::to_string(rect[i].y) + " " 
                        + std::to_string(rect[i].width) + " " + std::to_string(rect[i].height) + " " 
                        + std::to_string(image->bytes_per_line) + " " + std::to_string(compressedSize) + " \n";
                    char *info = (char *)data.c_str();
                    int infoSize = strlen(info);
                    XDestroyImage(image);
                    ws.sendFrame(info, buffer, infoSize, compressedSize);
                }
                XFree(rect);
            }
        }
    }
}

void VNCServer::threadSleep()
{
    // loop and sleep for 50ms
    int i = this->sleepLoop;
    while(i--)
    {
        inputs->dispatchEvents();
        usleep(100000);
    }
}

#endif