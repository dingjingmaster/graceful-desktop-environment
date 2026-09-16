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

#include <gtk/gtk.h>
#include <lightdm.h>

typedef struct _GreeterUi GreeterUi;

struct _GreeterUi
{
    GtkWidget* window;
    GtkWidget* userDropDown;
    GtkWidget* usernameEntry;
    GtkWidget* passwordEntry;
    GtkWidget* sessionDropDown;
    GtkWidget* promptLabel;
    GtkWidget* messageLabel;
    GtkWidget* loginButton;

    LightDMGreeter* greeter;
    GracefulGreeterLoginModel* model;
    GPtrArray* userNames;
    GPtrArray* sessionKeys;
    gboolean awaitingPrompt;
};

static void greeter_ui_set_message (GreeterUi* ui, const char* message)
{
    gtk_label_set_text (GTK_LABEL (ui->messageLabel), message != NULL ? message : "");
}

static void greeter_ui_update_login_button (GreeterUi* ui)
{
    gboolean canAuthenticate = graceful_greeter_login_model_can_authenticate (ui->model);

    gtk_widget_set_sensitive (ui->loginButton, canAuthenticate);
}

static const char* greeter_ui_get_selected_session_key (GreeterUi* ui)
{
    guint selected = gtk_drop_down_get_selected (GTK_DROP_DOWN (ui->sessionDropDown));

    if (selected == GTK_INVALID_LIST_POSITION || selected >= ui->sessionKeys->len) {
        return NULL;
    }

    return g_ptr_array_index (ui->sessionKeys, selected);
}

static void greeter_ui_store_selected_session (GreeterUi* ui)
{
    graceful_greeter_login_model_set_session_key (ui->model, greeter_ui_get_selected_session_key (ui));
    greeter_ui_update_login_button (ui);
}

static const char* greeter_ui_get_selected_username (GreeterUi* ui)
{
    guint selected = gtk_drop_down_get_selected (GTK_DROP_DOWN (ui->userDropDown));

    if (selected == GTK_INVALID_LIST_POSITION || selected >= ui->userNames->len) {
        return NULL;
    }

    return g_ptr_array_index (ui->userNames, selected);
}

static void greeter_ui_select_username (GreeterUi* ui, const char* username)
{
    graceful_greeter_login_model_select_username (ui->model, username);
    gtk_editable_set_text (GTK_EDITABLE (ui->usernameEntry), username != NULL ? username : "");
    gtk_editable_set_text (GTK_EDITABLE (ui->passwordEntry), "");
    greeter_ui_update_login_button (ui);
}

static void greeter_ui_respond_to_prompt (GreeterUi* ui)
{
    g_autoptr(GError) error = NULL;
    const char* password = graceful_greeter_login_model_get_password (ui->model);

    if (!ui->awaitingPrompt) {
        return;
    }

    if (password == NULL) {
        greeter_ui_set_message (ui, "Password is required.");
        return;
    }

    ui->awaitingPrompt = FALSE;
    gtk_widget_set_sensitive (ui->loginButton, FALSE);

    if (!lightdm_greeter_respond (ui->greeter, password, &error)) {
        graceful_greeter_login_model_clear_secret (ui->model);
        gtk_editable_set_text (GTK_EDITABLE (ui->passwordEntry), "");
        greeter_ui_set_message (ui, error != NULL ? error->message : "Failed to send authentication response.");
        greeter_ui_update_login_button (ui);
    }
}

static void greeter_ui_authenticate (GreeterUi* ui)
{
    g_autoptr(GError) error = NULL;
    const char* username = graceful_greeter_login_model_get_username (ui->model);

    if (!graceful_greeter_login_model_can_authenticate (ui->model)) {
        greeter_ui_set_message (ui, "Enter a username and choose a session.");
        return;
    }

    if (lightdm_greeter_get_in_authentication (ui->greeter)) {
        greeter_ui_respond_to_prompt (ui);
        return;
    }

    gtk_widget_set_sensitive (ui->loginButton, FALSE);
    greeter_ui_set_message (ui, "Authenticating...");

    if (!lightdm_greeter_authenticate (ui->greeter, username, &error)) {
        greeter_ui_set_message (ui, error != NULL ? error->message : "Failed to start authentication.");
        greeter_ui_update_login_button (ui);
    }
}

