/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software.
 */

#include "session-compositor.h"

#include <glib.h>

static void compositor_matches_mutter_wrapping_session (void)
{
    const char* argv[] = {
        "mutter",
        "--wayland",
        "--display-server",
        "--",
        "/usr/bin/graceful-session",
        NULL
    };

    g_assert_true (graceful_session_compositor_argv_matches (argv, "/usr/bin/graceful-session"));
}

static void compositor_rejects_debug_command_parent (void)
{
    const char* argv[] = {
        "bash",
        "-c",
        "/usr/bin/graceful-session -- /usr/bin/env",
        NULL
    };

    g_assert_false (graceful_session_compositor_argv_matches (argv, "/usr/bin/graceful-session"));
}

static void compositor_rejects_unrelated_mutter (void)
{
    const char* argv[] = {
        "mutter",
        "--wayland",
        NULL
    };

    g_assert_false (graceful_session_compositor_argv_matches (argv, "/usr/bin/graceful-session"));
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/session/compositor/matches-mutter-wrapping-session",
        compositor_matches_mutter_wrapping_session
    );
    g_test_add_func (
        "/session/compositor/rejects-debug-command-parent",
        compositor_rejects_debug_command_parent
    );
    g_test_add_func (
        "/session/compositor/rejects-unrelated-mutter",
        compositor_rejects_unrelated_mutter
    );

    return g_test_run ();
}
