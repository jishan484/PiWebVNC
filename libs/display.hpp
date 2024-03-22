/*
    display.hpp - source code

    =================================
    Display class for PIwebVNC in C++
    =================================

    Free to use, free to modify, free to redistribute.
    Created by : Jishan Ali Mondal

    This is a header-only library.
    created for only PIwebVNC
    * This code was created entirely for the most optimized performance for PIwebVNC *
    * May not be suitable for other projects *
    version 1.0.1
*/

#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "appConfigs.hpp"

#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xfixes.h>
#include <string>
#include "websocket.hpp"
#include "xvfb_support.hpp"

struct ScreenInfo
{
    int width;
    int height;
    Window root;
};

class XDisplay
{
public:
    XDisplay();
    ~XDisplay();
    void close();
    Display *getDisplay();
    std::string getDisplayConfig();
    ScreenInfo getScreenInfo();
    std::string getCursorName();
    int getBitPerLine();

private:
    Display *display = 0;
    int bitPerLine = 0;
};

XDisplay::XDisplay()
{
    //get result of command "echo $DISPLAY"
    try
    {
        std::string envDisplay = std::string(getenv("DISPLAY"));
        this->display = XOpenDisplay(envDisplay.c_str());
    }
    catch (std::exception &e)
    {
        #if LOG || DEBUG
            std::cout << "[LOG] No env set for display. Trying default.[0 to 9]." << std::endl;
        #endif
        int displayNumber = 0;
        while (this->display == 0 && displayNumber < 10)
        {
            std::string displayNumberStr = ":" + std::to_string(displayNumber);
            std::cout << "[LOG] Trying display number " << displayNumberStr << std::endl;
            this->display = XOpenDisplay(displayNumberStr.c_str());
            displayNumber++;
        }
    }
    if (this->display == 0)
    {
        #if ERROR || DEBUG
            std::cout << "[ERROR][EXIT APP] Could not open display. Please pass --display [id].\n\t eg: --display 18." << std::endl;
            std::cout << " or " << std::endl;
            installXvfb();
            initXvfb();
#endif
                exit(1);
    }
    #if ERROR || DEBUG
        std::cout << "[LOG] Display opened successfully." << std::endl;
    #endif
}
XDisplay::~XDisplay()
{
    // free display
    if(this->display != 0) {
        #if LOG
            std::cout << "[LOG] Closing display." << std::endl;
        #endif
        XCloseDisplay(this->display);
        this->display = 0;
    }
}

Display * XDisplay::getDisplay()
{
    return this->display;
}

int XDisplay::getBitPerLine()
{
    return this->bitPerLine;
}

std::string XDisplay::getDisplayConfig()
{
    ScreenInfo screenInfo = getScreenInfo();
    int width = screenInfo.width;
    int height = screenInfo.height;
    XImage * img = XGetImage(this->display, DefaultRootWindow(this->display), 0, 0, width, height, AllPlanes, ZPixmap);
    std::string config = "{ 'bytePerLine':" + std::to_string(img->bytes_per_line) +
                         ",'redMask':" + std::to_string(img->red_mask) +
                         ",'greenMask':" + std::to_string(img->green_mask) +
                         ",'blueMask':" + std::to_string(img->blue_mask) +
                         ",'width':" + std::to_string(img->width) +
                         ",'height':" + std::to_string(img->height) +
                         ",'depth':" + std::to_string(img->depth) +
                         ",'fps':" + std::to_string(FPS) +
                         +" }";
    this->bitPerLine = img->bytes_per_line;
    XDestroyImage(img);
    return config;
}

std::string XDisplay::getCursorName()
{
    // get cursor name using xfixes
    XFixesCursorImage *cursorImage = XFixesGetCursorImage(this->display);
    if (cursorImage == 0)
    {
        #if ERROR || DEBUG
            std::cout << "[ERROR][RET NULL] Could not get cursor image." << std::endl;
        #endif
        return "";
    }
    XFree(cursorImage);
    return std::string(cursorImage->name);   
}

ScreenInfo XDisplay::getScreenInfo()
{
    ScreenInfo screenInfo;
    // get screen info from root window
    screenInfo.root = DefaultRootWindow(this->display);
    screenInfo.width = DisplayWidth(this->display, DefaultScreen(this->display));
    screenInfo.height = DisplayHeight(this->display, DefaultScreen(this->display));
    return screenInfo;
}

void XDisplay::close()
{
    this->~XDisplay();
}

#endif

    /*
     * Helps to find correct Display so that it can be start from linux service without providing display env
     * running `echo $DISPLAY` command to get the display
     * otherwise it will loop though all displays and find the one that is running
     * loop limit is set to 10 change from config file (config.cpp)
     */

    /*
    * if display is not set then it will try to start Xvfb
    * and then start the display manager
    * it will try to start the following desktop environments
    * gnome, mate, cinnamon, pantheon, lxqt, kde, xfce, lxsession, enlightenment
    * if any of the above desktop environment is installed then it will start that
    * otherwise it will exit the app
    */