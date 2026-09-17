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

#include "session-autostart.h"
#include "session-component.h"
#include "session-environment.h"
#include "session-process.h"

struct _GracefulSessionManager
{
    GObject parentInstance;
    GracefulSessionDefinition* definition;
    GStrv commandArgv;
    int exitStatus;
};

typedef struct _GracefulSessionWaitContext GracefulSessionWaitContext;

struct _GracefulSessionWaitContext
{
    GMainLoop* loop;
    guint pending;
    gboolean finished;
    gboolean ok;
    int exitStatus;
    GError* error;
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

static void graceful_session_manager_wait_done (GObject* sourceObject, GAsyncResult* result, gpointer userData)
{
    GracefulSessionWaitContext* context = userData;
    GracefulSessionProcess* process = GRACEFUL_SESSION_PROCESS (sourceObject);
    g_autoptr(GError) error = NULL;
    gboolean ok = graceful_session_process_wait_finish (process, result, &error);

    if (context->pending > 0) {
        context->pending--;
    }

    if (context->finished) {
        return;
    }

    context->finished = TRUE;
    context->ok = ok;
    context->exitStatus = graceful_session_process_get_exit_status (process);
    if (error != NULL) {
        context->error = g_steal_pointer (&error);
    }

    g_main_loop_quit (context->loop);
}

static void graceful_session_manager_terminate_processes (GPtrArray* processes)
{
    for (guint i = 0; processes != NULL && i < processes->len; i++) {
        GracefulSessionProcess* process = g_ptr_array_index (processes, i);

        graceful_session_process_terminate (process);
    }
}

static gboolean graceful_session_manager_run_single_command (
    GracefulSessionManager* self,
    const char* const* envp,
    GCancellable* cancellable,
    GError** error
)
{
    g_autoptr(GracefulSessionProcess) process = NULL;
    gboolean ok = FALSE;

    if (self->commandArgv == NULL || self->commandArgv[0] == NULL) {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, "No session command was provided");
        return FALSE;
    }

    process = graceful_session_process_new ("session-command", (const char* const*) self->commandArgv, envp);

    if (!graceful_session_process_start (process, error)) {
        return FALSE;
    }

    ok = graceful_session_process_wait (process, cancellable, error);
    self->exitStatus = graceful_session_process_get_exit_status (process);

    return ok;
}

static gboolean graceful_session_manager_start_component (
    GracefulSessionComponent* component,
    const char* const* envp,
    GCancellable* cancellable,
    GPtrArray* processes,
    GError** error
)
{
    g_autoptr(GracefulSessionProcess) process = NULL;
    gboolean ok = FALSE;

    process = graceful_session_process_new (
        graceful_session_component_get_name (component),
        graceful_session_component_get_argv (component),
        envp
    );

    if (!graceful_session_process_start (process, error)) {
        return FALSE;
    }

    if (graceful_session_component_get_oneshot (component)) {
        ok = graceful_session_process_wait (process, cancellable, error);
        return ok;
    }

    g_ptr_array_add (processes, g_object_ref (process));
    return TRUE;
}

static GStrv graceful_session_manager_build_autostart_dirs (void)
{
    const char* const* configDirs = NULL;
    GPtrArray* dirs = g_ptr_array_new_with_free_func (g_free);
    const char* configHome = g_get_user_config_dir ();

    if (configHome != NULL) {
        g_ptr_array_add (dirs, g_strdup (configHome));
    }

    configDirs = g_get_system_config_dirs ();
    for (gsize i = 0; configDirs != NULL && configDirs[i] != NULL; i++) {
        g_ptr_array_add (dirs, g_strdup (configDirs[i]));
    }

    g_ptr_array_add (dirs, NULL);

    return (GStrv) g_ptr_array_free (dirs, FALSE);
}

