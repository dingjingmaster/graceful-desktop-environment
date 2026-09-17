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
        "  background: rgba(18, 24, 31, 0.58);"
        "  box-shadow: inset 0 1px rgba(255, 255, 255, 0.18), inset 0 -1px rgba(0, 0, 0, 0.36);"
        "}"
        ".panel-layout {"
        "  padding: 4px 8px;"
        "}"
        ".panel-menu-button {"
        "  min-width: 34px;"
        "  min-height: 30px;"
        "  border-radius: 7px;"
        "  padding: 0 6px;"
        "  background: rgba(255, 255, 255, 0.06);"
        "}"
        ".panel-menu-logo {"
        "  min-width: 24px;"
        "  min-height: 24px;"
        "}"
        ".panel-icon-button {"
        "  min-width: 30px;"
        "  min-height: 30px;"
        "  padding: 0;"
        "  border-radius: 7px;"
        "  background: transparent;"
        "}"
        ".panel-icon-button:hover, .panel-menu-button:hover, .panel-task-button:hover {"
        "  background: rgba(255, 255, 255, 0.13);"
        "}"
        ".panel-task-area {"
        "  margin: 0 16px;"
        "}"
        ".panel-task-button {"
        "  min-width: 34px;"
        "  min-height: 30px;"
        "  padding: 0 6px;"
        "  border-radius: 7px;"
        "  background: rgba(255, 255, 255, 0.06);"
        "}"
        ".panel-task-preview {"
        "  padding: 10px;"
        "  border-radius: 8px;"
        "  background: rgba(16, 22, 29, 0.92);"
        "  box-shadow: inset 0 1px rgba(255, 255, 255, 0.14);"
        "}"
        ".panel-task-preview-picture {"
        "  border-radius: 6px;"
        "  background: rgba(0, 0, 0, 0.28);"
        "}"
        ".panel-task-preview-title {"
        "  color: alpha(white, 0.90);"
        "  font-weight: 500;"
        "}"
        ".panel-task-preview-fallback-icon {"
        "  color: alpha(white, 0.78);"
        "}"
        ".start-menu-root {"
        "  padding: 18px;"
        "  border-radius: 12px;"
        "  background: rgba(20, 27, 35, 0.94);"
        "  box-shadow: inset 0 1px rgba(255, 255, 255, 0.14);"
        "}"
        ".start-menu-search {"
        "  min-height: 36px;"
        "  border-radius: 8px;"
        "}"
        ".start-menu-section-title {"
        "  color: alpha(white, 0.82);"
        "  font-weight: 700;"
        "}"
        ".start-menu-pinned-grid {"
        "  margin-bottom: 4px;"
        "}"
        ".start-menu-pinned-button {"
        "  min-width: 64px;"
        "  min-height: 70px;"
        "  padding: 8px 6px;"
        "  border-radius: 8px;"
        "  background: rgba(255, 255, 255, 0.06);"
        "}"
        ".start-menu-pinned-button:hover, .start-menu-app-row:hover {"
        "  background: rgba(255, 255, 255, 0.13);"
        "}"
        ".start-menu-app-label, .start-menu-row-label {"
        "  color: alpha(white, 0.90);"
        "}"
        ".start-menu-app-row {"
        "  min-height: 38px;"
        "  padding: 0 10px;"
        "  border-radius: 8px;"
        "  background: transparent;"
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
