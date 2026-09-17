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

#include "session-process.h"

#include <glib.h>

static void process_reports_success_exit_status (void)
{
    const char* argv[] = { "/bin/sh", "-c", "exit 0", NULL };
    g_autoptr(GError) error = NULL;
    g_autoptr(GracefulSessionProcess) process = graceful_session_process_new ("success", argv, NULL);

    g_assert_true (graceful_session_process_start (process, &error));
    g_assert_no_error (error);
    g_assert_true (graceful_session_process_wait (process, NULL, &error));
    g_assert_no_error (error);
    g_assert_cmpint (graceful_session_process_get_exit_status (process), ==, 0);
}

static void process_reports_failure_exit_status (void)
{
    const char* argv[] = { "/bin/sh", "-c", "exit 7", NULL };
    g_autoptr(GError) error = NULL;
    g_autoptr(GracefulSessionProcess) process = graceful_session_process_new ("failure", argv, NULL);

    g_assert_true (graceful_session_process_start (process, &error));
    g_assert_no_error (error);
    g_assert_false (graceful_session_process_wait (process, NULL, &error));
    g_assert_no_error (error);
    g_assert_cmpint (graceful_session_process_get_exit_status (process), ==, 7);
}

static void process_rejects_missing_command (void)
{
    const char* argv[] = { NULL };
    g_autoptr(GError) error = NULL;
    g_autoptr(GracefulSessionProcess) process = graceful_session_process_new ("missing", argv, NULL);

    g_assert_false (graceful_session_process_start (process, &error));
    g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);
}

typedef struct _AsyncWaitFixture AsyncWaitFixture;

struct _AsyncWaitFixture
{
    GMainLoop* loop;
    GracefulSessionProcess* process;
    gboolean ok;
    GError* error;
};

static void process_async_wait_done (GObject* sourceObject, GAsyncResult* result, gpointer userData)
{
    AsyncWaitFixture* fixture = userData;

    g_assert_true (GRACEFUL_IS_SESSION_PROCESS (sourceObject));
    fixture->ok = graceful_session_process_wait_finish (fixture->process, result, &fixture->error);
    g_main_loop_quit (fixture->loop);
}

static void process_reports_async_success_exit_status (void)
{
    const char* argv[] = { "/bin/sh", "-c", "exit 0", NULL };
    g_autoptr(GMainLoop) loop = g_main_loop_new (NULL, FALSE);
    g_autoptr(GracefulSessionProcess) process = graceful_session_process_new ("async-success", argv, NULL);
    AsyncWaitFixture fixture = { 0 };
    g_autoptr(GError) error = NULL;

    fixture.loop = loop;
    fixture.process = process;

    g_assert_true (graceful_session_process_start (process, &error));
    g_assert_no_error (error);

    graceful_session_process_wait_async (process, NULL, process_async_wait_done, &fixture);
    g_main_loop_run (loop);

    g_assert_no_error (fixture.error);
    g_assert_true (fixture.ok);
    g_assert_cmpint (graceful_session_process_get_exit_status (process), ==, 0);
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/session/process/reports-success-exit-status", process_reports_success_exit_status);
    g_test_add_func ("/session/process/reports-failure-exit-status", process_reports_failure_exit_status);
    g_test_add_func ("/session/process/rejects-missing-command", process_rejects_missing_command);
    g_test_add_func ("/session/process/reports-async-success-exit-status", process_reports_async_success_exit_status);

    return g_test_run ();
}
