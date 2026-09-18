/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#include "panel-xembed-tray-manager.h"

#include "panel-tray-model.h"

#include <gdk/x11/gdkx.h>
#include <glib-unix.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define SYSTEM_TRAY_REQUEST_DOCK 0
#define XEMBED_EMBEDDED_NOTIFY 0
#define XEMBED_WINDOW_ACTIVATE 1
#define XEMBED_ICON_SIZE 16
#define SYSTEM_TRAY_ORIENTATION_HORZ 0

typedef struct _XEmbedTrayWindow XEmbedTrayWindow;

struct _XEmbedTrayWindow
{
    Window trayWindow;
    Window hostWindow;
};

struct _GracefulPanelXEmbedTrayManager
{
    GObject parentInstance;

    Display* display;
    Window managerWindow;
    Atom selectionAtom;
    Atom opcodeAtom;
    Atom managerAtom;
    Atom xembedAtom;
    Atom xembedInfoAtom;
    Atom orientationAtom;
    Atom visualAtom;
    guint eventSourceId;
    GHashTable* trayWindows;
    gboolean started;
};

static GracefulPanelXEmbedTrayManager* gsActiveManager = NULL;
static int gsXErrorCode = 0;

G_DEFINE_TYPE (GracefulPanelXEmbedTrayManager, graceful_panel_xembed_tray_manager, G_TYPE_OBJECT)

static void xembed_tray_window_free (XEmbedTrayWindow* trayWindow)
{
    g_free (trayWindow);
}

static Atom intern_atom (Display* display, const char* name)
{
    return XInternAtom (display, name, False);
}

static int on_x_error (Display* display, XErrorEvent* event)
{
    (void) display;

    gsXErrorCode = event->error_code;
    return 0;
}

static void push_x_error_trap (void)
{
    gsXErrorCode = 0;
    XSetErrorHandler (on_x_error);
}

static gboolean pop_x_error_trap (Display* display)
{
    XSync (display, False);
    XSetErrorHandler (NULL);

    return gsXErrorCode == 0;
}

static gboolean window_has_xembed_info (GracefulPanelXEmbedTrayManager* self, Window window)
{
    Atom actualType = None;
    int actualFormat = 0;
    unsigned long itemCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char* data = NULL;
    int result = 0;
    gboolean hasInfo = FALSE;

    push_x_error_trap ();
    result = XGetWindowProperty (
        self->display,
        window,
        self->xembedInfoAtom,
        0,
        2,
        False,
        self->xembedInfoAtom,
        &actualType,
        &actualFormat,
        &itemCount,
        &bytesAfter,
        &data
    );
    hasInfo = pop_x_error_trap (self->display) &&
        result == Success &&
        actualType == self->xembedInfoAtom &&
        actualFormat == 32 &&
        itemCount >= 2;

    if (data != NULL) {
        XFree (data);
    }

    return hasInfo;
}

static char* read_xwindow_title (Display* display, Window window)
{
    XClassHint classHint = { 0 };
    char* fallbackTitle = NULL;
    char* title = NULL;

    if (XGetClassHint (display, window, &classHint) != 0) {
        if (classHint.res_class != NULL && classHint.res_class[0] != '\0') {
            title = g_strdup (classHint.res_class);
        }
        else if (classHint.res_name != NULL && classHint.res_name[0] != '\0') {
            title = g_strdup (classHint.res_name);
        }

        if (classHint.res_name != NULL) {
            XFree (classHint.res_name);
        }
        if (classHint.res_class != NULL) {
            XFree (classHint.res_class);
        }
    }

    if (title != NULL && title[0] != '\0') {
        return title;
    }

    if (XFetchName (display, window, &fallbackTitle) != 0 && fallbackTitle != NULL) {
        title = g_strdup (fallbackTitle);
        XFree (fallbackTitle);
        return title;
    }

    return g_strdup ("Legacy Tray Item");
}

static void send_manager_message (GracefulPanelXEmbedTrayManager* self)
{
    XClientMessageEvent event = { 0 };
    Window root = DefaultRootWindow (self->display);

    event.type = ClientMessage;
    event.window = root;
    event.message_type = self->managerAtom;
    event.format = 32;
    event.data.l[0] = CurrentTime;
    event.data.l[1] = self->selectionAtom;
    event.data.l[2] = self->managerWindow;
    XSendEvent (self->display, root, False, StructureNotifyMask, (XEvent*) &event);
    XFlush (self->display);
}

