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

#include <glib.h>

static void config_uses_environment_wallpaper_dir (void)
{
    g_autoptr(GracefulDesktopConfig) config = NULL;

    g_setenv ("GRACEFUL_DESKTOP_WALLPAPER_DIR", "/tmp/wallpapers", TRUE);

    config = graceful_desktop_config_new ();

    g_assert_cmpstr (graceful_desktop_config_get_wallpaper_dir (config), ==, "/tmp/wallpapers");
    g_assert_cmpuint (graceful_desktop_config_get_wallpaper_interval (config), ==, 300);

    g_unsetenv ("GRACEFUL_DESKTOP_WALLPAPER_DIR");
}

static void config_uses_environment_interval (void)
{
    g_autoptr(GracefulDesktopConfig) config = NULL;

    g_setenv ("GRACEFUL_DESKTOP_WALLPAPER_INTERVAL", "42", TRUE);

    config = graceful_desktop_config_new ();

    g_assert_cmpuint (graceful_desktop_config_get_wallpaper_interval (config), ==, 42);

    g_unsetenv ("GRACEFUL_DESKTOP_WALLPAPER_INTERVAL");
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/desktop/config/uses-environment-wallpaper-dir", config_uses_environment_wallpaper_dir);
    g_test_add_func ("/desktop/config/uses-environment-interval", config_uses_environment_interval);

    return g_test_run ();
}
