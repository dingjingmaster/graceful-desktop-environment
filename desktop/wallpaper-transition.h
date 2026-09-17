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
#ifndef GRACEFUL_DESKTOP_WALLPAPER_TRANSITION_H
#define GRACEFUL_DESKTOP_WALLPAPER_TRANSITION_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_WALLPAPER_TRANSITION (graceful_wallpaper_transition_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulWallpaperTransition,
    graceful_wallpaper_transition,
    GRACEFUL,
    WALLPAPER_TRANSITION,
    GObject
)

GracefulWallpaperTransition* graceful_wallpaper_transition_new (guint durationMs);
void graceful_wallpaper_transition_start (GracefulWallpaperTransition* transition);
void graceful_wallpaper_transition_advance (GracefulWallpaperTransition* transition, guint deltaMs);
double graceful_wallpaper_transition_get_progress (GracefulWallpaperTransition* transition);
gboolean graceful_wallpaper_transition_get_running (GracefulWallpaperTransition* transition);

G_END_DECLS

#endif
