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
#include "panel-app.h"

#include "panel-window.h"

#include <gtk/gtk.h>

struct _GracefulPanelApp
{
    GObject parentInstance;

    GtkApplication* gtkApp;
};

G_DEFINE_TYPE (GracefulPanelApp, graceful_panel_app, G_TYPE_OBJECT)

static void setup_panel_style (void)
{
    GtkCssProvider* provider = gtk_css_provider_new ();

    gtk_css_provider_load_from_string (
        provider,
        "window.graceful-panel-window {"
        "  background: alpha(#111820, 0.92);"
        "}"
        ".panel-layout {"
        "  padding: 4px 8px;"
        "}"
        ".panel-menu-button {"
        "  border-radius: 7px;"
        "  padding: 0 12px;"
        "  font-weight: 600;"
        "}"
        ".panel-icon-button {"
        "  min-width: 30px;"
        "  min-height: 30px;"
        "  padding: 0;"
        "  border-radius: 7px;"
        "}"
        ".panel-task-area {"
        "  margin: 0 16px;"
        "}"
        ".panel-muted-label {"
        "  color: alpha(white, 0.46);"
        "}"
        ".panel-clock-item {"
        "  padding: 0 8px;"
        "}"
        ".panel-item-label {"
        "  color: alpha(white, 0.88);"
        "  font-weight: 500;"
        "}"
    );
    gtk_style_context_add_provider_for_display (
        gdk_display_get_default (),
        GTK_STYLE_PROVIDER (provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref (provider);
}

static void graceful_panel_app_activate (GtkApplication* gtkApp, gpointer userData)
{
    GtkWidget* window = NULL;

    setup_panel_style ();

    window = graceful_panel_window_new (gtkApp);
    gtk_window_present (GTK_WINDOW (window));
}

static void graceful_panel_app_dispose (GObject* object)
{
    GracefulPanelApp* self = GRACEFUL_PANEL_APP (object);

    g_clear_object (&self->gtkApp);

    G_OBJECT_CLASS (graceful_panel_app_parent_class)->dispose (object);
}

static void graceful_panel_app_class_init (GracefulPanelAppClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_app_dispose;
}

static void graceful_panel_app_init (GracefulPanelApp* self)
{
    self->gtkApp = gtk_application_new ("org.graceful.panel", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (self->gtkApp, "activate", G_CALLBACK (graceful_panel_app_activate), self);
}

GracefulPanelApp* graceful_panel_app_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_APP, NULL);
}

int graceful_panel_app_run (GracefulPanelApp* app, int argc, char** argv)
{
    g_return_val_if_fail (GRACEFUL_IS_PANEL_APP (app), 1);

    return g_application_run (G_APPLICATION (app->gtkApp), argc, argv);
}
