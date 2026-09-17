/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "desktop-x11-protocol.h"

#include <gdk/x11/gdkx.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

static Atom intern_atom (Display* display, const char* name)
{
    return XInternAtom (display, name, False);
}

static void set_atom_property (Display* display, Window window, const char* propertyName, const Atom* values, int valueCount)
{
    XChangeProperty (
        display,
        window,
        intern_atom (display, propertyName),
        XA_ATOM,
        32,
        PropModeReplace,
        (const unsigned char*) values,
        valueCount
    );
}

void graceful_desktop_x11_apply_desktop_window (GtkWindow* window)
{
    GdkSurface* surface = NULL;
    Display* display = NULL;
    Window xwindow = 0;
    Window root = 0;
    XWindowAttributes rootAttributes = { 0 };
    Atom windowType = 0;
    Atom states[4] = { 0 };

    g_return_if_fail (GTK_IS_WINDOW (window));

    surface = gtk_native_get_surface (GTK_NATIVE (window));
    if (surface == NULL || !GDK_IS_X11_SURFACE (surface)) {
        return;
    }

    display = GDK_SURFACE_XDISPLAY (surface);
    xwindow = gdk_x11_surface_get_xid (surface);
    root = DefaultRootWindow (display);

    windowType = intern_atom (display, "_NET_WM_WINDOW_TYPE_DESKTOP");
    set_atom_property (display, xwindow, "_NET_WM_WINDOW_TYPE", &windowType, 1);

    states[0] = intern_atom (display, "_NET_WM_STATE_BELOW");
    states[1] = intern_atom (display, "_NET_WM_STATE_STICKY");
    states[2] = intern_atom (display, "_NET_WM_STATE_SKIP_TASKBAR");
    states[3] = intern_atom (display, "_NET_WM_STATE_SKIP_PAGER");
    set_atom_property (display, xwindow, "_NET_WM_STATE", states, G_N_ELEMENTS (states));

    if (XGetWindowAttributes (display, root, &rootAttributes) != 0) {
        XMoveResizeWindow (
            display,
            xwindow,
            0,
            0,
            (unsigned int) rootAttributes.width,
            (unsigned int) rootAttributes.height
        );
    }

    XLowerWindow (display, xwindow);
    XFlush (display);
}
