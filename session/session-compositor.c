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

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define COMPOSITOR_TERMINATE_WAIT_USEC 100000
#define COMPOSITOR_TERMINATE_ATTEMPTS 10

static gboolean basename_is_mutter (const char* command)
{
    const char* basename = NULL;

    if (command == NULL || command[0] == '\0') {
        return FALSE;
    }

    basename = strrchr (command, '/');
    basename = basename != NULL ? basename + 1 : command;

    return g_strcmp0 (basename, "mutter") == 0;
}

static gboolean argv_contains_session_command (const char* const* argv, const char* sessionCommand)
{
    const char* sessionBasename = NULL;

    if (sessionCommand == NULL || sessionCommand[0] == '\0') {
        return FALSE;
    }

    sessionBasename = strrchr (sessionCommand, '/');
    sessionBasename = sessionBasename != NULL ? sessionBasename + 1 : sessionCommand;

    for (gsize i = 0; argv != NULL && argv[i] != NULL; i++) {
        const char* arg = argv[i];
        const char* argBasename = strrchr (arg, '/');

        argBasename = argBasename != NULL ? argBasename + 1 : arg;
        if (g_strcmp0 (arg, sessionCommand) == 0 || g_strcmp0 (argBasename, sessionBasename) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

gboolean graceful_session_compositor_argv_matches (const char* const* argv, const char* sessionCommand)
{
    return basename_is_mutter (argv != NULL ? argv[0] : NULL) &&
        argv_contains_session_command (argv, sessionCommand);
}

static GStrv read_parent_argv (pid_t parentPid, GError** error)
{
    g_autofree char* path = NULL;
    g_autofree char* contents = NULL;
    gsize length = 0;
    GPtrArray* argv = NULL;
    gsize offset = 0;

    path = g_strdup_printf ("/proc/%ld/cmdline", (long) parentPid);
    if (!g_file_get_contents (path, &contents, &length, error)) {
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

gboolean graceful_session_compositor_terminate_parent (const char* sessionCommand, GError** error)
{
    pid_t parentPid = getppid ();
    g_auto(GStrv) parentArgv = NULL;

    if (parentPid <= 1) {
        return FALSE;
    }

    parentArgv = read_parent_argv (parentPid, error);
    if (parentArgv == NULL) {
        return FALSE;
    }

    if (!graceful_session_compositor_argv_matches ((const char* const*) parentArgv, sessionCommand)) {
        return FALSE;
    }

    if (kill (parentPid, SIGTERM) != 0) {
        g_set_error (
            error,
            G_IO_ERROR,
            g_io_error_from_errno (errno),
            "Failed to terminate parent compositor %ld: %s",
            (long) parentPid,
            g_strerror (errno)
        );
        return FALSE;
    }

    for (int i = 0; i < COMPOSITOR_TERMINATE_ATTEMPTS; i++) {
        g_auto(GStrv) currentArgv = NULL;

        g_usleep (COMPOSITOR_TERMINATE_WAIT_USEC);
        if (kill (parentPid, 0) != 0 && errno == ESRCH) {
            return TRUE;
        }

        currentArgv = read_parent_argv (parentPid, NULL);
        if (currentArgv == NULL ||
            !graceful_session_compositor_argv_matches ((const char* const*) currentArgv, sessionCommand)) {
            return TRUE;
        }
    }

    if (kill (parentPid, SIGKILL) != 0) {
        g_set_error (
            error,
            G_IO_ERROR,
            g_io_error_from_errno (errno),
            "Failed to kill parent compositor %ld after SIGTERM: %s",
            (long) parentPid,
            g_strerror (errno)
        );
        return FALSE;
    }

    return TRUE;
}