static void send_xembed_embedded_notify (GracefulPanelXEmbedTrayManager* self, Window trayWindow, Window hostWindow)
{
    XClientMessageEvent event = { 0 };

    event.type = ClientMessage;
    event.window = trayWindow;
    event.message_type = self->xembedAtom;
    event.format = 32;
    event.data.l[0] = CurrentTime;
    event.data.l[1] = XEMBED_EMBEDDED_NOTIFY;
    event.data.l[2] = hostWindow;
    XSendEvent (self->display, trayWindow, False, NoEventMask, (XEvent*) &event);
    XFlush (self->display);
}

static void send_xembed_window_activate (GracefulPanelXEmbedTrayManager* self, Window trayWindow)
{
    XClientMessageEvent event = { 0 };

    event.type = ClientMessage;
    event.window = trayWindow;
    event.message_type = self->xembedAtom;
    event.format = 32;
    event.data.l[0] = CurrentTime;
    event.data.l[1] = XEMBED_WINDOW_ACTIVATE;
    XSendEvent (self->display, trayWindow, False, NoEventMask, (XEvent*) &event);
    XFlush (self->display);
}

static void set_system_tray_properties (GracefulPanelXEmbedTrayManager* self)
{
    int screen = DefaultScreen (self->display);
    Visual* visual = DefaultVisual (self->display, screen);
    gulong orientation = SYSTEM_TRAY_ORIENTATION_HORZ;
    VisualID visualId = XVisualIDFromVisual (visual);

    XChangeProperty (
        self->display,
        self->managerWindow,
        self->orientationAtom,
        XA_CARDINAL,
        32,
        PropModeReplace,
        (unsigned char*) &orientation,
        1
    );
    XChangeProperty (
        self->display,
        self->managerWindow,
        self->visualAtom,
        XA_VISUALID,
        32,
        PropModeReplace,
        (unsigned char*) &visualId,
        1
    );
}

static void dock_xembed_window (GracefulPanelXEmbedTrayManager* self, Window trayWindow)
{
    XEmbedTrayWindow* info = NULL;
    g_autofree char* title = NULL;

    if (trayWindow == None) {
        return;
    }

    if (g_hash_table_contains (self->trayWindows, (gpointer) (guintptr) trayWindow)) {
        return;
    }

    if (!window_has_xembed_info (self, trayWindow)) {
        return;
    }

    info = g_new0 (XEmbedTrayWindow, 1);
    info->trayWindow = trayWindow;
    info->hostWindow = XCreateSimpleWindow (
        self->display,
        RootWindow (self->display, DefaultScreen (self->display)),
        -100,
        -100,
        XEMBED_ICON_SIZE,
        XEMBED_ICON_SIZE,
        0,
        0,
        0
    );
    title = read_xwindow_title (self->display, trayWindow);
    XSelectInput (self->display, trayWindow, StructureNotifyMask);
    XSelectInput (self->display, info->hostWindow, StructureNotifyMask);
    push_x_error_trap ();
    XReparentWindow (self->display, trayWindow, info->hostWindow, 0, 0);
    XResizeWindow (self->display, trayWindow, XEMBED_ICON_SIZE, XEMBED_ICON_SIZE);
    XMapWindow (self->display, trayWindow);
    if (!pop_x_error_trap (self->display)) {
        XDestroyWindow (self->display, info->hostWindow);
        g_free (info);
        return;
    }
    send_xembed_embedded_notify (self, trayWindow, info->hostWindow);
    send_xembed_window_activate (self, trayWindow);
    g_hash_table_insert (self->trayWindows, (gpointer) (guintptr) trayWindow, info);
    graceful_panel_tray_model_upsert_xembed_item ((guint64) trayWindow, title);
    XFlush (self->display);
}

static Bool is_known_tray_destroy_event (Display* display, XEvent* event, XPointer arg)
{
    GracefulPanelXEmbedTrayManager* self = (GracefulPanelXEmbedTrayManager*) arg;

    return event->type == DestroyNotify &&
        g_hash_table_contains (self->trayWindows, (gpointer) (guintptr) event->xdestroywindow.window);
}

static void remove_xembed_window (GracefulPanelXEmbedTrayManager* self, Window trayWindow)
{
    XEmbedTrayWindow* info = g_hash_table_lookup (self->trayWindows, (gpointer) (guintptr) trayWindow);

    if (info != NULL && info->hostWindow != None) {
        XDestroyWindow (self->display, info->hostWindow);
    }
    g_hash_table_remove (self->trayWindows, (gpointer) (guintptr) trayWindow);
    graceful_panel_tray_model_remove_xembed_item ((guint64) trayWindow);
}

