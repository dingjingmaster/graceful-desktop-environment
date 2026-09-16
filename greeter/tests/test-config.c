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

#include "greeter-config.h"

#include <glib.h>
#include <glib/gstdio.h>

static void config_reads_background_from_key_file (void)
{
    g_autofree char* configPath = g_build_filename (g_get_tmp_dir (), "graceful-greeter-test.conf", NULL);
    g_autoptr(GracefulGreeterConfig) config = NULL;

    g_assert_true (g_file_set_contents (configPath, "[Greeter]\nBackground=/tmp/background.png\n", -1, NULL));
    g_setenv ("GRACEFUL_GREETER_CONFIG", configPath, TRUE);

    config = graceful_greeter_config_new ();

    g_assert_cmpstr (graceful_greeter_config_get_background (config), ==, "/tmp/background.png");

    g_unsetenv ("GRACEFUL_GREETER_CONFIG");
    g_unlink (configPath);
}

static void config_allows_environment_background_override (void)
{
    g_autofree char* configPath = g_build_filename (g_get_tmp_dir (), "graceful-greeter-test.conf", NULL);
    g_autoptr(GracefulGreeterConfig) config = NULL;

    g_assert_true (g_file_set_contents (configPath, "[Greeter]\nBackground=/tmp/background.png\n", -1, NULL));
    g_setenv ("GRACEFUL_GREETER_CONFIG", configPath, TRUE);
    g_setenv ("GRACEFUL_GREETER_BACKGROUND", "/tmp/override.png", TRUE);

    config = graceful_greeter_config_new ();

    g_assert_cmpstr (graceful_greeter_config_get_background (config), ==, "/tmp/override.png");

    g_unsetenv ("GRACEFUL_GREETER_BACKGROUND");
    g_unsetenv ("GRACEFUL_GREETER_CONFIG");
    g_unlink (configPath);
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/greeter/config/reads-background-from-key-file", config_reads_background_from_key_file);
    g_test_add_func ("/greeter/config/allows-environment-background-override", config_allows_environment_background_override);

    return g_test_run ();
}