static void greeter_ui_populate_sessions (GreeterUi* ui)
{
    GtkStringList* sessionNames = gtk_string_list_new (NULL);
    const char* defaultSession = lightdm_greeter_get_default_session_hint (ui->greeter);
    GList* sessions = lightdm_get_sessions ();
    guint selectedIndex = GTK_INVALID_LIST_POSITION;
    guint index = 0;

    for (GList* iter = sessions; iter != NULL; iter = iter->next) {
        LightDMSession* session = LIGHTDM_SESSION (iter->data);
        const char* key = lightdm_session_get_key (session);
        const char* name = lightdm_session_get_name (session);

        if (key == NULL) {
            continue;
        }

        gtk_string_list_append (sessionNames, name != NULL ? name : key);
        g_ptr_array_add (ui->sessionKeys, g_strdup (key));

        if (defaultSession != NULL && g_strcmp0 (defaultSession, key) == 0) {
            selectedIndex = index;
        }

        index++;
    }

    if (ui->sessionKeys->len == 0 && defaultSession != NULL) {
        gtk_string_list_append (sessionNames, defaultSession);
        g_ptr_array_add (ui->sessionKeys, g_strdup (defaultSession));
        selectedIndex = 0;
    }

    gtk_drop_down_set_model (GTK_DROP_DOWN (ui->sessionDropDown), G_LIST_MODEL (sessionNames));
    g_object_unref (sessionNames);

    if (ui->sessionKeys->len > 0) {
        gtk_drop_down_set_selected (
            GTK_DROP_DOWN (ui->sessionDropDown),
            selectedIndex != GTK_INVALID_LIST_POSITION ? selectedIndex : 0
        );
    }

    greeter_ui_store_selected_session (ui);
}

static void greeter_ui_populate_users (GreeterUi* ui)
{
    LightDMUserList* userList = lightdm_user_list_get_instance ();
    GList* users = lightdm_user_list_get_users (userList);
    GtkStringList* userDisplayNames = gtk_string_list_new (NULL);
    const char* selectedUser = lightdm_greeter_get_select_user_hint (ui->greeter);
    guint selectedIndex = GTK_INVALID_LIST_POSITION;
    guint index = 0;

    if (lightdm_greeter_get_hide_users_hint (ui->greeter)) {
        gtk_widget_set_visible (ui->userDropDown, FALSE);
        return;
    }

    for (GList* iter = users; iter != NULL; iter = iter->next) {
        LightDMUser* user = LIGHTDM_USER (iter->data);
        const char* username = lightdm_user_get_name (user);
        const char* displayName = lightdm_user_get_display_name (user);

        if (username == NULL) {
            continue;
        }

        gtk_string_list_append (userDisplayNames, displayName != NULL ? displayName : username);
        g_ptr_array_add (ui->userNames, g_strdup (username));

        if (selectedUser != NULL && g_strcmp0 (selectedUser, username) == 0) {
            selectedIndex = index;
        }

        index++;
    }

    gtk_drop_down_set_model (GTK_DROP_DOWN (ui->userDropDown), G_LIST_MODEL (userDisplayNames));
    g_object_unref (userDisplayNames);

    if (ui->userNames->len == 0) {
        gtk_widget_set_visible (ui->userDropDown, FALSE);
        return;
    }

    gtk_drop_down_set_selected (
        GTK_DROP_DOWN (ui->userDropDown),
        selectedIndex != GTK_INVALID_LIST_POSITION ? selectedIndex : 0
    );
    greeter_ui_select_username (ui, greeter_ui_get_selected_username (ui));
}

static void username_changed_cb (GtkEditable* editable, gpointer userData)
{
    GreeterUi* ui = userData;

    graceful_greeter_login_model_set_username (ui->model, gtk_editable_get_text (editable));
    greeter_ui_update_login_button (ui);
}

static void password_changed_cb (GtkEditable* editable, gpointer userData)
{
    GreeterUi* ui = userData;

    graceful_greeter_login_model_set_password (ui->model, gtk_editable_get_text (editable));
}

static void session_selected_cb (GObject* object, GParamSpec* pspec, gpointer userData)
{
    GreeterUi* ui = userData;

    (void) object;
    (void) pspec;

    greeter_ui_store_selected_session (ui);
}

static void user_selected_cb (GObject* object, GParamSpec* pspec, gpointer userData)
{
    GreeterUi* ui = userData;

    (void) object;
    (void) pspec;

    greeter_ui_select_username (ui, greeter_ui_get_selected_username (ui));
}

static void login_clicked_cb (GtkButton* button, gpointer userData)
{
    GreeterUi* ui = userData;

    (void) button;

    greeter_ui_authenticate (ui);
}

