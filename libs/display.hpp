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


// forward declare dix_main from libXvfb.a
extern "C" int dix_main(int argc, char *argv[], char *envp[]);

struct ScreenInfo
{
    int width;
    int height;
    Window root;
};

class XDisplay
{
public:
    ~XDisplay();
    void close();
    Display *getDisplay();
    std::string getDisplayConfig();
    ScreenInfo getScreenInfo();
    std::string getCursorName();
    int getBitPerLine();
    void start();
    void setArg(int argc, char **argv) {
        this->argc = argc;
        this->argv = argv;
    }
private:
    void startXvfb();
    Display *display = 0;
    int bitPerLine = 0;
    std::thread xvfbThread;
    int argc =0;
    char **argv;
};

void XDisplay::start() {
    // get result of command "echo $DISPLAY"
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
            std::cout << "[ERROR][EXIT APP] Could not open display. The built-in X-server will be used !" << std::endl;
        #endif
        startXvfb();

        // retry until Xvfb is ready
        for (int i = 0; i < 50 && this->display == nullptr; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            this->display = XOpenDisplay(":0"); // assume :0
        }

        if (this->display == nullptr)
        {
            std::cerr << "[ERROR] Could not start built-in X-server, exiting app." << std::endl;
            exit(1);
        }
    }
    #if ERROR || DEBUG
        std::cout << "[LOG] Display opened successfully." << std::endl;
    #endif
}

void XDisplay::startXvfb()
{
    int main_argc = this->argc;
    char **orig_argv = this->argv;

    xvfbThread = std::thread([main_argc, orig_argv]() {
        // Copy argv safely
        char **main_argv = new char*[main_argc + 1];
        for (int i = 0; i < main_argc; i++) {
            main_argv[i] = new char[strlen(orig_argv[i]) + 1];
            strcpy(main_argv[i], orig_argv[i]);
        }
        main_argv[main_argc] = nullptr;

        extern char **environ;
        dix_main(main_argc, main_argv, environ); // blocks until server exit

        // Cleanup after dix_main returns
        for (int i = 0; i < main_argc; i++) {
            delete[] main_argv[i];
        }
        delete[] main_argv;
    });

    xvfbThread.detach();
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