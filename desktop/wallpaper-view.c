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
#include "wallpaper-view.h"

#include "wallpaper-transition.h"

#define WALLPAPER_TRANSITION_DURATION_MS 1200

struct _GracefulWallpaperView
{
    GtkWidget parentInstance;

    GdkPaintable* currentPaintable;
    GdkPaintable* previousPaintable;
    GracefulWallpaperTransition* transition;
    guint tickCallbackId;
    gint64 lastFrameTime;
};

G_DEFINE_TYPE (GracefulWallpaperView, graceful_wallpaper_view, GTK_TYPE_WIDGET)

static void snapshot_cover_paintable (GtkSnapshot* snapshot, GdkPaintable* paintable, double width, double height)
{
    int naturalWidth = 0;
    int naturalHeight = 0;
    double scale = 1.0;
    double renderWidth = width;
    double renderHeight = height;
    double x = 0.0;
    double y = 0.0;

    if (paintable == NULL || width <= 0.0 || height <= 0.0) {
        return;
    }

    naturalWidth = gdk_paintable_get_intrinsic_width (paintable);
    naturalHeight = gdk_paintable_get_intrinsic_height (paintable);

    if (naturalWidth > 0 && naturalHeight > 0) {
        scale = MAX (width / naturalWidth, height / naturalHeight);
        renderWidth = naturalWidth * scale;
        renderHeight = naturalHeight * scale;
        x = (width - renderWidth) / 2.0;
        y = (height - renderHeight) / 2.0;
    }

    gtk_snapshot_save (snapshot);
    gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT ((float) x, (float) y));
    gdk_paintable_snapshot (paintable, snapshot, renderWidth, renderHeight);
    gtk_snapshot_restore (snapshot);
}

static gboolean on_view_tick (GtkWidget* widget, GdkFrameClock* frameClock, gpointer userData)
{
    GracefulWallpaperView* self = GRACEFUL_WALLPAPER_VIEW (userData);
    gint64 frameTime = gdk_frame_clock_get_frame_time (frameClock);
    guint deltaMs = 0;

    if (self->lastFrameTime > 0) {
        deltaMs = (guint) ((frameTime - self->lastFrameTime) / 1000);
    }

    self->lastFrameTime = frameTime;
    graceful_wallpaper_transition_advance (self->transition, deltaMs);
    gtk_widget_queue_draw (widget);

    if (!graceful_wallpaper_transition_get_running (self->transition)) {
        g_clear_object (&self->previousPaintable);
        self->tickCallbackId = 0;
        self->lastFrameTime = 0;
        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}

static void graceful_wallpaper_view_snapshot (GtkWidget* widget, GtkSnapshot* snapshot)
{
    GracefulWallpaperView* self = GRACEFUL_WALLPAPER_VIEW (widget);
    double width = gtk_widget_get_width (widget);
    double height = gtk_widget_get_height (widget);
    double progress = graceful_wallpaper_transition_get_progress (self->transition);

    snapshot_cover_paintable (snapshot, self->previousPaintable, width, height);

    if (self->currentPaintable != NULL) {
        gtk_snapshot_push_opacity (snapshot, progress);
        snapshot_cover_paintable (snapshot, self->currentPaintable, width, height);
        gtk_snapshot_pop (snapshot);
    }
}

static void graceful_wallpaper_view_dispose (GObject* object)
{
    GracefulWallpaperView* self = GRACEFUL_WALLPAPER_VIEW (object);

    if (self->tickCallbackId != 0) {
        gtk_widget_remove_tick_callback (GTK_WIDGET (self), self->tickCallbackId);
        self->tickCallbackId = 0;
    }

    g_clear_object (&self->currentPaintable);
    g_clear_object (&self->previousPaintable);
    g_clear_object (&self->transition);

    G_OBJECT_CLASS (graceful_wallpaper_view_parent_class)->dispose (object);
}

static void graceful_wallpaper_view_class_init (GracefulWallpaperViewClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);
    GtkWidgetClass* widgetClass = GTK_WIDGET_CLASS (klass);

    objectClass->dispose = graceful_wallpaper_view_dispose;
    widgetClass->snapshot = graceful_wallpaper_view_snapshot;
}

static void graceful_wallpaper_view_init (GracefulWallpaperView* self)
{
    self->transition = graceful_wallpaper_transition_new (WALLPAPER_TRANSITION_DURATION_MS);
}

GtkWidget* graceful_wallpaper_view_new (void)
{
    return g_object_new (GRACEFUL_TYPE_WALLPAPER_VIEW, NULL);
}

void graceful_wallpaper_view_set_paintable (GracefulWallpaperView* view, GdkPaintable* paintable)
{
    g_return_if_fail (GRACEFUL_IS_WALLPAPER_VIEW (view));

    if (view->tickCallbackId != 0) {
        gtk_widget_remove_tick_callback (GTK_WIDGET (view), view->tickCallbackId);
        view->tickCallbackId = 0;
    }

    g_set_object (&view->previousPaintable, view->currentPaintable);
    g_set_object (&view->currentPaintable, paintable);

    if (view->previousPaintable != NULL && view->currentPaintable != NULL) {
        graceful_wallpaper_transition_start (view->transition);
        view->lastFrameTime = 0;
        view->tickCallbackId = gtk_widget_add_tick_callback (GTK_WIDGET (view), on_view_tick, view, NULL);
    }
    else {
        g_clear_object (&view->previousPaintable);
        graceful_wallpaper_transition_start (view->transition);
        graceful_wallpaper_transition_advance (view->transition, WALLPAPER_TRANSITION_DURATION_MS);
    }

    gtk_widget_queue_draw (GTK_WIDGET (view));
}
