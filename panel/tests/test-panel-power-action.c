/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */

#include "panel-power-action.h"

#include <glib.h>

static void power_action_maps_shutdown_to_systemctl_poweroff (void)
{
    g_autoptr(GPtrArray) candidates = graceful_panel_power_action_build_candidates (
        GRACEFUL_PANEL_POWER_ACTION_SHUTDOWN
    );
    GStrv argv = g_ptr_array_index (candidates, 0);

    g_assert_cmpuint (candidates->len, ==, 1);
    g_assert_cmpstr (argv[0], ==, "systemctl");
    g_assert_cmpstr (argv[1], ==, "poweroff");
    g_assert_null (argv[2]);
}

static void power_action_maps_reboot_to_systemctl_reboot (void)
{
    g_autoptr(GPtrArray) candidates = graceful_panel_power_action_build_candidates (
        GRACEFUL_PANEL_POWER_ACTION_REBOOT
    );
    GStrv argv = g_ptr_array_index (candidates, 0);

    g_assert_cmpuint (candidates->len, ==, 1);
    g_assert_cmpstr (argv[0], ==, "systemctl");
    g_assert_cmpstr (argv[1], ==, "reboot");
    g_assert_null (argv[2]);
}

static void power_action_keeps_logout_candidates_as_last_resort_only (void)
{
    g_autofree char* oldSession = g_strdup (g_getenv ("XDG_SESSION_ID"));
    g_autofree char* oldUser = g_strdup (g_getenv ("USER"));
    g_autoptr(GPtrArray) candidates = NULL;
    GStrv sessionArgv = NULL;
    GStrv userArgv = NULL;

    g_setenv ("XDG_SESSION_ID", "c7", TRUE);
    g_setenv ("USER", "tester", TRUE);

    candidates = graceful_panel_power_action_build_candidates (GRACEFUL_PANEL_POWER_ACTION_LOGOUT);
    g_assert_cmpuint (candidates->len, ==, 2);
    sessionArgv = g_ptr_array_index (candidates, 0);
    userArgv = g_ptr_array_index (candidates, 1);
    g_assert_cmpstr (sessionArgv[0], ==, "loginctl");
    g_assert_cmpstr (sessionArgv[1], ==, "terminate-session");
    g_assert_cmpstr (sessionArgv[2], ==, "c7");
    g_assert_null (sessionArgv[3]);
    g_assert_cmpstr (userArgv[0], ==, "loginctl");
    g_assert_cmpstr (userArgv[1], ==, "terminate-user");
    g_assert_cmpstr (userArgv[2], ==, "tester");
    g_assert_null (userArgv[3]);

    if (oldSession != NULL) {
        g_setenv ("XDG_SESSION_ID", oldSession, TRUE);
    }
    else {
        g_unsetenv ("XDG_SESSION_ID");
    }
    if (oldUser != NULL) {
        g_setenv ("USER", oldUser, TRUE);
    }
    else {
        g_unsetenv ("USER");
    }
}

static void power_action_maps_lock_to_loginctl_and_screensaver_fallbacks (void)
{
    g_autoptr(GPtrArray) candidates = graceful_panel_power_action_build_candidates (
        GRACEFUL_PANEL_POWER_ACTION_LOCK
    );
    GStrv loginctlArgv = g_ptr_array_index (candidates, 0);
    GStrv gnomeArgv = g_ptr_array_index (candidates, 1);
    GStrv xdgArgv = g_ptr_array_index (candidates, 2);

    g_assert_cmpuint (candidates->len, ==, 3);
    g_assert_cmpstr (loginctlArgv[0], ==, "loginctl");
    g_assert_cmpstr (loginctlArgv[1], ==, "lock-session");
    g_assert_null (loginctlArgv[2]);
    g_assert_cmpstr (gnomeArgv[0], ==, "gnome-screensaver-command");
    g_assert_cmpstr (gnomeArgv[1], ==, "-l");
    g_assert_null (gnomeArgv[2]);
    g_assert_cmpstr (xdgArgv[0], ==, "xdg-screensaver");
    g_assert_cmpstr (xdgArgv[1], ==, "lock");
    g_assert_null (xdgArgv[2]);
}

static void power_action_matches_graceful_mutter_parent (void)
{
    const char* argv[] = {
        "mutter",
        "--wayland",
        "--display-server",
        "--",
        "/usr/bin/graceful-session",
        NULL
    };

    g_assert_true (graceful_panel_power_action_is_graceful_mutter_argv (argv));
}

static void power_action_rejects_unrelated_mutter_parent (void)
{
    const char* argv[] = {
        "mutter",
        "--wayland",
        NULL
    };

    g_assert_false (graceful_panel_power_action_is_graceful_mutter_argv (argv));
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/panel/power-action/maps-shutdown-to-systemctl-poweroff",
        power_action_maps_shutdown_to_systemctl_poweroff
    );
    g_test_add_func (
        "/panel/power-action/maps-reboot-to-systemctl-reboot",
        power_action_maps_reboot_to_systemctl_reboot
    );
    g_test_add_func (
        "/panel/power-action/keeps-logout-candidates-as-last-resort-only",
        power_action_keeps_logout_candidates_as_last_resort_only
    );
    g_test_add_func (
        "/panel/power-action/maps-lock-to-loginctl-and-screensaver-fallbacks",
        power_action_maps_lock_to_loginctl_and_screensaver_fallbacks
    );
    g_test_add_func (
        "/panel/power-action/matches-graceful-mutter-parent",
        power_action_matches_graceful_mutter_parent
    );
    g_test_add_func (
        "/panel/power-action/rejects-unrelated-mutter-parent",
        power_action_rejects_unrelated_mutter_parent
    );

    return g_test_run ();
}