static gboolean process_xembed_events (int fd, GIOCondition condition, gpointer userData)
{
    GracefulPanelXEmbedTrayManager* self = GRACEFUL_PANEL_XEMBED_TRAY_MANAGER (userData);
    XEvent event = { 0 };
    gboolean handled = FALSE;

    while (XCheckTypedWindowEvent (self->display, self->managerWindow, ClientMessage, &event)) {
        if (event.xclient.message_type == self->opcodeAtom &&
            event.xclient.format == 32 &&
            event.xclient.data.l[1] == SYSTEM_TRAY_REQUEST_DOCK) {
            dock_xembed_window (self, (Window) event.xclient.data.l[2]);
        }
        handled = TRUE;
    }

    while (XCheckIfEvent (self->display, &event, is_known_tray_destroy_event, (XPointer) self)) {
        remove_xembed_window (self, event.xdestroywindow.window);
        handled = TRUE;
    }

    return G_SOURCE_CONTINUE;
}

static void graceful_panel_xembed_tray_manager_dispose (GObject* object)
{
    GracefulPanelXEmbedTrayManager* self = GRACEFUL_PANEL_XEMBED_TRAY_MANAGER (object);

    if (self->eventSourceId != 0) {
        g_source_remove (self->eventSourceId);
        self->eventSourceId = 0;
    }

    if (self->display != NULL && self->managerWindow != None) {
        GHashTableIter iter;
        gpointer value = NULL;

        g_hash_table_iter_init (&iter, self->trayWindows);
        while (g_hash_table_iter_next (&iter, NULL, &value)) {
            XEmbedTrayWindow* info = value;

            if (info->hostWindow != None) {
                XDestroyWindow (self->display, info->hostWindow);
                info->hostWindow = None;
            }
        }
        g_hash_table_remove_all (self->trayWindows);
        if (XGetSelectionOwner (self->display, self->selectionAtom) == self->managerWindow) {
            XSetSelectionOwner (self->display, self->selectionAtom, None, CurrentTime);
        }
        XDestroyWindow (self->display, self->managerWindow);
        XFlush (self->display);
        self->managerWindow = None;
    }
    if (self->display != NULL) {
        XCloseDisplay (self->display);
        self->display = NULL;
    }

    G_OBJECT_CLASS (graceful_panel_xembed_tray_manager_parent_class)->dispose (object);
}

static void graceful_panel_xembed_tray_manager_finalize (GObject* object)
{
    GracefulPanelXEmbedTrayManager* self = GRACEFUL_PANEL_XEMBED_TRAY_MANAGER (object);

    g_clear_pointer (&self->trayWindows, g_hash_table_unref);
    if (gsActiveManager == self) {
        gsActiveManager = NULL;
    }

    G_OBJECT_CLASS (graceful_panel_xembed_tray_manager_parent_class)->finalize (object);
}

static void graceful_panel_xembed_tray_manager_class_init (GracefulPanelXEmbedTrayManagerClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_xembed_tray_manager_dispose;
    objectClass->finalize = graceful_panel_xembed_tray_manager_finalize;
}

static void graceful_panel_xembed_tray_manager_init (GracefulPanelXEmbedTrayManager* self)
{
    self->trayWindows = g_hash_table_new_full (
        g_direct_hash,
        g_direct_equal,
        NULL,
        (GDestroyNotify) xembed_tray_window_free
    );
}

GracefulPanelXEmbedTrayManager* graceful_panel_xembed_tray_manager_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_XEMBED_TRAY_MANAGER, NULL);
}

