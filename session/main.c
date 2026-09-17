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
#include "session-manager.h"

#include <glib.h>
#include <locale.h>

static GStrv graceful_session_build_search_dirs (void)
{
    const char* const* configDirs = NULL;
    const char* const* dataDirs = NULL;
    GPtrArray* dirs = g_ptr_array_new_with_free_func (g_free);
    const char* configHome = g_get_user_config_dir ();

    if (configHome != NULL) {
        g_ptr_array_add (dirs, g_build_filename (configHome, "graceful-session", "sessions", NULL));
    }

    configDirs = g_get_system_config_dirs ();
    for (gsize i = 0; configDirs != NULL && configDirs[i] != NULL; i++) {
        g_ptr_array_add (dirs, g_build_filename (configDirs[i], "graceful-session", "sessions", NULL));
    }

    dataDirs = g_get_system_data_dirs ();
    for (gsize i = 0; dataDirs != NULL && dataDirs[i] != NULL; i++) {
        g_ptr_array_add (dirs, g_build_filename (dataDirs[i], "graceful-session", "sessions", NULL));
    }

    g_ptr_array_add (dirs, NULL);

    return (GStrv) g_ptr_array_free (dirs, FALSE);
}

static GStrv graceful_session_build_default_command (void)
{
    const char* command = g_getenv ("GRACEFUL_SESSION_COMMAND");

    if (command != NULL && command[0] != '\0') {
        return g_strsplit (command, " ", -1);
    }

    return NULL;
}

int main (int argc, char* argv[])
{
    g_autoptr(GError) error = NULL;
    g_autoptr(GracefulSessionDefinition) definition = NULL;
    g_autoptr(GracefulSessionManager) manager = NULL;
    g_auto(GStrv) searchDirs = NULL;
    g_auto(GStrv) defaultCommand = NULL;
    g_auto(GStrv) optionCommandArgv = NULL;
    g_autofree char* sessionId = NULL;
    const char* const* commandArgv = NULL;
    GOptionEntry entries[] = {
        { "session", 0, 0, G_OPTION_ARG_STRING, &sessionId, "Session id to load", "SESSION" },
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &optionCommandArgv, "Command to run", "COMMAND" },
        { NULL }
    };
    g_autoptr(GOptionContext) context = NULL;
    gboolean ok = FALSE;

    setlocale (LC_ALL, "");

    context = g_option_context_new ("-- [COMMAND...]");
    g_option_context_add_main_entries (context, entries, NULL);

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_printerr ("graceful-session: %s\n", error->message);
        return 2;
    }

    searchDirs = graceful_session_build_search_dirs ();
    definition = graceful_session_definition_load (sessionId, (const char* const*) searchDirs, &error);
    if (definition == NULL) {
        g_printerr ("graceful-session: %s\n", error->message);
        return 2;
    }

    commandArgv = (const char* const*) optionCommandArgv;
    if (commandArgv == NULL || commandArgv[0] == NULL) {
        defaultCommand = graceful_session_build_default_command ();
        commandArgv = (const char* const*) defaultCommand;
    }

    manager = graceful_session_manager_new (definition, commandArgv);
    ok = graceful_session_manager_run (manager, NULL, &error);
    if (!ok) {
        if (error != NULL) {
            g_printerr ("graceful-session: %s\n", error->message);
            return 1;
        }

        return graceful_session_manager_get_exit_status (manager);
    }

    return 0;
}