static void lightdm_show_prompt_cb (
    LightDMGreeter* greeter,
    const gchar* text,
    LightDMPromptType type,
    gpointer userData
)
{
    GreeterUi* ui = userData;

    (void) greeter;

    gtk_label_set_text (GTK_LABEL (ui->promptLabel), text != NULL ? text : "");
    ui->awaitingPrompt = TRUE;

    if (type == LIGHTDM_PROMPT_TYPE_SECRET) {
        gtk_widget_grab_focus (ui->passwordEntry);
    }
    else {
        gtk_widget_grab_focus (ui->usernameEntry);
    }

    greeter_ui_update_login_button (ui);

    if (type == LIGHTDM_PROMPT_TYPE_SECRET && graceful_greeter_login_model_get_password (ui->model) != NULL) {
        greeter_ui_respond_to_prompt (ui);
    }
}

static void lightdm_show_message_cb (
    LightDMGreeter* greeter,
    const gchar* text,
    LightDMMessageType type,
    gpointer userData
)
{
    GreeterUi* ui = userData;

    (void) greeter;
    (void) type;

    greeter_ui_set_message (ui, text);
}

static void lightdm_authentication_complete_cb (LightDMGreeter* greeter, gpointer userData)
{
    GreeterUi* ui = userData;
    const char* sessionKey = graceful_greeter_login_model_get_session_key (ui->model);
    g_autoptr(GError) error = NULL;

    ui->awaitingPrompt = FALSE;

    if (!lightdm_greeter_get_is_authenticated (greeter)) {
        graceful_greeter_login_model_clear_secret (ui->model);
        gtk_editable_set_text (GTK_EDITABLE (ui->passwordEntry), "");
        greeter_ui_set_message (ui, "Authentication failed.");
        greeter_ui_update_login_button (ui);
        return;
    }

    greeter_ui_set_message (ui, "Starting session...");

    if (!lightdm_greeter_start_session_sync (greeter, sessionKey, &error)) {
        greeter_ui_set_message (ui, error != NULL ? error->message : "Failed to start session.");
        greeter_ui_update_login_button (ui);
    }
}

static void lightdm_reset_cb (LightDMGreeter* greeter, gpointer userData)
{
    GreeterUi* ui = userData;

    (void) greeter;

    ui->awaitingPrompt = FALSE;
    graceful_greeter_login_model_clear_secret (ui->model);
    gtk_editable_set_text (GTK_EDITABLE (ui->passwordEntry), "");
    gtk_label_set_text (GTK_LABEL (ui->promptLabel), "");
    greeter_ui_set_message (ui, "");
    greeter_ui_update_login_button (ui);
}

static void greeter_ui_free (gpointer data)
{
    GreeterUi* ui = data;

    g_clear_object (&ui->greeter);
    g_clear_object (&ui->model);
    g_clear_pointer (&ui->userNames, g_ptr_array_unref);
    g_clear_pointer (&ui->sessionKeys, g_ptr_array_unref);
    g_free (ui);
}

static void lightdm_connect_complete_cb (GObject* object, GAsyncResult* result, gpointer userData)
{
    GreeterUi* ui = userData;
    g_autoptr(GError) error = NULL;

    if (!lightdm_greeter_connect_to_daemon_finish (LIGHTDM_GREETER (object), result, &error)) {
        greeter_ui_set_message (ui, error != NULL ? error->message : "Failed to connect to LightDM.");
        gtk_widget_set_sensitive (ui->userDropDown, FALSE);
        gtk_widget_set_sensitive (ui->usernameEntry, FALSE);
        gtk_widget_set_sensitive (ui->passwordEntry, FALSE);
        gtk_widget_set_sensitive (ui->sessionDropDown, FALSE);
        gtk_widget_set_sensitive (ui->loginButton, FALSE);
        return;
    }

    greeter_ui_populate_users (ui);
    greeter_ui_populate_sessions (ui);
    gtk_widget_set_sensitive (ui->userDropDown, TRUE);
    gtk_widget_set_sensitive (ui->usernameEntry, TRUE);
    gtk_widget_set_sensitive (ui->passwordEntry, TRUE);
    gtk_widget_set_sensitive (ui->sessionDropDown, TRUE);
    greeter_ui_set_message (ui, "");
    greeter_ui_update_login_button (ui);
}

static gboolean window_close_request_cb (GtkWindow* window, gpointer userData)
{
    GMainLoop* loop = userData;

    (void) window;

    g_main_loop_quit (loop);

    return TRUE;
}