void graceful_panel_xembed_tray_manager_start (GracefulPanelXEmbedTrayManager* self)
{
    int screen = 0;
    g_autofree char* selectionName = NULL;

    g_return_if_fail (GRACEFUL_IS_PANEL_XEMBED_TRAY_MANAGER (self));

    if (self->started) {
        return;
    }

    self->display = XOpenDisplay (NULL);
    if (self->display == NULL) {
        return;
    }

    screen = DefaultScreen (self->display);
    selectionName = g_strdup_printf ("_NET_SYSTEM_TRAY_S%d", screen);
    self->selectionAtom = intern_atom (self->display, selectionName);
    self->opcodeAtom = intern_atom (self->display, "_NET_SYSTEM_TRAY_OPCODE");
    self->managerAtom = intern_atom (self->display, "MANAGER");
    self->xembedAtom = intern_atom (self->display, "_XEMBED");
    self->xembedInfoAtom = intern_atom (self->display, "_XEMBED_INFO");
    self->orientationAtom = intern_atom (self->display, "_NET_SYSTEM_TRAY_ORIENTATION");
    self->visualAtom = intern_atom (self->display, "_NET_SYSTEM_TRAY_VISUAL");
    self->managerWindow = XCreateSimpleWindow (
        self->display,
        RootWindow (self->display, screen),
        -1,
        -1,
        1,
        1,
        0,
        0,
        0
    );

    XSelectInput (self->display, self->managerWindow, StructureNotifyMask);
    set_system_tray_properties (self);
    XSetSelectionOwner (self->display, self->selectionAtom, self->managerWindow, CurrentTime);
    if (XGetSelectionOwner (self->display, self->selectionAtom) != self->managerWindow) {
        XDestroyWindow (self->display, self->managerWindow);
        self->managerWindow = None;
        return;
    }

    send_manager_message (self);
    gsActiveManager = self;
    self->eventSourceId = g_unix_fd_add (
        ConnectionNumber (self->display),
        G_IO_IN,
        process_xembed_events,
        self
    );
    self->started = TRUE;
}

void graceful_panel_xembed_tray_manager_show_item_for_widget (guint64 window, GtkWidget* widget)
{
    XEmbedTrayWindow* info = NULL;
    GtkNative* native = NULL;
    GdkSurface* surface = NULL;
    graphene_rect_t bounds = GRAPHENE_RECT_INIT (0, 0, 0, 0);
    Window surfaceWindow = None;
    int localX = 0;
    int localY = 0;

    if (gsActiveManager == NULL || gsActiveManager->display == NULL || widget == NULL || window == 0) {
        return;
    }

    info = g_hash_table_lookup (gsActiveManager->trayWindows, (gpointer) (guintptr) window);
    if (info == NULL || info->hostWindow == None || !gtk_widget_get_mapped (widget)) {
        return;
    }

    native = gtk_widget_get_native (widget);
    if (native == NULL || !GTK_IS_WIDGET (native)) {
        return;
    }

    surface = gtk_native_get_surface (native);
    if (surface == NULL || !GDK_IS_X11_SURFACE (surface)) {
        return;
    }

    if (!gtk_widget_compute_bounds (widget, GTK_WIDGET (native), &bounds)) {
        return;
    }

    localX = (int) bounds.origin.x + MAX ((int) ((bounds.size.width - XEMBED_ICON_SIZE) / 2.0), 0);
    localY = (int) bounds.origin.y + MAX ((int) ((bounds.size.height - XEMBED_ICON_SIZE) / 2.0), 0);
    surfaceWindow = gdk_x11_surface_get_xid (surface);

    push_x_error_trap ();
    XReparentWindow (gsActiveManager->display, info->hostWindow, surfaceWindow, localX, localY);
    XMoveResizeWindow (gsActiveManager->display, info->hostWindow, localX, localY, XEMBED_ICON_SIZE, XEMBED_ICON_SIZE);
    XResizeWindow (gsActiveManager->display, info->trayWindow, XEMBED_ICON_SIZE, XEMBED_ICON_SIZE);
    XMapWindow (gsActiveManager->display, info->hostWindow);
    XMapWindow (gsActiveManager->display, info->trayWindow);
    pop_x_error_trap (gsActiveManager->display);
    XFlush (gsActiveManager->display);
}

void graceful_panel_xembed_tray_manager_hide_item (guint64 window)
{
    XEmbedTrayWindow* info = NULL;

    if (gsActiveManager == NULL || gsActiveManager->display == NULL || window == 0) {
        return;
    }

    info = g_hash_table_lookup (gsActiveManager->trayWindows, (gpointer) (guintptr) window);
    if (info == NULL || info->hostWindow == None) {
        return;
    }

    XUnmapWindow (gsActiveManager->display, info->hostWindow);
    XFlush (gsActiveManager->display);
}

void graceful_panel_xembed_tray_manager_hide_all (void)
{
    GHashTableIter iter;
    gpointer value = NULL;

    if (gsActiveManager == NULL || gsActiveManager->display == NULL) {
        return;
    }

    g_hash_table_iter_init (&iter, gsActiveManager->trayWindows);
    while (g_hash_table_iter_next (&iter, NULL, &value)) {
        XEmbedTrayWindow* info = value;

        if (info->hostWindow != None) {
            XUnmapWindow (gsActiveManager->display, info->hostWindow);
        }
    }
    XFlush (gsActiveManager->display);
}
