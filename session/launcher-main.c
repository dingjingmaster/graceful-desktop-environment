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

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#define TERMINATE_WAIT_USEC 100000
#define TERMINATE_ATTEMPTS 10

static volatile sig_atomic_t terminationRequested = 0;
static pid_t mutterPid = -1;

static void on_termination_signal (int signo)
{
    terminationRequested = signo;
    if (mutterPid > 0) {
        kill (mutterPid, SIGTERM);
    }
}

static gboolean process_exists (pid_t pid)
{
    if (pid <= 0) {
        return FALSE;
    }

    if (kill (pid, 0) == 0) {
        return TRUE;
    }

    return errno != ESRCH;
}

static void terminate_mutter_child (void)
{
    if (mutterPid <= 0) {
        return;
    }

    kill (mutterPid, SIGTERM);
    for (int i = 0; i < TERMINATE_ATTEMPTS; i++) {
        int status = 0;
        pid_t result = waitpid (mutterPid, &status, WNOHANG);

        if (result == mutterPid || (result < 0 && errno == ECHILD)) {
            mutterPid = -1;
            return;
        }

        if (!process_exists (mutterPid)) {
            mutterPid = -1;
            return;
        }

        g_usleep (TERMINATE_WAIT_USEC);
    }

    kill (mutterPid, SIGKILL);
    waitpid (mutterPid, NULL, 0);
    mutterPid = -1;
}

static int wait_for_mutter_child (void)
{
    int status = 0;

    while (TRUE) {
        pid_t result = waitpid (mutterPid, &status, 0);

        if (result == mutterPid) {
            mutterPid = -1;
            if (WIFEXITED (status)) {
                return WEXITSTATUS (status);
            }
            if (WIFSIGNALED (status)) {
                return 128 + WTERMSIG (status);
            }
            return 1;
        }

        if (result < 0 && errno == EINTR) {
            if (terminationRequested != 0) {
                terminate_mutter_child ();
                return 128 + terminationRequested;
            }
            continue;
        }

        if (result < 0) {
            g_printerr ("graceful-session-launcher: waitpid failed: %s\n", g_strerror (errno));
            return 1;
        }
    }
}

int main (int argc, char* argv[])
{
    g_auto(GStrv) mutterArgv = NULL;
    struct sigaction action = { 0 };

    (void) argc;
    (void) argv;

    action.sa_handler = on_termination_signal;
    sigemptyset (&action.sa_mask);
    sigaction (SIGTERM, &action, NULL);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGHUP, &action, NULL);

    mutterArgv = graceful_session_launcher_build_mutter_argv (NULL);
    mutterPid = fork ();
    if (mutterPid < 0) {
        g_printerr ("graceful-session-launcher: fork failed: %s\n", g_strerror (errno));
        return 1;
    }

    if (mutterPid == 0) {
        execvp (mutterArgv[0], mutterArgv);
        g_printerr ("graceful-session-launcher: failed to exec mutter: %s\n", g_strerror (errno));
        _exit (127);
    }

    g_printerr ("graceful-session-launcher: started mutter pid %ld\n", (long) mutterPid);

    return wait_for_mutter_child ();
}
