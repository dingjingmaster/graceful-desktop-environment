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

#include "session-definition.h"

#include <glib/gstdio.h>

struct _GracefulSessionDefinition
{
    GObject parentInstance;
    char* id;
    char* name;
    gboolean kiosk;
};

G_DEFINE_TYPE (GracefulSessionDefinition, graceful_session_definition, G_TYPE_OBJECT)

static void graceful_session_definition_finalize (GObject* object)
{
    GracefulSessionDefinition* self = GRACEFUL_SESSION_DEFINITION (object);

    g_clear_pointer (&self->id, g_free);
    g_clear_pointer (&self->name, g_free);

    G_OBJECT_CLASS (graceful_session_definition_parent_class)->finalize (object);
}

static void graceful_session_definition_class_init (GracefulSessionDefinitionClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->finalize = graceful_session_definition_finalize;
}

static void graceful_session_definition_init (GracefulSessionDefinition* self)
{
    self->kiosk = FALSE;
}

static GracefulSessionDefinition* graceful_session_definition_new (const char* sessionId, const char* name, gboolean kiosk)
{
    GracefulSessionDefinition* self = g_object_new (GRACEFUL_TYPE_SESSION_DEFINITION, NULL);

    self->id = g_strdup (sessionId);
    self->name = g_strdup (name);
    self->kiosk = kiosk;

    return self;
}

static gboolean graceful_session_definition_is_empty (const char* text)
{
    return text == NULL || text[0] == '\0';
}

static char* graceful_session_definition_find_path (const char* sessionId, const char* const* searchDirs)
{
    g_autofree char* fileName = g_strdup_printf ("%s.session", sessionId);

    if (searchDirs == NULL) {
        return NULL;
    }

    for (gsize i = 0; searchDirs[i] != NULL; i++) {
        g_autofree char* path = g_build_filename (searchDirs[i], fileName, NULL);

        if (g_file_test (path, G_FILE_TEST_IS_REGULAR)) {
            return g_steal_pointer (&path);
        }
    }

    return NULL;
}

static GracefulSessionDefinition* graceful_session_definition_load_file (
    const char* sessionId,
    const char* path,
    GError** error
)
{
    g_autoptr(GKeyFile) keyFile = g_key_file_new ();
    g_autofree char* name = NULL;
    gboolean kiosk = FALSE;

    if (!g_key_file_load_from_file (keyFile, path, G_KEY_FILE_NONE, error)) {
        return NULL;
    }

    name = g_key_file_get_locale_string (keyFile, "GNOME Session", "Name", NULL, NULL);
    if (graceful_session_definition_is_empty (name)) {
        g_free (name);
        name = g_strdup (sessionId);
    }

    if (g_key_file_has_key (keyFile, "GNOME Session", "Kiosk", NULL)) {
        kiosk = g_key_file_get_boolean (keyFile, "GNOME Session", "Kiosk", NULL);
    }

    return graceful_session_definition_new (sessionId, name, kiosk);
}

GracefulSessionDefinition* graceful_session_definition_load (
    const char* sessionId,
    const char* const* searchDirs,
    GError** error
)
{
    const char* effectiveSessionId = graceful_session_definition_is_empty (sessionId) ? "graceful" : sessionId;
    g_autofree char* path = graceful_session_definition_find_path (effectiveSessionId, searchDirs);

    if (path != NULL) {
        return graceful_session_definition_load_file (effectiveSessionId, path, error);
    }

    if (g_strcmp0 (effectiveSessionId, "graceful") == 0) {
        return graceful_session_definition_new ("graceful", "Graceful", FALSE);
    }

    g_set_error (
        error,
        G_FILE_ERROR,
        G_FILE_ERROR_NOENT,
        "Session definition '%s.session' was not found",
        effectiveSessionId
    );

    return NULL;
}

const char* graceful_session_definition_get_id (GracefulSessionDefinition* self)
{
    g_return_val_if_fail (GRACEFUL_IS_SESSION_DEFINITION (self), NULL);

    return self->id;
}

const char* graceful_session_definition_get_name (GracefulSessionDefinition* self)
{
    g_return_val_if_fail (GRACEFUL_IS_SESSION_DEFINITION (self), NULL);

    return self->name;
}

gboolean graceful_session_definition_get_kiosk (GracefulSessionDefinition* self)
{
    g_return_val_if_fail (GRACEFUL_IS_SESSION_DEFINITION (self), FALSE);

    return self->kiosk;
}
