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
#include "panel-power-item.h"

#include "panel-power-action.h"

#include <gdk/x11/gdkx.h>
#include <X11/Xatom.h>

#define POWER_MENU_FOCUS_CHECK_INTERVAL_MS 200

struct _GracefulPanelPowerItem
{
    GtkButton parentInstance;

    GtkWidget* menuPopover;
    guint focusCheckTimerId;
    Window allowedActiveWindow;
    Window panelWindow;
};

G_DEFINE_TYPE (GracefulPanelPowerItem, graceful_panel_power_item, GTK_TYPE_BUTTON)

static Display* get_xdisplay (GtkWidget* widget)
{
    GdkDisplay* display = gtk_widget_get_display (widget);

    if (!GDK_IS_X11_DISPLAY (display)) {
        return NULL;
    }

    return gdk_x11_display_get_xdisplay (display);
}

static Window get_active_window (GtkWidget* widget)
{
    Display* display = get_xdisplay (widget);
    Atom activeAtom = None;
    Atom actualType = None;
    int actualFormat = 0;
    unsigned long nitems = 0;
    unsigned long bytesAfter = 0;
    unsigned char* data = NULL;
    Window activeWindow = None;

    if (display == NULL) {
        return None;
    }

    activeAtom = XInternAtom (display, "_NET_ACTIVE_WINDOW", False);
    if (XGetWindowProperty (
            display,
            DefaultRootWindow (display),
            activeAtom,
            0,
            1,
            False,
            XA_WINDOW,
            &actualType,
            &actualFormat,
            &nitems,
            &bytesAfter,
            &data
        ) == Success &&
        data != NULL &&
        actualType == XA_WINDOW &&
        actualFormat == 32 &&
        nitems == 1) {
        activeWindow = *(Window*) data;
    }

    if (data != NULL) {
        XFree (data);
    }

    return activeWindow;
}

static Window get_widget_xwindow (GtkWidget* widget)
{
    GtkNative* native = gtk_widget_get_native (widget);
    GdkSurface* surface = NULL;

    if (native == NULL || get_xdisplay (widget) == NULL) {
        return None;
    }

    surface = gtk_native_get_surface (native);
    if (surface == NULL || !GDK_IS_X11_SURFACE (surface)) {
        return None;
    }

    return gdk_x11_surface_get_xid (surface);
}

static void stop_focus_check (GracefulPanelPowerItem* self)
{
    if (self->focusCheckTimerId != 0) {
        g_source_remove (self->focusCheckTimerId);
        self->focusCheckTimerId = 0;
    }
}

