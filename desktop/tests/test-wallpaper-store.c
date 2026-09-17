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

#include <glib.h>
#include <glib/gstdio.h>

static void write_empty_file (const char* path)
{
    g_assert_true (g_file_set_contents (path, "", -1, NULL));
}

static gboolean wallpapers_contain (const GPtrArray* wallpapers, const char* path)
{
    guint i = 0;

    for (i = 0; i < wallpapers->len; i++) {
        if (g_strcmp0 (g_ptr_array_index (wallpapers, i), path) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

static void store_loads_only_supported_images (void)
{
    g_autofree char* dir = g_dir_make_tmp ("graceful-wallpaper-store-XXXXXX", NULL);
    g_autofree char* pngPath = g_build_filename (dir, "one.png", NULL);
    g_autofree char* jpgPath = g_build_filename (dir, "two.JPG", NULL);
    g_autofree char* textPath = g_build_filename (dir, "note.txt", NULL);
    g_autoptr(GracefulWallpaperStore) store = NULL;
    const GPtrArray* wallpapers = NULL;

    write_empty_file (pngPath);
    write_empty_file (jpgPath);
    write_empty_file (textPath);

    store = graceful_wallpaper_store_new (dir);
    wallpapers = graceful_wallpaper_store_get_wallpapers (store);

    g_assert_cmpuint (wallpapers->len, ==, 2);
    g_assert_true (wallpapers_contain (wallpapers, pngPath));
    g_assert_true (wallpapers_contain (wallpapers, jpgPath));

    g_unlink (pngPath);
    g_unlink (jpgPath);
    g_unlink (textPath);
    g_rmdir (dir);
}

static void store_reports_empty_directory_selection (void)
{
    g_autofree char* dir = g_dir_make_tmp ("graceful-wallpaper-empty-XXXXXX", NULL);
    g_autoptr(GracefulWallpaperStore) store = graceful_wallpaper_store_new (dir);
    g_autoptr(GError) error = NULL;
    g_autofree char* selected = graceful_wallpaper_store_choose_random (store, &error);

    g_assert_null (selected);
    g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_NOENT);

    g_rmdir (dir);
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/desktop/wallpaper-store/loads-only-supported-images", store_loads_only_supported_images);
    g_test_add_func ("/desktop/wallpaper-store/reports-empty-directory-selection", store_reports_empty_directory_selection);

    return g_test_run ();
}
