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
#include "desktop-app.h"

#include "desktop-config.h"
#include "desktop-x11-protocol.h"
#include "wallpaper-view.h"
#include "wallpaper-store.h"

#include <gtk/gtk.h>

#define DESKTOP_PROTOCOL_SYNC_INTERVAL_MS 500

struct _GracefulDesktopApp
{
    GObject parentInstance;

    GtkApplication* gtkApp;
    GracefulDesktopConfig* config;
    GracefulWallpaperStore* wallpaperStore;
    GtkWidget* window;
    GtkWidget* wallpaperView;
    guint wallpaperTimerId;
    guint protocolSyncTimerId;
};

G_DEFINE_TYPE (GracefulDesktopApp, graceful_desktop_app, G_TYPE_OBJECT)

static void show_empty_background (GracefulDesktopApp* self)
{
    graceful_wallpaper_view_set_paintable (GRACEFUL_WALLPAPER_VIEW (self->wallpaperView), NULL);
}

static gboolean set_random_wallpaper (GracefulDesktopApp* self)
{
    g_autoptr(GError) error = NULL;
    g_autofree char* wallpaperPath = NULL;
    g_autoptr(GFile) file = NULL;
    g_autoptr(GdkTexture) texture = NULL;

    graceful_wallpaper_store_reload (self->wallpaperStore);
    wallpaperPath = graceful_wallpaper_store_choose_random (self->wallpaperStore, &error);
    if (wallpaperPath == NULL) {
        g_warning ("%s", error->message);
        show_empty_background (self);
        return G_SOURCE_CONTINUE;
    }

    file = g_file_new_for_path (wallpaperPath);
    texture = gdk_texture_new_from_file (file, &error);
    if (texture == NULL) {
        g_warning ("Failed to load wallpaper %s: %s", wallpaperPath, error->message);
        show_empty_background (self);
        return G_SOURCE_CONTINUE;
    }

    graceful_wallpaper_view_set_paintable (GRACEFUL_WALLPAPER_VIEW (self->wallpaperView), GDK_PAINTABLE (texture));

    return G_SOURCE_CONTINUE;
}

static gboolean on_wallpaper_timer (gpointer userData)
{
    return set_random_wallpaper (GRACEFUL_DESKTOP_APP (userData));
}

static void apply_desktop_protocol (GracefulDesktopApp* self)
{
    if (self->window != NULL) {
        graceful_desktop_x11_apply_desktop_window (GTK_WINDOW (self->window));
    }
}

static gboolean on_desktop_window_apply_protocol_idle (gpointer userData)
{
    GracefulDesktopApp* self = GRACEFUL_DESKTOP_APP (userData);

    apply_desktop_protocol (self);

    return G_SOURCE_REMOVE;
}

static gboolean on_desktop_protocol_sync_timer (gpointer userData)
{
    GracefulDesktopApp* self = GRACEFUL_DESKTOP_APP (userData);

    apply_desktop_protocol (self);

    return G_SOURCE_CONTINUE;
}

static void queue_desktop_protocol_sync (GracefulDesktopApp* self)
{
    g_idle_add_full (
        G_PRIORITY_DEFAULT_IDLE,
        on_desktop_window_apply_protocol_idle,
        g_object_ref (self),
        g_object_unref
    );
}

static void on_desktop_window_realize (GtkWidget* widget, gpointer userData)
{
    queue_desktop_protocol_sync (GRACEFUL_DESKTOP_APP (userData));
}

static void on_desktop_window_map (GtkWidget* widget, gpointer userData)
{
    queue_desktop_protocol_sync (GRACEFUL_DESKTOP_APP (userData));
}

static void setup_window_style (void)
{
    GtkCssProvider* provider = gtk_css_provider_new ();

    gtk_css_provider_load_from_string (
        provider,
        "window.graceful-desktop-window { background: #000000; }"
        "widget.graceful-desktop-background { background: #000000; }"
    );
    gtk_style_context_add_provider_for_display (
        gdk_display_get_default (),
        GTK_STYLE_PROVIDER (provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref (provider);
}

static void graceful_desktop_app_activate (GtkApplication* gtkApp, gpointer userData)
{
    GracefulDesktopApp* self = GRACEFUL_DESKTOP_APP (userData);
    guint interval = 0;

    setup_window_style ();

    self->window = gtk_application_window_new (gtkApp);
    gtk_window_set_title (GTK_WINDOW (self->window), "Graceful Desktop");
    gtk_window_set_decorated (GTK_WINDOW (self->window), FALSE);
    gtk_window_set_default_size (GTK_WINDOW (self->window), 1, 1);
    gtk_widget_add_css_class (self->window, "graceful-desktop-window");
    g_signal_connect (self->window, "realize", G_CALLBACK (on_desktop_window_realize), self);
    g_signal_connect (self->window, "map", G_CALLBACK (on_desktop_window_map), self);

    self->wallpaperView = graceful_wallpaper_view_new ();
    gtk_widget_set_hexpand (self->wallpaperView, TRUE);
    gtk_widget_set_vexpand (self->wallpaperView, TRUE);
    gtk_widget_add_css_class (self->wallpaperView, "graceful-desktop-background");
    gtk_window_set_child (GTK_WINDOW (self->window), self->wallpaperView);

    set_random_wallpaper (self);

    interval = graceful_desktop_config_get_wallpaper_interval (self->config);
    self->wallpaperTimerId = g_timeout_add_seconds (interval, on_wallpaper_timer, self);
    self->protocolSyncTimerId = g_timeout_add (
        DESKTOP_PROTOCOL_SYNC_INTERVAL_MS,
        on_desktop_protocol_sync_timer,
        self
    );

    gtk_window_present (GTK_WINDOW (self->window));
}

static void graceful_desktop_app_finalize (GObject* object)
{
    GracefulDesktopApp* self = GRACEFUL_DESKTOP_APP (object);

    if (self->wallpaperTimerId != 0) {
        g_source_remove (self->wallpaperTimerId);
    }
    if (self->protocolSyncTimerId != 0) {
        g_source_remove (self->protocolSyncTimerId);
    }

    g_clear_object (&self->gtkApp);
    g_clear_object (&self->config);
    g_clear_object (&self->wallpaperStore);

    G_OBJECT_CLASS (graceful_desktop_app_parent_class)->finalize (object);
}

static void graceful_desktop_app_class_init (GracefulDesktopAppClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->finalize = graceful_desktop_app_finalize;
}

static void graceful_desktop_app_init (GracefulDesktopApp* self)
{
    self->config = graceful_desktop_config_new ();
    self->wallpaperStore = graceful_wallpaper_store_new (graceful_desktop_config_get_wallpaper_dir (self->config));
    self->gtkApp = gtk_application_new ("org.graceful.desktop", G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect (self->gtkApp, "activate", G_CALLBACK (graceful_desktop_app_activate), self);
}

GracefulDesktopApp* graceful_desktop_app_new (void)
{
    return g_object_new (GRACEFUL_TYPE_DESKTOP_APP, NULL);
}

int graceful_desktop_app_run (GracefulDesktopApp* app, int argc, char** argv)
{
    g_return_val_if_fail (GRACEFUL_IS_DESKTOP_APP (app), 1);

    return g_application_run (G_APPLICATION (app->gtkApp), argc, argv);
}
