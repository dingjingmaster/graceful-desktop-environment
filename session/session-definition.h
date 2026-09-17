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

#ifndef SESSION_SESSION_DEFINITION_H
#define SESSION_SESSION_DEFINITION_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_SESSION_DEFINITION (graceful_session_definition_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulSessionDefinition,
    graceful_session_definition,
    GRACEFUL,
    SESSION_DEFINITION,
    GObject
)

GracefulSessionDefinition* graceful_session_definition_load (
    const char* sessionId,
    const char* const* searchDirs,
    GError** error
);
const char* graceful_session_definition_get_id (GracefulSessionDefinition* self);
const char* graceful_session_definition_get_name (GracefulSessionDefinition* self);
gboolean graceful_session_definition_get_kiosk (GracefulSessionDefinition* self);

G_END_DECLS

#endif