static gboolean on_focus_check_timer (gpointer userData)
{
    GracefulPanelPowerItem* self = GRACEFUL_PANEL_POWER_ITEM (userData);
    Window activeWindow = None;

    if (!gtk_widget_get_mapped (self->menuPopover)) {
        self->focusCheckTimerId = 0;
        return G_SOURCE_REMOVE;
    }

    activeWindow = get_active_window (GTK_WIDGET (self));
    if (activeWindow != None &&
        activeWindow != self->allowedActiveWindow &&
        activeWindow != self->panelWindow) {
        gtk_popover_popdown (GTK_POPOVER (self->menuPopover));
        self->focusCheckTimerId = 0;
        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}

static void start_focus_check (GracefulPanelPowerItem* self)
{
    stop_focus_check (self);
    self->allowedActiveWindow = get_active_window (GTK_WIDGET (self));
    self->panelWindow = get_widget_xwindow (GTK_WIDGET (self));
    self->focusCheckTimerId = g_timeout_add (
        POWER_MENU_FOCUS_CHECK_INTERVAL_MS,
        on_focus_check_timer,
        self
    );
}

static void on_power_popover_closed (GtkPopover* popover, gpointer userData)
{
    stop_focus_check (GRACEFUL_PANEL_POWER_ITEM (userData));
}

static void on_power_action_clicked (GtkButton* button, gpointer userData)
{
    GracefulPanelPowerAction action = GPOINTER_TO_INT (userData);
    GtkWidget* popover = gtk_widget_get_ancestor (GTK_WIDGET (button), GTK_TYPE_POPOVER);
    g_autoptr(GError) error = NULL;

    if (popover != NULL) {
        gtk_popover_popdown (GTK_POPOVER (popover));
    }

    if (!graceful_panel_power_action_run (action, &error)) {
        g_warning ("Failed to run power action: %s", error->message);
    }
}

static GtkWidget* create_power_menu_item (GracefulPanelPowerAction action)
{
    GtkWidget* button = gtk_button_new_with_label (graceful_panel_power_action_get_label (action));

    gtk_widget_add_css_class (button, "panel-power-menu-item");
    g_signal_connect (button, "clicked", G_CALLBACK (on_power_action_clicked), GINT_TO_POINTER (action));

    return button;
}

static GtkWidget* create_power_menu (GtkWidget* parent)
{
    GtkWidget* popover = gtk_popover_new ();
    GtkWidget* box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    gtk_widget_add_css_class (popover, "panel-power-popover");
    gtk_widget_add_css_class (box, "panel-power-menu");
    gtk_box_append (GTK_BOX (box), create_power_menu_item (GRACEFUL_PANEL_POWER_ACTION_SHUTDOWN));
    gtk_box_append (GTK_BOX (box), create_power_menu_item (GRACEFUL_PANEL_POWER_ACTION_REBOOT));
    gtk_box_append (GTK_BOX (box), create_power_menu_item (GRACEFUL_PANEL_POWER_ACTION_LOGOUT));
    gtk_box_append (GTK_BOX (box), create_power_menu_item (GRACEFUL_PANEL_POWER_ACTION_LOCK));
    gtk_popover_set_child (GTK_POPOVER (popover), box);
    gtk_popover_set_autohide (GTK_POPOVER (popover), TRUE);
    gtk_popover_set_has_arrow (GTK_POPOVER (popover), FALSE);
    gtk_popover_set_position (GTK_POPOVER (popover), GTK_POS_TOP);
    gtk_widget_set_parent (popover, parent);

    return popover;
}

static void on_power_button_clicked (GtkButton* button, gpointer userData)
{
    GracefulPanelPowerItem* self = GRACEFUL_PANEL_POWER_ITEM (button);

    gtk_popover_popup (GTK_POPOVER (self->menuPopover));
    start_focus_check (self);
}

static void graceful_panel_power_item_dispose (GObject* object)
{
    GracefulPanelPowerItem* self = GRACEFUL_PANEL_POWER_ITEM (object);

    if (self->menuPopover != NULL) {
        gtk_popover_popdown (GTK_POPOVER (self->menuPopover));
        if (gtk_widget_get_parent (self->menuPopover) != NULL) {
            gtk_widget_unparent (self->menuPopover);
        }
        g_clear_object (&self->menuPopover);
    }
    stop_focus_check (self);

    G_OBJECT_CLASS (graceful_panel_power_item_parent_class)->dispose (object);
}

static void graceful_panel_power_item_class_init (GracefulPanelPowerItemClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_power_item_dispose;
}

static void graceful_panel_power_item_init (GracefulPanelPowerItem* self)
{
    gtk_button_set_icon_name (GTK_BUTTON (self), "system-shutdown-symbolic");
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-icon-button");
    gtk_widget_set_tooltip_text (GTK_WIDGET (self), "Power");
    self->menuPopover = create_power_menu (GTK_WIDGET (self));
    g_object_ref_sink (self->menuPopover);
    g_signal_connect (self->menuPopover, "closed", G_CALLBACK (on_power_popover_closed), self);
    g_signal_connect (self, "clicked", G_CALLBACK (on_power_button_clicked), NULL);
}

GtkWidget* graceful_panel_power_item_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_POWER_ITEM, NULL);
}
