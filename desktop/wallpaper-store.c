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
#include "wallpaper-store.h"

#include <gio/gio.h>

enum
{
    PROP_0,
    PROP_WALLPAPER_DIR,
    N_PROPERTIES
};

struct _GracefulWallpaperStore
{
    GObject parentInstance;

    char* wallpaperDir;
    GPtrArray* wallpapers;
};

static GParamSpec* gsProperties[N_PROPERTIES] = { NULL };

G_DEFINE_TYPE (GracefulWallpaperStore, graceful_wallpaper_store, G_TYPE_OBJECT)

static gboolean is_supported_image (const char* name)
{
    g_autofree char* lowerName = NULL;

    if (name == NULL) {
        return FALSE;
    }

    lowerName = g_ascii_strdown (name, -1);

    return g_str_has_suffix (lowerName, ".jpg") ||
        g_str_has_suffix (lowerName, ".jpeg") ||
        g_str_has_suffix (lowerName, ".png") ||
        g_str_has_suffix (lowerName, ".webp") ||
        g_str_has_suffix (lowerName, ".bmp");
}

static gint compare_strings (gconstpointer left, gconstpointer right)
{
    const char* const* leftString = left;
    const char* const* rightString = right;

    return g_strcmp0 (*leftString, *rightString);
}

static void graceful_wallpaper_store_set_property (
    GObject* object,
    guint propertyId,
    const GValue* value,
    GParamSpec* pspec
)
{
    GracefulWallpaperStore* self = GRACEFUL_WALLPAPER_STORE (object);

    switch (propertyId) {
        case PROP_WALLPAPER_DIR:
            self->wallpaperDir = g_value_dup_string (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propertyId, pspec);
            break;
    }
}

static void graceful_wallpaper_store_finalize (GObject* object)
{
    GracefulWallpaperStore* self = GRACEFUL_WALLPAPER_STORE (object);

    g_free (self->wallpaperDir);
    g_clear_pointer (&self->wallpapers, g_ptr_array_unref);

    G_OBJECT_CLASS (graceful_wallpaper_store_parent_class)->finalize (object);
}

static void graceful_wallpaper_store_constructed (GObject* object)
{
    GracefulWallpaperStore* self = GRACEFUL_WALLPAPER_STORE (object);

    G_OBJECT_CLASS (graceful_wallpaper_store_parent_class)->constructed (object);

    graceful_wallpaper_store_reload (self);
}

static void graceful_wallpaper_store_class_init (GracefulWallpaperStoreClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->set_property = graceful_wallpaper_store_set_property;
    objectClass->finalize = graceful_wallpaper_store_finalize;
    objectClass->constructed = graceful_wallpaper_store_constructed;

    gsProperties[PROP_WALLPAPER_DIR] = g_param_spec_string (
        "wallpaper-dir",
        "Wallpaper directory",
        "Directory containing wallpaper images",
        NULL,
        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
    );

    g_object_class_install_properties (objectClass, N_PROPERTIES, gsProperties);
}

static void graceful_wallpaper_store_init (GracefulWallpaperStore* self)
{
    self->wallpapers = g_ptr_array_new_with_free_func (g_free);
}

GracefulWallpaperStore* graceful_wallpaper_store_new (const char* wallpaperDir)
{
    return g_object_new (GRACEFUL_TYPE_WALLPAPER_STORE, "wallpaper-dir", wallpaperDir, NULL);
}

void graceful_wallpaper_store_reload (GracefulWallpaperStore* store)
{
    g_autoptr(GDir) dir = NULL;
    const char* name = NULL;

    g_return_if_fail (GRACEFUL_IS_WALLPAPER_STORE (store));

    g_ptr_array_set_size (store->wallpapers, 0);

    if (store->wallpaperDir == NULL || store->wallpaperDir[0] == '\0') {
        return;
    }

    dir = g_dir_open (store->wallpaperDir, 0, NULL);
    if (dir == NULL) {
        return;
    }

    while ((name = g_dir_read_name (dir)) != NULL) {
        g_autofree char* path = NULL;

        if (!is_supported_image (name)) {
            continue;
        }

        path = g_build_filename (store->wallpaperDir, name, NULL);
        if (g_file_test (path, G_FILE_TEST_IS_REGULAR)) {
            g_ptr_array_add (store->wallpapers, g_steal_pointer (&path));
        }
    }

    g_ptr_array_sort (store->wallpapers, compare_strings);
}

const GPtrArray* graceful_wallpaper_store_get_wallpapers (GracefulWallpaperStore* store)
{
    g_return_val_if_fail (GRACEFUL_IS_WALLPAPER_STORE (store), NULL);

    return store->wallpapers;
}

char* graceful_wallpaper_store_choose_random (GracefulWallpaperStore* store, GError** error)
{
    guint index = 0;

    g_return_val_if_fail (GRACEFUL_IS_WALLPAPER_STORE (store), NULL);

    if (store->wallpapers->len == 0) {
        g_set_error (
            error,
            G_FILE_ERROR,
            G_FILE_ERROR_NOENT,
            "No wallpaper images found in %s",
            store->wallpaperDir != NULL ? store->wallpaperDir : "(null)"
        );
        return NULL;
    }

    index = g_random_int_range (0, (gint32) store->wallpapers->len);

    return g_strdup (g_ptr_array_index (store->wallpapers, index));
}
