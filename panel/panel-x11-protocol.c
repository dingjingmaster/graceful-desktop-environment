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
#include "panel-x11-protocol.h"

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

static void request_wm_state (Display* display, Window window, Atom firstState, Atom secondState)
{
    XEvent event = { 0 };

    event.xclient.type = ClientMessage;
    event.xclient.message_type = intern_atom (display, "_NET_WM_STATE");
    event.xclient.display = display;
    event.xclient.window = window;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 1;
    event.xclient.data.l[1] = (long) firstState;
    event.xclient.data.l[2] = (long) secondState;
    event.xclient.data.l[3] = 1;

    XSendEvent (
        display,
        DefaultRootWindow (display),
        False,
        SubstructureRedirectMask | SubstructureNotifyMask,
        &event
    );
}

void graceful_panel_x11_calculate_bottom_strut (
    GracefulPanelX11Strut* strut,
    int screenWidth,
    int screenHeight,
    int panelHeight
)
{
    int height = CLAMP (panelHeight, 0, screenHeight);

    g_return_if_fail (strut != NULL);

    memset (strut->values, 0, sizeof (strut->values));
    strut->values[3] = (unsigned long) height;
    strut->values[10] = 0;
    strut->values[11] = screenWidth > 0 ? (unsigned long) (screenWidth - 1) : 0;
}

void graceful_panel_x11_calculate_bottom_placement (
    GracefulPanelX11Placement* placement,
    int screenWidth,
    int screenHeight,
    int panelHeight
)
{
    int width = MAX (screenWidth, 1);
    int height = CLAMP (panelHeight, 1, MAX (screenHeight, 1));

    g_return_if_fail (placement != NULL);

    placement->x = 0;
    placement->y = MAX (screenHeight - height, 0);
    placement->width = width;
    placement->height = height;
}

void graceful_panel_x11_apply_dock_window (GtkWindow* window, int panelHeight)
{
    GdkSurface* surface = NULL;
    Display* display = NULL;
    Window xwindow = 0;
    Window root = 0;
    XWindowAttributes rootAttributes = { 0 };
    Atom windowType = 0;
    Atom states[4] = { 0 };
    GracefulPanelX11Strut strut = { { 0 } };
    GracefulPanelX11Placement placement = { 0 };

    g_return_if_fail (GTK_IS_WINDOW (window));

    surface = gtk_native_get_surface (GTK_NATIVE (window));
    if (surface == NULL || !GDK_IS_X11_SURFACE (surface)) {
        return;
    }

    display = GDK_SURFACE_XDISPLAY (surface);
    xwindow = gdk_x11_surface_get_xid (surface);
    root = DefaultRootWindow (display);
    if (XGetWindowAttributes (display, root, &rootAttributes) == 0) {
        return;
    }

    windowType = intern_atom (display, "_NET_WM_WINDOW_TYPE_DOCK");
    set_atom_property (display, xwindow, "_NET_WM_WINDOW_TYPE", &windowType, 1);

    states[0] = intern_atom (display, "_NET_WM_STATE_ABOVE");
    states[1] = intern_atom (display, "_NET_WM_STATE_STICKY");
    states[2] = intern_atom (display, "_NET_WM_STATE_SKIP_TASKBAR");
    states[3] = intern_atom (display, "_NET_WM_STATE_SKIP_PAGER");
    set_atom_property (display, xwindow, "_NET_WM_STATE", states, G_N_ELEMENTS (states));
    request_wm_state (display, xwindow, states[0], states[1]);
    request_wm_state (display, xwindow, states[2], states[3]);

    graceful_panel_x11_calculate_bottom_strut (&strut, rootAttributes.width, rootAttributes.height, panelHeight);
    graceful_panel_x11_calculate_bottom_placement (
        &placement,
        rootAttributes.width,
        rootAttributes.height,
        panelHeight
    );
    gtk_window_set_default_size (window, placement.width, placement.height);
    gtk_widget_set_size_request (GTK_WIDGET (window), placement.width, placement.height);

    XChangeProperty (
        display,
        xwindow,
        intern_atom (display, "_NET_WM_STRUT_PARTIAL"),
        XA_CARDINAL,
        32,
        PropModeReplace,
        (const unsigned char*) strut.values,
        G_N_ELEMENTS (strut.values)
    );
    XChangeProperty (
        display,
        xwindow,
        intern_atom (display, "_NET_WM_STRUT"),
        XA_CARDINAL,
        32,
        PropModeReplace,
        (const unsigned char*) strut.values,
        4
    );
    XMoveResizeWindow (
        display,
        xwindow,
        placement.x,
        placement.y,
        (unsigned int) placement.width,
        (unsigned int) placement.height
    );
    XRaiseWindow (display, xwindow);
    XFlush (display);
}
