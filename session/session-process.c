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

#include <string.h>

struct _GracefulSessionProcess
{
    GObject parentInstance;
    char* name;
    GStrv argv;
    GStrv envp;
    GSubprocess* subprocess;
    int exitStatus;
};

G_DEFINE_TYPE (GracefulSessionProcess, graceful_session_process, G_TYPE_OBJECT)

static void graceful_session_process_finalize (GObject* object)
{
    GracefulSessionProcess* self = GRACEFUL_SESSION_PROCESS (object);

    g_clear_pointer (&self->name, g_free);
    g_clear_pointer (&self->argv, g_strfreev);
    g_clear_pointer (&self->envp, g_strfreev);
    g_clear_object (&self->subprocess);

    G_OBJECT_CLASS (graceful_session_process_parent_class)->finalize (object);
}

static void graceful_session_process_class_init (GracefulSessionProcessClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->finalize = graceful_session_process_finalize;
}

static void graceful_session_process_init (GracefulSessionProcess* self)
{
    self->exitStatus = -1;
}

static void graceful_session_process_apply_environment (GSubprocessLauncher* launcher, GStrv envp)
{
    if (envp == NULL) {
        return;
    }

    for (gsize i = 0; envp[i] != NULL; i++) {
        const char* equals = strchr (envp[i], '=');
        g_autofree char* variable = NULL;

        if (equals == NULL || equals == envp[i]) {
            continue;
        }

        variable = g_strndup (envp[i], equals - envp[i]);
        g_subprocess_launcher_setenv (launcher, variable, equals + 1, TRUE);
    }
}

GracefulSessionProcess* graceful_session_process_new (const char* name, const char* const* argv, const char* const* envp)
{
    GracefulSessionProcess* self = g_object_new (GRACEFUL_TYPE_SESSION_PROCESS, NULL);

    self->name = g_strdup (name != NULL ? name : "session-process");
    self->argv = g_strdupv ((GStrv) argv);
    self->envp = g_strdupv ((GStrv) envp);

    return self;
}

gboolean graceful_session_process_start (GracefulSessionProcess* self, GError** error)
{
    g_autoptr(GSubprocessLauncher) launcher = NULL;

    g_return_val_if_fail (GRACEFUL_IS_SESSION_PROCESS (self), FALSE);

    if (self->argv == NULL || self->argv[0] == NULL || self->argv[0][0] == '\0') {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, "Session process '%s' has no command", self->name);
        return FALSE;
    }

    if (self->subprocess != NULL) {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_EXISTS, "Session process '%s' has already been started", self->name);
        return FALSE;
    }

    launcher = g_subprocess_launcher_new (G_SUBPROCESS_FLAGS_NONE);
    graceful_session_process_apply_environment (launcher, self->envp);
    self->subprocess = g_subprocess_launcher_spawnv (launcher, (const char* const*) self->argv, error);

    return self->subprocess != NULL;
}

gboolean graceful_session_process_wait (GracefulSessionProcess* self, GCancellable* cancellable, GError** error)
{
    gboolean waitOk = FALSE;

    g_return_val_if_fail (GRACEFUL_IS_SESSION_PROCESS (self), FALSE);

    if (self->subprocess == NULL) {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED, "Session process '%s' has not been started", self->name);
        return FALSE;
    }

    waitOk = g_subprocess_wait (self->subprocess, cancellable, error);
    if (!waitOk) {
        return FALSE;
    }

    if (g_subprocess_get_if_exited (self->subprocess)) {
        self->exitStatus = g_subprocess_get_exit_status (self->subprocess);
        return self->exitStatus == 0;
    }

    self->exitStatus = -1;
    return FALSE;
}

void graceful_session_process_terminate (GracefulSessionProcess* self)
{
    g_return_if_fail (GRACEFUL_IS_SESSION_PROCESS (self));

    if (self->subprocess != NULL) {
        g_subprocess_force_exit (self->subprocess);
    }
}

int graceful_session_process_get_exit_status (GracefulSessionProcess* self)
{
    g_return_val_if_fail (GRACEFUL_IS_SESSION_PROCESS (self), -1);

    return self->exitStatus;
}
