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

#include "session-manager.h"

#include "session-environment.h"
#include "session-process.h"

struct _GracefulSessionManager
{
    GObject parentInstance;
    GracefulSessionDefinition* definition;
    GStrv commandArgv;
    int exitStatus;
};

G_DEFINE_TYPE (GracefulSessionManager, graceful_session_manager, G_TYPE_OBJECT)

static void graceful_session_manager_finalize (GObject* object)
{
    GracefulSessionManager* self = GRACEFUL_SESSION_MANAGER (object);

    g_clear_object (&self->definition);
    g_clear_pointer (&self->commandArgv, g_strfreev);

    G_OBJECT_CLASS (graceful_session_manager_parent_class)->finalize (object);
}

static void graceful_session_manager_class_init (GracefulSessionManagerClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->finalize = graceful_session_manager_finalize;
}

static void graceful_session_manager_init (GracefulSessionManager* self)
{
    self->exitStatus = -1;
}

GracefulSessionManager* graceful_session_manager_new (
    GracefulSessionDefinition* definition,
    const char* const* commandArgv
)
{
    GracefulSessionManager* self = NULL;

    g_return_val_if_fail (GRACEFUL_IS_SESSION_DEFINITION (definition), NULL);

    self = g_object_new (GRACEFUL_TYPE_SESSION_MANAGER, NULL);
    self->definition = g_object_ref (definition);
    self->commandArgv = g_strdupv ((GStrv) commandArgv);

    return self;
}

gboolean graceful_session_manager_run (GracefulSessionManager* self, GCancellable* cancellable, GError** error)
{
    g_autoptr(GracefulSessionEnvironment) environment = NULL;
    g_autoptr(GracefulSessionProcess) process = NULL;
    g_auto(GStrv) envp = NULL;
    gboolean ok = FALSE;

    g_return_val_if_fail (GRACEFUL_IS_SESSION_MANAGER (self), FALSE);

    if (self->commandArgv == NULL || self->commandArgv[0] == NULL) {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, "No session command was provided");
        return FALSE;
    }

    environment = graceful_session_environment_new (
        graceful_session_definition_get_id (self->definition),
        graceful_session_definition_get_name (self->definition)
    );
    envp = graceful_session_environment_build (environment, NULL);
    process = graceful_session_process_new ("session-command", (const char* const*) self->commandArgv, (const char* const*) envp);

    if (!graceful_session_process_start (process, error)) {
        return FALSE;
    }

    ok = graceful_session_process_wait (process, cancellable, error);
    self->exitStatus = graceful_session_process_get_exit_status (process);

    return ok;
}

int graceful_session_manager_get_exit_status (GracefulSessionManager* self)
{
    g_return_val_if_fail (GRACEFUL_IS_SESSION_MANAGER (self), -1);

    return self->exitStatus;
}