static void graceful_session_manager_start_autostart (const char* const* envp)
{
    g_autoptr(GracefulSessionAutostart) autostart = graceful_session_autostart_new ("Graceful");
    g_auto(GStrv) configDirs = graceful_session_manager_build_autostart_dirs ();
    g_autoptr(GPtrArray) commands = NULL;
    const char* pathEnv = g_environ_getenv ((GStrv) envp, "PATH");

    commands = graceful_session_autostart_list_commands (autostart, (const char* const*) configDirs, pathEnv);
    for (guint i = 0; commands != NULL && i < commands->len; i++) {
        const char* const* argv = g_ptr_array_index (commands, i);
        g_autoptr(GracefulSessionProcess) process = graceful_session_process_new ("xdg-autostart", argv, envp);
        g_autoptr(GError) error = NULL;

        if (!graceful_session_process_start (process, &error)) {
            g_printerr (
                "graceful-session: autostart '%s' failed: %s\n",
                argv != NULL && argv[0] != NULL ? argv[0] : "unknown",
                error != NULL ? error->message : "unknown error"
            );
        }
    }
}

static gboolean graceful_session_manager_run_components (
    GracefulSessionManager* self,
    const char* const* envp,
    GCancellable* cancellable,
    GError** error
)
{
    g_autoptr(GPtrArray) components = graceful_session_component_list_new_default ();
    g_autoptr(GPtrArray) processes = g_ptr_array_new_with_free_func (g_object_unref);
    g_autoptr(GMainLoop) loop = NULL;
    GracefulSessionWaitContext waitContext = { 0 };

    for (guint i = 0; i < components->len; i++) {
        GracefulSessionComponent* component = g_ptr_array_index (components, i);
        g_autoptr(GError) componentError = NULL;

        if (!graceful_session_manager_start_component (component, envp, cancellable, processes, &componentError)) {
            if (graceful_session_component_get_required (component)) {
                graceful_session_manager_terminate_processes (processes);
                g_propagate_prefixed_error (
                    error,
                    g_steal_pointer (&componentError),
                    "Failed to start session component '%s': ",
                    graceful_session_component_get_name (component)
                );
                return FALSE;
            }

            g_printerr (
                "graceful-session: optional component '%s' failed: %s\n",
                graceful_session_component_get_name (component),
                componentError != NULL ? componentError->message : "unknown error"
            );
        }
    }

    graceful_session_manager_start_autostart (envp);

    loop = g_main_loop_new (NULL, FALSE);
    waitContext.loop = loop;
    waitContext.pending = processes->len;
    waitContext.exitStatus = -1;

    if (processes->len == 0) {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED, "No session background component was started");
        return FALSE;
    }

    for (guint i = 0; i < processes->len; i++) {
        GracefulSessionProcess* process = g_ptr_array_index (processes, i);

        graceful_session_process_wait_async (process, cancellable, graceful_session_manager_wait_done, &waitContext);
    }

    g_main_loop_run (loop);

    self->exitStatus = waitContext.exitStatus;
    graceful_session_manager_terminate_processes (processes);
    while (waitContext.pending > 0) {
        g_main_context_iteration (NULL, TRUE);
    }

    if (waitContext.error != NULL) {
        g_propagate_error (error, waitContext.error);
        return FALSE;
    }

    return waitContext.ok;
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
    g_auto(GStrv) envp = NULL;

    g_return_val_if_fail (GRACEFUL_IS_SESSION_MANAGER (self), FALSE);

    environment = graceful_session_environment_new (
        graceful_session_definition_get_id (self->definition),
        graceful_session_definition_get_name (self->definition)
    );
    envp = graceful_session_environment_build (environment, NULL);

    if (self->commandArgv != NULL && self->commandArgv[0] != NULL) {
        return graceful_session_manager_run_single_command (self, (const char* const*) envp, cancellable, error);
    }

    return graceful_session_manager_run_components (self, (const char* const*) envp, cancellable, error);
}

int graceful_session_manager_get_exit_status (GracefulSessionManager* self)
{
    g_return_val_if_fail (GRACEFUL_IS_SESSION_MANAGER (self), -1);

    return self->exitStatus;
}
