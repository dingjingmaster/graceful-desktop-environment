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

#include "session-environment.h"

#include <glib.h>

static void environment_sets_session_variables (void)
{
    const char* baseEnv[] = { "PATH=/bin", "XDG_CURRENT_DESKTOP=Old", "GTK_MODULES=gail:atk-bridge", NULL };
    g_autoptr(GracefulSessionEnvironment) environment = graceful_session_environment_new ("graceful", "Graceful");
    g_auto(GStrv) envp = graceful_session_environment_build (environment, baseEnv);

    g_assert_cmpstr (g_environ_getenv (envp, "PATH"), ==, "/bin");
    g_assert_cmpstr (g_environ_getenv (envp, "DESKTOP_SESSION"), ==, "graceful");
    g_assert_cmpstr (g_environ_getenv (envp, "GDMSESSION"), ==, "graceful");
    g_assert_cmpstr (g_environ_getenv (envp, "XDG_CURRENT_DESKTOP"), ==, "Graceful");
    g_assert_cmpstr (g_environ_getenv (envp, "XDG_SESSION_TYPE"), ==, "wayland");
    g_assert_cmpstr (g_environ_getenv (envp, "GDK_BACKEND"), ==, "x11");
    g_assert_null (g_environ_getenv (envp, "GTK_MODULES"));
    g_assert_cmpstr (g_environ_getenv (envp, "NO_AT_BRIDGE"), ==, "1");
    g_assert_cmpstr (g_environ_getenv (envp, "GTK_IM_MODULE"), ==, "ibus");
    g_assert_cmpstr (g_environ_getenv (envp, "QT_IM_MODULE"), ==, "ibus");
    g_assert_cmpstr (g_environ_getenv (envp, "XMODIFIERS"), ==, "@im=ibus");
}

static void environment_uses_defaults_for_empty_values (void)
{
    const char* baseEnv[] = { NULL };
    g_autoptr(GracefulSessionEnvironment) environment = graceful_session_environment_new (NULL, NULL);
    g_auto(GStrv) envp = graceful_session_environment_build (environment, baseEnv);

    g_assert_cmpstr (g_environ_getenv (envp, "DESKTOP_SESSION"), ==, "graceful");
    g_assert_cmpstr (g_environ_getenv (envp, "XDG_CURRENT_DESKTOP"), ==, "Graceful");
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/session/environment/sets-session-variables", environment_sets_session_variables);
    g_test_add_func ("/session/environment/uses-defaults-for-empty-values", environment_uses_defaults_for_empty_values);

    return g_test_run ();
}
