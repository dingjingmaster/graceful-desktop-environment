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
#include "panel-window.h"

#include "panel-layout.h"
#include "panel-x11-protocol.h"

#define PANEL_HEIGHT 42
#define PANEL_PROTOCOL_SYNC_INTERVAL_MS 500

struct _GracefulPanelWindow
{
    GtkApplicationWindow parentInstance;

    guint protocolSyncTimerId;
};

G_DEFINE_TYPE (GracefulPanelWindow, graceful_panel_window, GTK_TYPE_APPLICATION_WINDOW)

static void apply_panel_protocol (GracefulPanelWindow* self)
{
    graceful_panel_x11_apply_dock_window (GTK_WINDOW (self), PANEL_HEIGHT);
}

static gboolean on_panel_window_apply_protocol_idle (gpointer userData)
{
    GracefulPanelWindow* self = GRACEFUL_PANEL_WINDOW (userData);

    apply_panel_protocol (self);

    return G_SOURCE_REMOVE;
}

static gboolean on_panel_protocol_sync_timer (gpointer userData)
{
    GracefulPanelWindow* self = GRACEFUL_PANEL_WINDOW (userData);

    apply_panel_protocol (self);

    return G_SOURCE_CONTINUE;
}

static void queue_panel_protocol_sync (GracefulPanelWindow* self)
{
    g_idle_add_full (
        G_PRIORITY_DEFAULT_IDLE,
        on_panel_window_apply_protocol_idle,
        g_object_ref (self),
        g_object_unref
    );
}

static void on_panel_window_realize (GtkWidget* widget, gpointer userData)
{
    GracefulPanelWindow* self = GRACEFUL_PANEL_WINDOW (widget);

    apply_panel_protocol (self);
    queue_panel_protocol_sync (self);
}

static void on_panel_window_map (GtkWidget* widget, gpointer userData)
{
    GracefulPanelWindow* self = GRACEFUL_PANEL_WINDOW (widget);

    apply_panel_protocol (self);
    queue_panel_protocol_sync (self);
}

static void graceful_panel_window_dispose (GObject* object)
{
    GracefulPanelWindow* self = GRACEFUL_PANEL_WINDOW (object);

    if (self->protocolSyncTimerId != 0) {
        g_source_remove (self->protocolSyncTimerId);
        self->protocolSyncTimerId = 0;
    }

    G_OBJECT_CLASS (graceful_panel_window_parent_class)->dispose (object);
}

static void graceful_panel_window_class_init (GracefulPanelWindowClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_window_dispose;
}

static void graceful_panel_window_init (GracefulPanelWindow* self)
{
    GtkWidget* layout = graceful_panel_layout_new ();

    gtk_window_set_title (GTK_WINDOW (self), "Graceful Panel");
    gtk_window_set_decorated (GTK_WINDOW (self), FALSE);
    gtk_window_set_resizable (GTK_WINDOW (self), FALSE);
    gtk_window_set_default_size (GTK_WINDOW (self), 1, PANEL_HEIGHT);
    gtk_widget_set_size_request (GTK_WIDGET (self), -1, PANEL_HEIGHT);
    gtk_widget_add_css_class (GTK_WIDGET (self), "graceful-panel-window");
    gtk_window_set_child (GTK_WINDOW (self), layout);
    g_signal_connect (self, "realize", G_CALLBACK (on_panel_window_realize), NULL);
    g_signal_connect (self, "map", G_CALLBACK (on_panel_window_map), NULL);

    self->protocolSyncTimerId = g_timeout_add (
        PANEL_PROTOCOL_SYNC_INTERVAL_MS,
        on_panel_protocol_sync_timer,
        self
    );
}

GtkWidget* graceful_panel_window_new (GtkApplication* application)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_WINDOW, "application", application, NULL);
}
