/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */

#include "session-component.h"

#include <glib.h>

static void default_components_match_desktop_session_order (void)
{
    g_autoptr(GPtrArray) components = graceful_session_component_list_new_default ();
    const GracefulSessionComponent* ibus = g_ptr_array_index (components, 0);
    const GracefulSessionComponent* rime = g_ptr_array_index (components, 1);
    const GracefulSessionComponent* desktop = g_ptr_array_index (components, 2);
    const GracefulSessionComponent* panel = g_ptr_array_index (components, 3);
    const char* const* argv = NULL;

    g_assert_cmpuint (components->len, ==, 4);

    argv = graceful_session_component_get_argv (ibus);
    g_assert_cmpstr (graceful_session_component_get_name (ibus), ==, "ibus-daemon");
    g_assert_cmpstr (argv[0], ==, "ibus-daemon");
    g_assert_cmpstr (argv[1], ==, "--daemonize");
    g_assert_cmpstr (argv[2], ==, "--xim");
    g_assert_false (graceful_session_component_get_required (ibus));
    g_assert_true (graceful_session_component_get_oneshot (ibus));

    argv = graceful_session_component_get_argv (rime);
    g_assert_cmpstr (graceful_session_component_get_name (rime), ==, "ibus-rime");
    g_assert_cmpstr (argv[0], ==, "ibus");
    g_assert_cmpstr (argv[1], ==, "engine");
    g_assert_cmpstr (argv[2], ==, "rime");
    g_assert_false (graceful_session_component_get_required (rime));
    g_assert_true (graceful_session_component_get_oneshot (rime));

    argv = graceful_session_component_get_argv (desktop);
    g_assert_cmpstr (graceful_session_component_get_name (desktop), ==, "graceful-desktop");
    g_assert_true (g_str_has_suffix (argv[0], "/graceful-desktop"));
    g_assert_true (g_path_is_absolute (argv[0]));
    g_assert_true (graceful_session_component_get_required (desktop));
    g_assert_false (graceful_session_component_get_oneshot (desktop));

    argv = graceful_session_component_get_argv (panel);
    g_assert_cmpstr (graceful_session_component_get_name (panel), ==, "graceful-panel");
    g_assert_true (g_str_has_suffix (argv[0], "/graceful-panel"));
    g_assert_true (g_path_is_absolute (argv[0]));
    g_assert_true (graceful_session_component_get_required (panel));
    g_assert_false (graceful_session_component_get_oneshot (panel));
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/session/component/default-components-match-desktop-session-order",
        default_components_match_desktop_session_order
    );

    return g_test_run ();
}
