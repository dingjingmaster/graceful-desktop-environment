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

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOGOUT_TERMINATE_WAIT_USEC 100000
#define LOGOUT_TERMINATE_ATTEMPTS 10

static void add_candidate (GPtrArray* candidates, const char* const* argv)
{
    g_ptr_array_add (candidates, g_strdupv ((char**) argv));
}

static gboolean basename_is (const char* command, const char* expected)
{
    const char* basename = NULL;

    if (command == NULL || command[0] == '\0') {
        return FALSE;
    }

    basename = strrchr (command, '/');
    basename = basename != NULL ? basename + 1 : command;

    return g_strcmp0 (basename, expected) == 0;
}

gboolean graceful_panel_power_action_is_graceful_mutter_argv (const char* const* argv)
{
    if (argv == NULL || !basename_is (argv[0], "mutter")) {
        return FALSE;
    }

    for (gsize i = 1; argv[i] != NULL; i++) {
        if (basename_is (argv[i], "graceful-session")) {
            return TRUE;
        }
    }

    return FALSE;
}

static GStrv read_process_argv (pid_t pid)
{
    g_autofree char* path = NULL;
    g_autofree char* contents = NULL;
    gsize length = 0;
    GPtrArray* argv = NULL;
    gsize offset = 0;

    path = g_strdup_printf ("/proc/%ld/cmdline", (long) pid);
    if (!g_file_get_contents (path, &contents, &length, NULL)) {
        return NULL;
    }

    argv = g_ptr_array_new_with_free_func (g_free);
    while (offset < length) {
        const char* arg = contents + offset;
        gsize argLen = strlen (arg);

        if (argLen > 0) {
            g_ptr_array_add (argv, g_strdup (arg));
        }
        offset += argLen + 1;
    }
    g_ptr_array_add (argv, NULL);

    return (GStrv) g_ptr_array_free (argv, FALSE);
}

static pid_t read_parent_pid (pid_t pid)
{
    g_autofree char* path = NULL;
    g_autofree char* contents = NULL;
    char* afterCommand = NULL;
    char* parentPidText = NULL;
    char* end = NULL;
    long parentPid = 0;

    path = g_strdup_printf ("/proc/%ld/stat", (long) pid);
    if (!g_file_get_contents (path, &contents, NULL, NULL)) {
        return 0;
    }

    afterCommand = strrchr (contents, ')');
    if (afterCommand == NULL || afterCommand[1] != ' ') {
        return 0;
    }

    parentPidText = afterCommand + 4;
    errno = 0;
    parentPid = strtol (parentPidText, &end, 10);
    if (errno != 0 || end == parentPidText || parentPid <= 0) {
        return 0;
    }

    return (pid_t) parentPid;
}

static pid_t find_graceful_mutter_ancestor (void)
{
    pid_t pid = getpid ();

    for (int i = 0; i < 8; i++) {
        g_auto(GStrv) argv = NULL;

        pid = read_parent_pid (pid);
        if (pid <= 1) {
            return 0;
        }

        argv = read_process_argv (pid);
        if (graceful_panel_power_action_is_graceful_mutter_argv ((const char* const*) argv)) {
            return pid;
        }
    }

    return 0;
}

static gboolean process_is_graceful_mutter (pid_t pid)
{
    g_auto(GStrv) argv = read_process_argv (pid);

    return graceful_panel_power_action_is_graceful_mutter_argv ((const char* const*) argv);
}

static gboolean terminate_graceful_mutter_logout (GError** error)
{
    pid_t pid = find_graceful_mutter_ancestor ();

    if (pid <= 1) {
        return FALSE;
    }

    if (kill (pid, SIGTERM) != 0) {
        g_set_error (
            error,
            G_IO_ERROR,
            g_io_error_from_errno (errno),
            "Failed to terminate graceful mutter %ld: %s",
            (long) pid,
            g_strerror (errno)
        );
        return FALSE;
    }

    for (int i = 0; i < LOGOUT_TERMINATE_ATTEMPTS; i++) {
        g_usleep (LOGOUT_TERMINATE_WAIT_USEC);
        if (kill (pid, 0) != 0 && errno == ESRCH) {
            return TRUE;
        }
        if (!process_is_graceful_mutter (pid)) {
            return TRUE;
        }
    }

    if (kill (pid, SIGKILL) != 0) {
        g_set_error (
            error,
            G_IO_ERROR,
            g_io_error_from_errno (errno),
            "Failed to kill graceful mutter %ld after SIGTERM: %s",
            (long) pid,
            g_strerror (errno)
        );
        return FALSE;
    }

    return TRUE;
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

    if (action == GRACEFUL_PANEL_POWER_ACTION_LOGOUT && terminate_graceful_mutter_logout (NULL)) {
        return TRUE;
    }

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
