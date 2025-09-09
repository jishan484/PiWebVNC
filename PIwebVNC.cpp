/*
    PIwebVNC.cpp - main app

    =================================
           PIwebVNC app in C++
    =================================

    Free to use, free to modify, free to redistribute.
    Created by : Jishan Ali Mondal
    
    Created for only PIwebVNC
    * This code purely developed for PIwebVNC for most optimized performance *
    * May not be suitable for other projects *
    version 1.0.1
*/

#include <iostream>
#include <signal.h>
#include <X11/Xlib.h>
#include "libs/sha1.hpp"
#include "libs/base64.hpp"
#include "libs/websocket.hpp"
#include "libs/display.hpp"
#include "libs/input.hpp"
#include "libs/vncserver.hpp"
#include "libs/appConfigs.hpp"

using namespace std;

VNCServer vncServer; // VNC server object
Websocket *wss;      // websocket server object
XInputs *xinputs;    // XInputs object

/**
 * main loop to send frames to the client
 */
void frameLoop()
{
    vncServer.start_service(*wss);
}

void firstFrame(int sid)
{
    vncServer.send_first_frame(sid);
}

void onMessageCLBK(void *data, int clientSD)
{
    xinputs->queueInputs((char *)data, clientSD);
}

void handle_sigint(int sig)
{
    printf("Caught signal %d\n", sig);
    if (wss != NULL)
        wss->stop = true;
    vncServer.stop_service();
    exit(0);
}
void conLoop()
{
    wss->connections();
}

int main(int argc, char *argv[])
{
    std::cout << "\n[HINT] Open http://localhost:" << SERVER_PORT << " in your browser" << std::endl;
    std::cout << "[HINT] ---OR---" << std::endl;
    std::cout << "[HINT] Open http://Ip-of-PI:" << SERVER_PORT << " from different PC\n"<< std::endl;

    Display *display = vncServer.xdisplay.getDisplay();
    Websocket ws;
    XInputs input(display);
    vncServer.inputs = &input;
    wss = &ws;
    xinputs = &input;
    signal(SIGINT, handle_sigint);
    ws.begin(SERVER_PORT);
    std::thread t1(conLoop);
    std::thread t2(frameLoop);
    ws.onMessage(onMessageCLBK);
    ws.onConnect(firstFrame);
    
    t1.join();
    t2.join();
    return 0;
}
