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
#include "desktop-config.h"

#define GRACEFUL_DESKTOP_DEFAULT_INTERVAL 300

struct _GracefulDesktopConfig
{
    GObject parentInstance;

    char* wallpaperDir;
    guint wallpaperInterval;
};

G_DEFINE_TYPE (GracefulDesktopConfig, graceful_desktop_config, G_TYPE_OBJECT)

static char* normalize_text (const char* text)
{
    char* normalized = NULL;

    if (text == NULL) {
        return NULL;
    }

    normalized = g_strdup (text);
    g_strstrip (normalized);

    if (normalized[0] == '\0') {
        g_free (normalized);
        return NULL;
    }

    return normalized;
}

static char* get_default_wallpaper_dir (void)
{
    const char* picturesDir = g_get_user_special_dir (G_USER_DIRECTORY_PICTURES);

    if (picturesDir != NULL && picturesDir[0] != '\0') {
        return g_build_filename (picturesDir, "Wallpapers", NULL);
    }

    return g_build_filename (g_get_home_dir (), "Pictures", "Wallpapers", NULL);
}

static guint get_interval_from_environment (void)
{
    const char* value = g_getenv ("GRACEFUL_DESKTOP_WALLPAPER_INTERVAL");
    guint64 parsed = 0;
    char* end = NULL;

    if (value == NULL || value[0] == '\0') {
        return GRACEFUL_DESKTOP_DEFAULT_INTERVAL;
    }

    parsed = g_ascii_strtoull (value, &end, 10);
    if (end == value || end == NULL || end[0] != '\0' || parsed == 0 || parsed > G_MAXUINT) {
        return GRACEFUL_DESKTOP_DEFAULT_INTERVAL;
    }

    return (guint) parsed;
}

static void graceful_desktop_config_finalize (GObject* object)
{
    GracefulDesktopConfig* self = GRACEFUL_DESKTOP_CONFIG (object);

    g_free (self->wallpaperDir);

    G_OBJECT_CLASS (graceful_desktop_config_parent_class)->finalize (object);
}

static void graceful_desktop_config_class_init (GracefulDesktopConfigClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->finalize = graceful_desktop_config_finalize;
}

static void graceful_desktop_config_init (GracefulDesktopConfig* self)
{
    self->wallpaperDir = normalize_text (g_getenv ("GRACEFUL_DESKTOP_WALLPAPER_DIR"));
    if (self->wallpaperDir == NULL) {
        self->wallpaperDir = get_default_wallpaper_dir ();
    }

    self->wallpaperInterval = get_interval_from_environment ();
}

GracefulDesktopConfig* graceful_desktop_config_new (void)
{
    return g_object_new (GRACEFUL_TYPE_DESKTOP_CONFIG, NULL);
}

const char* graceful_desktop_config_get_wallpaper_dir (GracefulDesktopConfig* config)
{
    g_return_val_if_fail (GRACEFUL_IS_DESKTOP_CONFIG (config), NULL);

    return config->wallpaperDir;
}

guint graceful_desktop_config_get_wallpaper_interval (GracefulDesktopConfig* config)
{
    g_return_val_if_fail (GRACEFUL_IS_DESKTOP_CONFIG (config), GRACEFUL_DESKTOP_DEFAULT_INTERVAL);

    return config->wallpaperInterval;
}
