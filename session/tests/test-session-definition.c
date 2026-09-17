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

#include "session-definition.h"

#include <glib.h>
#include <glib/gstdio.h>

static void definition_uses_builtin_graceful_defaults (void)
{
    const char* dirs[] = { "/tmp/graceful-session-test-missing", NULL };
    g_autoptr(GError) error = NULL;
    g_autoptr(GracefulSessionDefinition) definition = graceful_session_definition_load (NULL, dirs, &error);

    g_assert_no_error (error);
    g_assert_nonnull (definition);
    g_assert_cmpstr (graceful_session_definition_get_id (definition), ==, "graceful");
    g_assert_cmpstr (graceful_session_definition_get_name (definition), ==, "Graceful");
    g_assert_false (graceful_session_definition_get_kiosk (definition));
}

static void definition_reads_session_file (void)
{
    g_autofree char* dir = g_dir_make_tmp ("graceful-session-definition-XXXXXX", NULL);
    g_autofree char* sessionPath = g_build_filename (dir, "lab.session", NULL);
    const char* dirs[] = { NULL, NULL };
    g_autoptr(GError) error = NULL;
    g_autoptr(GracefulSessionDefinition) definition = NULL;

    dirs[0] = dir;
    g_assert_true (g_file_set_contents (sessionPath, "[GNOME Session]\nName=Lab Session\nKiosk=true\n", -1, NULL));

    definition = graceful_session_definition_load ("lab", dirs, &error);

    g_assert_no_error (error);
    g_assert_nonnull (definition);
    g_assert_cmpstr (graceful_session_definition_get_id (definition), ==, "lab");
    g_assert_cmpstr (graceful_session_definition_get_name (definition), ==, "Lab Session");
    g_assert_true (graceful_session_definition_get_kiosk (definition));

    g_unlink (sessionPath);
    g_rmdir (dir);
}

static void definition_rejects_missing_named_session (void)
{
    const char* dirs[] = { "/tmp/graceful-session-test-missing", NULL };
    g_autoptr(GError) error = NULL;
    g_autoptr(GracefulSessionDefinition) definition = graceful_session_definition_load ("missing", dirs, &error);

    g_assert_null (definition);
    g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_NOENT);
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/session/definition/uses-builtin-graceful-defaults", definition_uses_builtin_graceful_defaults);
    g_test_add_func ("/session/definition/reads-session-file", definition_reads_session_file);
    g_test_add_func ("/session/definition/rejects-missing-named-session", definition_rejects_missing_named_session);

    return g_test_run ();
}