static void greeter_ui_new (GMainLoop* loop)
{
    GreeterUi* ui = g_new0 (GreeterUi, 1);
    GtkWidget* content = NULL;
    GtkWidget* title = NULL;

    ui->greeter = lightdm_greeter_new ();
    ui->model = graceful_greeter_login_model_new ();
    ui->userNames = g_ptr_array_new_with_free_func (g_free);
    ui->sessionKeys = g_ptr_array_new_with_free_func (g_free);

    ui->window = gtk_window_new ();
    gtk_window_set_title (GTK_WINDOW (ui->window), "Graceful");
    gtk_window_set_default_size (GTK_WINDOW (ui->window), 420, 320);
    g_object_set_data_full (G_OBJECT (ui->window), "greeter-ui", ui, greeter_ui_free);
    g_signal_connect (ui->window, "close-request", G_CALLBACK (window_close_request_cb), loop);

    content = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top (content, 36);
    gtk_widget_set_margin_bottom (content, 36);
    gtk_widget_set_margin_start (content, 36);
    gtk_widget_set_margin_end (content, 36);
    gtk_window_set_child (GTK_WINDOW (ui->window), content);

    title = gtk_label_new ("Graceful");
    gtk_widget_add_css_class (title, "title-1");
    gtk_box_append (GTK_BOX (content), title);

    ui->promptLabel = gtk_label_new ("");
    gtk_label_set_wrap (GTK_LABEL (ui->promptLabel), TRUE);
    gtk_box_append (GTK_BOX (content), ui->promptLabel);

    ui->userDropDown = gtk_drop_down_new (NULL, NULL);
    gtk_box_append (GTK_BOX (content), ui->userDropDown);

    ui->usernameEntry = gtk_entry_new ();
    gtk_entry_set_placeholder_text (GTK_ENTRY (ui->usernameEntry), "Username");
    gtk_box_append (GTK_BOX (content), ui->usernameEntry);

    ui->passwordEntry = gtk_password_entry_new ();
    gtk_password_entry_set_show_peek_icon (GTK_PASSWORD_ENTRY (ui->passwordEntry), TRUE);
    gtk_editable_set_text (GTK_EDITABLE (ui->passwordEntry), "");
    gtk_box_append (GTK_BOX (content), ui->passwordEntry);

    ui->sessionDropDown = gtk_drop_down_new (NULL, NULL);
    gtk_box_append (GTK_BOX (content), ui->sessionDropDown);

    ui->loginButton = gtk_button_new_with_label ("Log In");
    gtk_widget_set_sensitive (ui->loginButton, FALSE);
    gtk_box_append (GTK_BOX (content), ui->loginButton);

    ui->messageLabel = gtk_label_new ("");
    gtk_label_set_wrap (GTK_LABEL (ui->messageLabel), TRUE);
    gtk_box_append (GTK_BOX (content), ui->messageLabel);

    g_signal_connect (ui->usernameEntry, "changed", G_CALLBACK (username_changed_cb), ui);
    g_signal_connect (ui->passwordEntry, "changed", G_CALLBACK (password_changed_cb), ui);
    g_signal_connect (ui->userDropDown, "notify::selected", G_CALLBACK (user_selected_cb), ui);
    g_signal_connect (ui->sessionDropDown, "notify::selected", G_CALLBACK (session_selected_cb), ui);
    g_signal_connect (ui->loginButton, "clicked", G_CALLBACK (login_clicked_cb), ui);
    g_signal_connect (ui->greeter, "show-prompt", G_CALLBACK (lightdm_show_prompt_cb), ui);
    g_signal_connect (ui->greeter, "show-message", G_CALLBACK (lightdm_show_message_cb), ui);
    g_signal_connect (ui->greeter, "authentication-complete", G_CALLBACK (lightdm_authentication_complete_cb), ui);
    g_signal_connect (ui->greeter, "reset", G_CALLBACK (lightdm_reset_cb), ui);

    greeter_ui_set_message (ui, "Connecting to LightDM...");
    gtk_widget_set_sensitive (ui->userDropDown, FALSE);
    gtk_widget_set_sensitive (ui->usernameEntry, FALSE);
    gtk_widget_set_sensitive (ui->passwordEntry, FALSE);
    gtk_widget_set_sensitive (ui->sessionDropDown, FALSE);
    gtk_widget_set_sensitive (ui->loginButton, FALSE);
    gtk_window_present (GTK_WINDOW (ui->window));
    lightdm_greeter_connect_to_daemon (ui->greeter, NULL, lightdm_connect_complete_cb, ui);
}

int main (int argc, char* argv[])
{
    g_autoptr(GMainLoop) loop = NULL;

    g_setenv ("GTK_USE_PORTAL", "0", TRUE);
    gtk_init ();

    loop = g_main_loop_new (NULL, FALSE);
    greeter_ui_new (loop);
    g_main_loop_run (loop);

    (void) argc;
    (void) argv;

    return 0;
}
