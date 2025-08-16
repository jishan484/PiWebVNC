/*
    clipboard.hpp - source code

    =================================
      clipboard class for PIwebVNC in C++
    =================================

    Free to use, free to modify, free to redistribute.
    Created by : Jishan Ali Mondal

    This is a header-only library.
    created for only PIwebVNC
    * This code was created entirely for the most optimized performance for
   PIwebVNC *
    * May not be suitable for other projects *
    version 1.0.1
*/



#ifndef CLIPBOARD_HPP
#define CLIPBOARD_HPP

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string>
#include <thread>
#include <atomic>
#include <iostream>
#include <cstring>
#include <chrono>

class Clipboard {
private:
    Display *display;
    Window   window;
    Atom     XA_CLIPBOARD_;
    Atom     XA_UTF8_;
    std::string offeredText_;
    std::thread eventThread;
    std::atomic<bool> running;

    void eventLoop() {
        while (running) {
            while (XPending(display)) {
                XEvent ev;
                XNextEvent(display, &ev);

                if (ev.type == SelectionRequest) {
                    XSelectionRequestEvent *req = &ev.xselectionrequest;

                    XEvent respond;
                    memset(&respond, 0, sizeof(respond));
                    respond.xselection.type      = SelectionNotify;
                    respond.xselection.display   = req->display;
                    respond.xselection.requestor = req->requestor;
                    respond.xselection.selection = req->selection;
                    respond.xselection.target    = req->target;
                    respond.xselection.time      = req->time;
                    respond.xselection.property  = None;

                    // Handle TARGETS request (list formats we support)
                    if (req->target == XInternAtom(display, "TARGETS", False)) {
                        Atom supported[2] = {
                            XA_UTF8_,
                            XInternAtom(display, "STRING", False)
                        };
                        XChangeProperty(display,
                                        req->requestor,
                                        req->property,
                                        XA_ATOM,
                                        32,
                                        PropModeReplace,
                                        (unsigned char*)supported,
                                        2);
                        respond.xselection.property = req->property;
                    }
                    // Serve actual text
                    else if (req->target == XA_UTF8_ ||
                             req->target == XInternAtom(display, "STRING", False)) {
                        XChangeProperty(display,
                                        req->requestor,
                                        req->property,
                                        req->target,
                                        8,
                                        PropModeReplace,
                                        (unsigned char*)offeredText_.c_str(),
                                        offeredText_.size());
                        respond.xselection.property = req->property;
                    }

                    XSendEvent(display, req->requestor, True, 0, &respond);
                    XFlush(display);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

public:
    Clipboard(Display *dpy) {
        if (!XInitThreads()) {
            std::cerr << "Failed to initialize X11 threads!" << std::endl;
        }

        display = dpy;
        window = XCreateSimpleWindow(display, DefaultRootWindow(display),
                                     0, 0, 1, 1, 0, 0, 0);
        XA_CLIPBOARD_ = XInternAtom(display, "CLIPBOARD", False);
        XA_UTF8_      = XInternAtom(display, "UTF8_STRING", False);

        running = true;
        eventThread = std::thread(&Clipboard::eventLoop, this);
    }

    ~Clipboard() {
        running = false;
        if (eventThread.joinable()) eventThread.join();
        if (window) XDestroyWindow(display, window);
    }

    void setText(const char *text) {
        if (!text) return;
        offeredText_ = text;

        // Claim ownership of CLIPBOARD
        XSetSelectionOwner(display, XA_CLIPBOARD_, window, CurrentTime);
        XFlush(display);

        if (XGetSelectionOwner(display, XA_CLIPBOARD_) != window) {
            std::cerr << "Clipboard: failed to become owner" << std::endl;
        }
    }

    std::string getText() {
        Atom prop = XInternAtom(display, "XSEL_DATA", False);
        XConvertSelection(display, XA_CLIPBOARD_, XA_UTF8_, prop, window, CurrentTime);
        XFlush(display);

        XEvent ev;
        XNextEvent(display, &ev);

        if (ev.type == SelectionNotify && ev.xselection.property != None) {
            Atom type;
            int format;
            unsigned long nitems, bytes_after;
            unsigned char *data = nullptr;

            XGetWindowProperty(display, window, prop, 0, ~0, False, AnyPropertyType,
                               &type, &format, &nitems, &bytes_after, &data);
            if (data) {
                std::string result((char*)data, nitems);
                XFree(data);
                return result;
            }
        }
        return "";
    }
};

#endif
