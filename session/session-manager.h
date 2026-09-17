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

#ifndef SESSION_SESSION_MANAGER_H
#define SESSION_SESSION_MANAGER_H

#include "session-definition.h"

#include <gio/gio.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_SESSION_MANAGER (graceful_session_manager_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulSessionManager,
    graceful_session_manager,
    GRACEFUL,
    SESSION_MANAGER,
    GObject
)

GracefulSessionManager* graceful_session_manager_new (
    GracefulSessionDefinition* definition,
    const char* const* commandArgv
);
gboolean graceful_session_manager_run (GracefulSessionManager* self, GCancellable* cancellable, GError** error);
int graceful_session_manager_get_exit_status (GracefulSessionManager* self);

G_END_DECLS

#endif
