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

#include <glib.h>

static void login_model_requires_username_password_and_session (void)
{
    GracefulGreeterLoginModel* model = graceful_greeter_login_model_new ();

    g_assert_false (graceful_greeter_login_model_can_authenticate (model));

    graceful_greeter_login_model_set_username (model, "dingjing");
    g_assert_false (graceful_greeter_login_model_can_authenticate (model));

    graceful_greeter_login_model_set_session_key (model, "graceful");
    g_assert_false (graceful_greeter_login_model_can_authenticate (model));

    graceful_greeter_login_model_set_password (model, "secret");
    g_assert_true (graceful_greeter_login_model_can_authenticate (model));

    g_object_unref (model);
}

static void login_model_normalizes_empty_text_to_null (void)
{
    GracefulGreeterLoginModel* model = graceful_greeter_login_model_new ();

    graceful_greeter_login_model_set_username (model, "");
    graceful_greeter_login_model_set_password (model, "");
    graceful_greeter_login_model_set_session_key (model, "");

    g_assert_null (graceful_greeter_login_model_get_username (model));
    g_assert_null (graceful_greeter_login_model_get_password (model));
    g_assert_null (graceful_greeter_login_model_get_session_key (model));

    g_object_unref (model);
}

static void login_model_clear_secret_only_clears_password (void)
{
    GracefulGreeterLoginModel* model = graceful_greeter_login_model_new ();

    graceful_greeter_login_model_set_username (model, "dingjing");
    graceful_greeter_login_model_set_password (model, "secret");
    graceful_greeter_login_model_set_session_key (model, "graceful");

    graceful_greeter_login_model_clear_secret (model);

    g_assert_cmpstr (graceful_greeter_login_model_get_username (model), ==, "dingjing");
    g_assert_null (graceful_greeter_login_model_get_password (model));
    g_assert_cmpstr (graceful_greeter_login_model_get_session_key (model), ==, "graceful");

    g_object_unref (model);
}

static void login_model_preserves_password_whitespace (void)
{
    GracefulGreeterLoginModel* model = graceful_greeter_login_model_new ();

    graceful_greeter_login_model_set_password (model, " secret ");

    g_assert_cmpstr (graceful_greeter_login_model_get_password (model), ==, " secret ");

    g_object_unref (model);
}

static void login_model_select_username_clears_password (void)
{
    GracefulGreeterLoginModel* model = graceful_greeter_login_model_new ();

    graceful_greeter_login_model_set_username (model, "dingjing");
    graceful_greeter_login_model_set_password (model, "secret");
    graceful_greeter_login_model_set_session_key (model, "ubuntu");

    graceful_greeter_login_model_select_username (model, "lightdm");

    g_assert_cmpstr (graceful_greeter_login_model_get_username (model), ==, "lightdm");
    g_assert_null (graceful_greeter_login_model_get_password (model));
    g_assert_cmpstr (graceful_greeter_login_model_get_session_key (model), ==, "ubuntu");

    g_object_unref (model);
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/greeter/login-model/requires-username-password-and-session",
        login_model_requires_username_password_and_session
    );
    g_test_add_func ("/greeter/login-model/normalizes-empty-text-to-null", login_model_normalizes_empty_text_to_null);
    g_test_add_func ("/greeter/login-model/clear-secret-only-clears-password", login_model_clear_secret_only_clears_password);
    g_test_add_func ("/greeter/login-model/preserves-password-whitespace", login_model_preserves_password_whitespace);
    g_test_add_func ("/greeter/login-model/select-username-clears-password", login_model_select_username_clears_password);

    return g_test_run ();
}
