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

static void add_candidate (GPtrArray* candidates, const char* const* argv)
{
    g_ptr_array_add (candidates, g_strdupv ((char**) argv));
}

static gboolean add_env_candidate (GPtrArray* candidates, const char* command, const char* argument, const char* envName)
{
    const char* value = g_getenv (envName);

    if (value == NULL || value[0] == '\0') {
        return FALSE;
    }

    g_ptr_array_add (candidates, g_strdupv ((char*[]) { (char*) command, (char*) argument, (char*) value, NULL }));

    return TRUE;
}

const char* graceful_panel_power_action_get_label (GracefulPanelPowerAction action)
{
    switch (action) {
    case GRACEFUL_PANEL_POWER_ACTION_SHUTDOWN:
        return "关机";
    case GRACEFUL_PANEL_POWER_ACTION_REBOOT:
        return "重启";
    case GRACEFUL_PANEL_POWER_ACTION_LOGOUT:
        return "登出";
    case GRACEFUL_PANEL_POWER_ACTION_LOCK:
        return "锁屏";
    default:
        return "Power";
    }
}

GPtrArray* graceful_panel_power_action_build_candidates (GracefulPanelPowerAction action)
{
    GPtrArray* candidates = g_ptr_array_new_with_free_func ((GDestroyNotify) g_strfreev);

    switch (action) {
    case GRACEFUL_PANEL_POWER_ACTION_SHUTDOWN:
        add_candidate (candidates, (const char* const[]) { "systemctl", "poweroff", NULL });
        break;
    case GRACEFUL_PANEL_POWER_ACTION_REBOOT:
        add_candidate (candidates, (const char* const[]) { "systemctl", "reboot", NULL });
        break;
    case GRACEFUL_PANEL_POWER_ACTION_LOGOUT:
        add_env_candidate (candidates, "loginctl", "terminate-session", "XDG_SESSION_ID");
        add_env_candidate (candidates, "loginctl", "terminate-user", "USER");
        break;
    case GRACEFUL_PANEL_POWER_ACTION_LOCK:
        add_candidate (candidates, (const char* const[]) { "loginctl", "lock-session", NULL });
        add_candidate (candidates, (const char* const[]) { "gnome-screensaver-command", "-l", NULL });
        add_candidate (candidates, (const char* const[]) { "xdg-screensaver", "lock", NULL });
        break;
    default:
        break;
    }

    return candidates;
}

gboolean graceful_panel_power_action_run (GracefulPanelPowerAction action, GError** error)
{
    g_autoptr(GPtrArray) candidates = graceful_panel_power_action_build_candidates (action);
    guint i = 0;

    for (i = 0; i < candidates->len; ++i) {
        const char* const* argv = g_ptr_array_index (candidates, i);
        g_autofree char* path = NULL;
        g_autoptr(GError) localError = NULL;
        g_autoptr(GSubprocess) subprocess = NULL;

        if (argv == NULL || argv[0] == NULL) {
            continue;
        }

        path = g_find_program_in_path (argv[0]);
        if (path == NULL) {
            continue;
        }

        subprocess = g_subprocess_newv (argv, G_SUBPROCESS_FLAGS_NONE, &localError);
        if (subprocess != NULL) {
            return TRUE;
        }
    }

    g_set_error (
        error,
        G_IO_ERROR,
        G_IO_ERROR_NOT_FOUND,
        "No command was available for %s",
        graceful_panel_power_action_get_label (action)
    );

    return FALSE;
}
