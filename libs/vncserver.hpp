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

class VNCServer
{
    public:
        XDisplay xdisplay;
        VNCServer();
        ~VNCServer();
        void start_service(Websocket &ws);
        void stop_service();
        void send_first_frame(int sd); // send the first frame to the client
    private:
        Display * display;
        Damage damage;
        ScreenInfo screenInfo;
        char config[500] = {0}; // 500byte not too much
        bool isRunning = true;
        bool sendFirstFrame = false;
        int clientSD = 0;
        int damage_event, damage_error, test;
};

VNCServer::VNCServer()
{  
    this->display = xdisplay.getDisplay();
    this->screenInfo = xdisplay.getScreenInfo();
    strcpy(config,xdisplay.getDisplayConfig().c_str());

    test = XDamageQueryExtension(display, &damage_event, &damage_error);
    this->damage = XDamageCreate(display, this->screenInfo.root, XDamageReportRawRectangles);
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
    int sleepDelay = 1000000 / FPS;
    XImage *image;
    XDamageNotifyEvent *dmg_ev = NULL;
    XEvent event;
    while(isRunning)
    {
        usleep(sleepDelay);
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
                ws.sendText(config, this->clientSD);
                ws.sendFrame(info, buffer, infoSize, compressedSize, this->clientSD);
                XDestroyImage(image);
                this->sendFirstFrame = false;
                usleep(1000000);
            }
            else
            {
                XNextEvent(display, &event);
                if (event.type == damage_event + XDamageNotify)
                {
                    dmg_ev = (XDamageNotifyEvent *)&event;
                    auto rect = dmg_ev->area;
                    image = XGetImage(display, this->screenInfo.root, rect.x, rect.y, rect.width, rect.height, AllPlanes, ZPixmap);
                    int frameSize = (rect.height * image->bytes_per_line);
                    int compressedSize = LZ4_compress_default(image->data, buffer, frameSize, bufferSize);
                    std::string data = "UPD" + std::to_string(rect.x) + " " + std::to_string(rect.y) + " " + std::to_string(rect.width) + " " + std::to_string(rect.height) + " " + std::to_string(image->bytes_per_line) + " " + std::to_string(compressedSize) + " \n";
                    char *info = (char *)data.c_str();
                    int infoSize = strlen(info);
                    ws.sendFrame(info, buffer, infoSize, compressedSize);
                    XDestroyImage(image);
                    usleep(3000);
                }
            }
        }
    }
}

#endif