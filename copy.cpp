#include <stdio.h>
#include <unistd.h>
#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/Xdamage.h>

using namespace std;

int main()
{
    Display *display = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(display);
    int damage_event, damage_error, count = 0;

    XDamageQueryExtension(display, &damage_event, &damage_error);
    // tried all 4  damage levels giving the same output
    Damage dmg = XDamageCreate(display, root, XDamageReportNonEmpty);
    XDamageNotifyEvent *dmg_ev = NULL;
    XEvent event;
    int i = 0;
    while (1)
    {
        sleep(1);

        XNextEvent(display, &event);

        if (event.type == damage_event + XDamageNotify)
        {
            printf("Got event\n");
            dmg_ev = (XDamageNotifyEvent *)&event;
            printf("[%d] : x=%d y=%d w=%d h=%d\n",i++, dmg_ev->area.x, dmg_ev->area.y, dmg_ev->area.width, dmg_ev->area.height);
            XDamageSubtract(display,dmg_ev->damage,0,0);
            dmg = dmg_ev->damage;
        }
    }
}