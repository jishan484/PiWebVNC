// this is for testing. soo ths will be the main app

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

VNCServer vncServer;            // VNC server object
Websocket * wss;               // websocket server object
XInputs * xinputs;            // XInputs object

/**
 * main loop to send frames to the client
 */
void frameLoop(Websocket *ws)
{
    vncServer.start_service(*ws);
}

void firstFrame(int sid)
{
    vncServer.send_first_frame(sid);
}

void onMessageCLBK(void * data , int clientSD)
{
    xinputs->processInputs((char *)data, clientSD);
}

void handle_sigint(int sig)
{
    printf("Caught signal %d\n", sig);
    if (wss != NULL)
        wss->stop = true;
    vncServer.stop_service();
    exit(0);
}

int main(int argc, char *argv[])
{
    Display * display = vncServer.xdisplay.getDisplay();
    Websocket ws;
    XInputs input(display);
    wss = &ws;
    xinputs = &input;
    signal(SIGINT, handle_sigint);
    std::thread t1 = ws.begin(SERVER_PORT);
    std::thread t2(frameLoop, &ws);
    ws.onMessage(onMessageCLBK);
    ws.onConnect(firstFrame);
    t1.join();
    t2.join();
    return 0;
}