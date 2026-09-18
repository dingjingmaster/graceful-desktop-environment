/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software.
 */

#include "session-launcher.h"

#include <glib.h>

static void launcher_builds_mutter_display_server_command (void)
{
    g_auto(GStrv) argv = graceful_session_launcher_build_mutter_argv ("/usr/bin/graceful-session");

    g_assert_cmpstr (argv[0], ==, "mutter");
    g_assert_cmpstr (argv[1], ==, "--wayland");
    g_assert_cmpstr (argv[2], ==, "--display-server");
    g_assert_cmpstr (argv[3], ==, "--");
    g_assert_cmpstr (argv[4], ==, "/usr/bin/graceful-session");
    g_assert_null (argv[5]);
}

static void launcher_uses_installed_session_path_by_default (void)
{
    g_auto(GStrv) argv = graceful_session_launcher_build_mutter_argv (NULL);

    g_assert_true (g_str_has_suffix (argv[4], "/graceful-session"));
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/session/launcher/builds-mutter-display-server-command",
        launcher_builds_mutter_display_server_command
    );
    g_test_add_func (
        "/session/launcher/uses-installed-session-path-by-default",
        launcher_uses_installed_session_path_by_default
    );

    return g_test_run ();
}
