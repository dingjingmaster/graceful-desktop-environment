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

#include "session-environment.h"

struct _GracefulSessionEnvironment
{
    GObject parentInstance;
    char* sessionId;
    char* currentDesktop;
};

G_DEFINE_TYPE (GracefulSessionEnvironment, graceful_session_environment, G_TYPE_OBJECT)

static void graceful_session_environment_finalize (GObject* object)
{
    GracefulSessionEnvironment* self = GRACEFUL_SESSION_ENVIRONMENT (object);

    g_clear_pointer (&self->sessionId, g_free);
    g_clear_pointer (&self->currentDesktop, g_free);

    G_OBJECT_CLASS (graceful_session_environment_parent_class)->finalize (object);
}

static void graceful_session_environment_class_init (GracefulSessionEnvironmentClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->finalize = graceful_session_environment_finalize;
}

static void graceful_session_environment_init (GracefulSessionEnvironment* self)
{
}

static const char* graceful_session_environment_default_text (const char* text, const char* fallback)
{
    if (text == NULL || text[0] == '\0') {
        return fallback;
    }

    return text;
}

GracefulSessionEnvironment* graceful_session_environment_new (const char* sessionId, const char* currentDesktop)
{
    GracefulSessionEnvironment* self = g_object_new (GRACEFUL_TYPE_SESSION_ENVIRONMENT, NULL);

    self->sessionId = g_strdup (graceful_session_environment_default_text (sessionId, "graceful"));
    self->currentDesktop = g_strdup (graceful_session_environment_default_text (currentDesktop, "Graceful"));

    return self;
}

GStrv graceful_session_environment_build (GracefulSessionEnvironment* self, const char* const* baseEnv)
{
    GStrv envp = NULL;

    g_return_val_if_fail (GRACEFUL_IS_SESSION_ENVIRONMENT (self), NULL);

    if (baseEnv != NULL) {
        envp = g_strdupv ((GStrv) baseEnv);
    }
    else {
        envp = g_get_environ ();
    }

    envp = g_environ_setenv (envp, "DESKTOP_SESSION", self->sessionId, TRUE);
    envp = g_environ_setenv (envp, "GDMSESSION", self->sessionId, TRUE);
    envp = g_environ_setenv (envp, "XDG_CURRENT_DESKTOP", self->currentDesktop, TRUE);

    return envp;
}
