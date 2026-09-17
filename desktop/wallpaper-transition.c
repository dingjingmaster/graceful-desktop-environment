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
#include "wallpaper-transition.h"

enum
{
    PROP_0,
    PROP_DURATION_MS,
    N_PROPERTIES
};

struct _GracefulWallpaperTransition
{
    GObject parentInstance;

    guint durationMs;
    guint elapsedMs;
    gboolean running;
};

static GParamSpec* gsProperties[N_PROPERTIES] = { NULL };

G_DEFINE_TYPE (GracefulWallpaperTransition, graceful_wallpaper_transition, G_TYPE_OBJECT)

static void graceful_wallpaper_transition_set_property (
    GObject* object,
    guint propertyId,
    const GValue* value,
    GParamSpec* pspec
)
{
    GracefulWallpaperTransition* self = GRACEFUL_WALLPAPER_TRANSITION (object);

    switch (propertyId) {
        case PROP_DURATION_MS:
            self->durationMs = g_value_get_uint (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propertyId, pspec);
            break;
    }
}

static void graceful_wallpaper_transition_class_init (GracefulWallpaperTransitionClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->set_property = graceful_wallpaper_transition_set_property;

    gsProperties[PROP_DURATION_MS] = g_param_spec_uint (
        "duration-ms",
        "Duration in milliseconds",
        "Transition duration in milliseconds",
        1,
        G_MAXUINT,
        1200,
        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
    );

    g_object_class_install_properties (objectClass, N_PROPERTIES, gsProperties);
}

static void graceful_wallpaper_transition_init (GracefulWallpaperTransition* self)
{
    self->durationMs = 1200;
    self->elapsedMs = 0;
    self->running = FALSE;
}

GracefulWallpaperTransition* graceful_wallpaper_transition_new (guint durationMs)
{
    return g_object_new (
        GRACEFUL_TYPE_WALLPAPER_TRANSITION,
        "duration-ms",
        durationMs > 0 ? durationMs : 1,
        NULL
    );
}

void graceful_wallpaper_transition_start (GracefulWallpaperTransition* transition)
{
    g_return_if_fail (GRACEFUL_IS_WALLPAPER_TRANSITION (transition));

    transition->elapsedMs = 0;
    transition->running = TRUE;
}

void graceful_wallpaper_transition_advance (GracefulWallpaperTransition* transition, guint deltaMs)
{
    guint remainingMs = 0;

    g_return_if_fail (GRACEFUL_IS_WALLPAPER_TRANSITION (transition));

    if (!transition->running) {
        return;
    }

    remainingMs = transition->durationMs - transition->elapsedMs;
    if (deltaMs >= remainingMs) {
        transition->elapsedMs = transition->durationMs;
        transition->running = FALSE;
        return;
    }

    transition->elapsedMs += deltaMs;
}

double graceful_wallpaper_transition_get_progress (GracefulWallpaperTransition* transition)
{
    g_return_val_if_fail (GRACEFUL_IS_WALLPAPER_TRANSITION (transition), 1.0);

    return (double) transition->elapsedMs / (double) transition->durationMs;
}

gboolean graceful_wallpaper_transition_get_running (GracefulWallpaperTransition* transition)
{
    g_return_val_if_fail (GRACEFUL_IS_WALLPAPER_TRANSITION (transition), FALSE);

    return transition->running;
}
