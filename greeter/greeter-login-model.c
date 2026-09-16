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
#include "greeter-login-model.h"

struct _GracefulGreeterLoginModel
{
    GObject parentInstance;

    char* username;
    char* password;
    char* sessionKey;
};

G_DEFINE_TYPE (GracefulGreeterLoginModel, graceful_greeter_login_model, G_TYPE_OBJECT)

static char* normalize_text (const char* text)
{
    char* normalized = NULL;

    if (text == NULL) {
        return NULL;
    }

    normalized = g_strdup (text);
    g_strstrip (normalized);

    if (normalized[0] == '\0') {
        g_free (normalized);
        return NULL;
    }

    return normalized;
}

static char* normalize_secret (const char* text)
{
    if (text == NULL || text[0] == '\0') {
        return NULL;
    }

    return g_strdup (text);
}

static void replace_text (char** target, const char* text, gboolean secret)
{
    char* normalized = secret ? normalize_secret (text) : normalize_text (text);

    g_free (*target);
    *target = normalized;
}

static void graceful_greeter_login_model_finalize (GObject* object)
{
    GracefulGreeterLoginModel* self = GRACEFUL_GREETER_LOGIN_MODEL (object);

    g_free (self->username);
    g_free (self->password);
    g_free (self->sessionKey);

    G_OBJECT_CLASS (graceful_greeter_login_model_parent_class)->finalize (object);
}

static void graceful_greeter_login_model_class_init (GracefulGreeterLoginModelClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->finalize = graceful_greeter_login_model_finalize;
}

static void graceful_greeter_login_model_init (GracefulGreeterLoginModel* self)
{
    self->username = NULL;
    self->password = NULL;
    self->sessionKey = NULL;
}

GracefulGreeterLoginModel* graceful_greeter_login_model_new (void)
{
    return g_object_new (GRACEFUL_TYPE_GREETER_LOGIN_MODEL, NULL);
}

void graceful_greeter_login_model_set_username (GracefulGreeterLoginModel* model, const char* username)
{
    g_return_if_fail (GRACEFUL_IS_GREETER_LOGIN_MODEL (model));

    replace_text (&model->username, username, FALSE);
}

void graceful_greeter_login_model_select_username (GracefulGreeterLoginModel* model, const char* username)
{
    g_return_if_fail (GRACEFUL_IS_GREETER_LOGIN_MODEL (model));

    replace_text (&model->username, username, FALSE);
    graceful_greeter_login_model_clear_secret (model);
}

const char* graceful_greeter_login_model_get_username (GracefulGreeterLoginModel* model)
{
    g_return_val_if_fail (GRACEFUL_IS_GREETER_LOGIN_MODEL (model), NULL);

    return model->username;
}

void graceful_greeter_login_model_set_password (GracefulGreeterLoginModel* model, const char* password)
{
    g_return_if_fail (GRACEFUL_IS_GREETER_LOGIN_MODEL (model));

    replace_text (&model->password, password, TRUE);
}

const char* graceful_greeter_login_model_get_password (GracefulGreeterLoginModel* model)
{
    g_return_val_if_fail (GRACEFUL_IS_GREETER_LOGIN_MODEL (model), NULL);

    return model->password;
}

void graceful_greeter_login_model_clear_secret (GracefulGreeterLoginModel* model)
{
    g_return_if_fail (GRACEFUL_IS_GREETER_LOGIN_MODEL (model));

    g_clear_pointer (&model->password, g_free);
}

void graceful_greeter_login_model_set_session_key (GracefulGreeterLoginModel* model, const char* sessionKey)
{
    g_return_if_fail (GRACEFUL_IS_GREETER_LOGIN_MODEL (model));

    replace_text (&model->sessionKey, sessionKey, FALSE);
}

const char* graceful_greeter_login_model_get_session_key (GracefulGreeterLoginModel* model)
{
    g_return_val_if_fail (GRACEFUL_IS_GREETER_LOGIN_MODEL (model), NULL);

    return model->sessionKey;
}

gboolean graceful_greeter_login_model_can_authenticate (GracefulGreeterLoginModel* model)
{
    g_return_val_if_fail (GRACEFUL_IS_GREETER_LOGIN_MODEL (model), FALSE);

    return model->username != NULL && model->password != NULL && model->sessionKey != NULL;
}
