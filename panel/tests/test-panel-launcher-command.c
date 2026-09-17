/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */

#include "panel-launcher-command.h"

#include <glib.h>
#include <glib/gstdio.h>

static void write_executable (const char* path)
{
    g_assert_true (g_file_set_contents (path, "#!/bin/sh\nexit 0\n", -1, NULL));
    g_assert_cmpint (g_chmod (path, 0755), ==, 0);
}

static void launcher_command_resolves_first_available_candidate (void)
{
    const char* candidates[] = { "missing-terminal", "available-terminal", "later-terminal", NULL };
    g_autofree char* directory = g_dir_make_tmp ("graceful-panel-launcher-XXXXXX", NULL);
    g_autofree char* oldPath = g_strdup (g_getenv ("PATH"));
    g_autofree char* newPath = NULL;
    g_autofree char* executable = g_build_filename (directory, "available-terminal", NULL);
    g_autofree char* resolved = NULL;

    write_executable (executable);
    newPath = g_strdup_printf ("%s%s%s", directory, G_SEARCHPATH_SEPARATOR_S, oldPath != NULL ? oldPath : "");
    g_setenv ("PATH", newPath, TRUE);

    resolved = graceful_panel_launcher_command_resolve (candidates);

    g_assert_cmpstr (resolved, ==, "available-terminal");

    if (oldPath != NULL) {
        g_setenv ("PATH", oldPath, TRUE);
    }
    else {
        g_unsetenv ("PATH");
    }
    g_remove (executable);
    g_rmdir (directory);
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/panel/launcher-command/resolves-first-available-candidate",
        launcher_command_resolves_first_available_candidate
    );

    return g_test_run ();
}
