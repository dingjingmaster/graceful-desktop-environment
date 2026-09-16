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
#include "greeter-config.h"
#include "greeter-login-model.h"

#include <gdk/x11/gdkx.h>
#include <gtk/gtk.h>
#include <lightdm.h>
#include <X11/cursorfont.h>

typedef struct _GreeterUi GreeterUi;

struct _GreeterUi
{
    GtkWidget* window;
    GtkWidget* userDropDown;
    GtkWidget* passwordEntry;
    GtkWidget* sessionDropDown;
    GtkWidget* promptLabel;
    GtkWidget* messageLabel;
    GtkWidget* loginButton;

    GracefulGreeterConfig* config;
    LightDMGreeter* greeter;
    GracefulGreeterLoginModel* model;
    GPtrArray* userNames;
    GPtrArray* sessionKeys;
    guint geometryWatchId;
    int monitorWidth;
    int monitorHeight;
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
    gtk_editable_set_text (GTK_EDITABLE (ui->passwordEntry), "");
    greeter_ui_update_login_button (ui);
}

static void greeter_ui_sync_password (GreeterUi* ui)
{
    graceful_greeter_login_model_set_password (ui->model, gtk_editable_get_text (GTK_EDITABLE (ui->passwordEntry)));
}

static void greeter_ui_respond_to_prompt (GreeterUi* ui)
{
    g_autoptr(GError) error = NULL;
    const char* password = NULL;

    if (!ui->awaitingPrompt) {
        return;
    }

    greeter_ui_sync_password (ui);
    password = graceful_greeter_login_model_get_password (ui->model);

    if (password == NULL) {
        greeter_ui_set_message (ui, "Password is required.");
        return;
    }

    ui->awaitingPrompt = FALSE;
    gtk_widget_set_sensitive (ui->loginButton, FALSE);
    g_message ("Responding to LightDM secret prompt for user '%s' with password length %" G_GSIZE_FORMAT,
        graceful_greeter_login_model_get_username (ui->model),
        strlen (password)
    );

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
    const char* username = NULL;

    greeter_ui_sync_password (ui);
    username = graceful_greeter_login_model_get_username (ui->model);

    if (!graceful_greeter_login_model_can_authenticate (ui->model)) {
        greeter_ui_set_message (ui, "Select a user, enter a password, and choose a session.");
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

static void password_changed_cb (GtkEditable* editable, gpointer userData)
{
    GreeterUi* ui = userData;

    (void) editable;

    greeter_ui_sync_password (ui);
    greeter_ui_update_login_button (ui);
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

static gboolean password_key_pressed_cb (
    GtkEventControllerKey* controller,
    guint keyval,
    guint keycode,
    GdkModifierType state,
    gpointer userData
)
{
    GreeterUi* ui = userData;

    (void) controller;
    (void) keycode;
    (void) state;

    if (keyval != GDK_KEY_Return && keyval != GDK_KEY_KP_Enter) {
        return FALSE;
    }

    greeter_ui_authenticate (ui);

    return TRUE;
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

    ui->awaitingPrompt = TRUE;

    if (type == LIGHTDM_PROMPT_TYPE_SECRET) {
        gtk_label_set_text (GTK_LABEL (ui->promptLabel), "");
        gtk_widget_grab_focus (ui->passwordEntry);
    }
    else {
        gtk_label_set_text (GTK_LABEL (ui->promptLabel), text != NULL ? text : "");
        gtk_widget_grab_focus (ui->userDropDown);
    }

    g_message ("LightDM prompt received: type=%s", type == LIGHTDM_PROMPT_TYPE_SECRET ? "secret" : "question");

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
        g_message ("LightDM authentication failed");
        greeter_ui_update_login_button (ui);
        return;
    }

    greeter_ui_set_message (ui, "Starting session...");
    g_message ("LightDM authentication succeeded; starting session '%s'", sessionKey != NULL ? sessionKey : "");

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
    g_clear_object (&ui->config);
    g_clear_object (&ui->model);
    g_clear_handle_id (&ui->geometryWatchId, g_source_remove);
    g_clear_pointer (&ui->userNames, g_ptr_array_unref);
    g_clear_pointer (&ui->sessionKeys, g_ptr_array_unref);
    g_free (ui);
}

static void greeter_ui_load_css (void)
{
    GtkCssProvider* provider = gtk_css_provider_new ();
    GdkDisplay* display = gdk_display_get_default ();

    gtk_css_provider_load_from_string (
        provider,
        ".greeter-root {"
        "  background: #111318;"
        "}"
        ".greeter-background {"
        "  background: #111318;"
        "}"
        ".login-card {"
        "  background: alpha(#151820, 0.88);"
        "  color: #f4f6fb;"
        "  border-radius: 8px;"
        "  padding: 28px;"
        "  box-shadow: 0 16px 48px alpha(#000000, 0.35);"
        "}"
        ".login-card entry,"
        ".login-card passwordentry,"
        ".login-card dropdown,"
        ".login-card button {"
        "  min-height: 38px;"
        "}"
        ".login-title {"
        "  font-size: 28px;"
        "  font-weight: 700;"
        "}"
        ".login-message {"
        "  color: #d6dbea;"
        "}"
    );

    gtk_style_context_add_provider_for_display (
        display,
        GTK_STYLE_PROVIDER (provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref (provider);
}

static GtkWidget* greeter_ui_create_background (GracefulGreeterConfig* config)
{
    const char* background = graceful_greeter_config_get_background (config);
    GtkWidget* picture = NULL;

    if (background != NULL && g_file_test (background, G_FILE_TEST_IS_REGULAR)) {
        picture = gtk_picture_new_for_filename (background);
        gtk_picture_set_content_fit (GTK_PICTURE (picture), GTK_CONTENT_FIT_COVER);
    }
    else {
        picture = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    }

    gtk_widget_add_css_class (picture, "greeter-background");
    gtk_widget_set_hexpand (picture, TRUE);
    gtk_widget_set_vexpand (picture, TRUE);

    return picture;
}

static void greeter_ui_apply_cursor (GtkWidget* widget)
{
    gtk_widget_set_cursor_from_name (widget, "default");
}

static gboolean greeter_ui_apply_monitor_geometry (GreeterUi* ui)
{
    GdkDisplay* display = gdk_display_get_default ();
    GListModel* monitors = NULL;
    g_autoptr(GdkMonitor) monitor = NULL;
    GdkRectangle geometry = { 0 };
    GdkSurface* surface = NULL;
    Display* xdisplay = NULL;
    Window xroot = 0;
    Window xwindow = 0;
    Cursor xcursor = 0;
    XWindowAttributes xrootAttributes = { 0 };

    if (display == NULL) {
        return G_SOURCE_CONTINUE;
    }

    if (GDK_IS_X11_DISPLAY (display)) {
        xdisplay = gdk_x11_display_get_xdisplay (display);
        xroot = gdk_x11_display_get_xrootwindow (display);

        if (XGetWindowAttributes (xdisplay, xroot, &xrootAttributes) != 0) {
            geometry.x = 0;
            geometry.y = 0;
            geometry.width = xrootAttributes.width;
            geometry.height = xrootAttributes.height;
        }
    }
    else {
        monitors = gdk_display_get_monitors (display);
        if (g_list_model_get_n_items (monitors) == 0) {
            return G_SOURCE_CONTINUE;
        }

        monitor = g_list_model_get_item (monitors, 0);
        gdk_monitor_get_geometry (monitor, &geometry);
    }

    if (geometry.width <= 0 || geometry.height <= 0) {
        return G_SOURCE_CONTINUE;
    }

    ui->monitorWidth = geometry.width;
    ui->monitorHeight = geometry.height;
    gtk_window_set_default_size (GTK_WINDOW (ui->window), geometry.width, geometry.height);
    gtk_widget_set_size_request (ui->window, geometry.width, geometry.height);

    surface = gtk_native_get_surface (GTK_NATIVE (ui->window));
    if (surface != NULL && GDK_IS_X11_SURFACE (surface)) {
        xdisplay = GDK_SURFACE_XDISPLAY (surface);
        xwindow = gdk_x11_surface_get_xid (surface);
        xcursor = XCreateFontCursor (xdisplay, XC_left_ptr);
        XMoveResizeWindow (xdisplay, xwindow, 0, 0, (unsigned int) geometry.width, (unsigned int) geometry.height);
        if (xcursor != 0) {
            XDefineCursor (xdisplay, xroot != 0 ? xroot : DefaultRootWindow (xdisplay), xcursor);
            XDefineCursor (xdisplay, xwindow, xcursor);
            XFreeCursor (xdisplay, xcursor);
        }
        XMapRaised (xdisplay, xwindow);
        XFlush (xdisplay);
    }

    return G_SOURCE_CONTINUE;
}

static void lightdm_connect_complete_cb (GObject* object, GAsyncResult* result, gpointer userData)
{
    GreeterUi* ui = userData;
    g_autoptr(GError) error = NULL;

    if (!lightdm_greeter_connect_to_daemon_finish (LIGHTDM_GREETER (object), result, &error)) {
        greeter_ui_set_message (ui, error != NULL ? error->message : "Failed to connect to LightDM.");
        gtk_widget_set_sensitive (ui->userDropDown, FALSE);
        gtk_widget_set_sensitive (ui->passwordEntry, FALSE);
        gtk_widget_set_sensitive (ui->sessionDropDown, FALSE);
        gtk_widget_set_sensitive (ui->loginButton, FALSE);
        return;
    }

    greeter_ui_populate_users (ui);
    greeter_ui_populate_sessions (ui);
    gtk_widget_set_sensitive (ui->userDropDown, TRUE);
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
    GtkWidget* overlay = NULL;
    GtkWidget* background = NULL;
    GtkWidget* card = NULL;
    GtkWidget* content = NULL;
    GtkWidget* title = NULL;
    GtkEventController* passwordKeyController = NULL;

    greeter_ui_load_css ();

    ui->config = graceful_greeter_config_new ();
    ui->greeter = lightdm_greeter_new ();
    ui->model = graceful_greeter_login_model_new ();
    ui->userNames = g_ptr_array_new_with_free_func (g_free);
    ui->sessionKeys = g_ptr_array_new_with_free_func (g_free);

    ui->window = gtk_window_new ();
    gtk_window_set_title (GTK_WINDOW (ui->window), "Graceful");
    gtk_window_set_decorated (GTK_WINDOW (ui->window), FALSE);
    gtk_window_set_default_size (GTK_WINDOW (ui->window), 1024, 768);
    gtk_widget_add_css_class (ui->window, "greeter-root");
    greeter_ui_apply_cursor (ui->window);
    g_object_set_data_full (G_OBJECT (ui->window), "greeter-ui", ui, greeter_ui_free);
    g_signal_connect (ui->window, "close-request", G_CALLBACK (window_close_request_cb), loop);

    overlay = gtk_overlay_new ();
    gtk_widget_set_hexpand (overlay, TRUE);
    gtk_widget_set_vexpand (overlay, TRUE);
    greeter_ui_apply_cursor (overlay);
    gtk_window_set_child (GTK_WINDOW (ui->window), overlay);

    background = greeter_ui_create_background (ui->config);
    greeter_ui_apply_cursor (background);
    gtk_overlay_set_child (GTK_OVERLAY (overlay), background);

    card = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class (card, "login-card");
    greeter_ui_apply_cursor (card);
    gtk_widget_set_halign (card, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (card, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (card, 24);
    gtk_widget_set_margin_bottom (card, 24);
    gtk_widget_set_margin_start (card, 24);
    gtk_widget_set_margin_end (card, 24);
    gtk_widget_set_size_request (card, 360, -1);
    gtk_overlay_add_overlay (GTK_OVERLAY (overlay), card);

    content = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_hexpand (content, TRUE);
    gtk_box_append (GTK_BOX (card), content);

    title = gtk_label_new ("Graceful");
    gtk_widget_add_css_class (title, "login-title");
    gtk_box_append (GTK_BOX (content), title);

    ui->promptLabel = gtk_label_new ("");
    gtk_label_set_wrap (GTK_LABEL (ui->promptLabel), TRUE);
    gtk_box_append (GTK_BOX (content), ui->promptLabel);

    ui->userDropDown = gtk_drop_down_new (NULL, NULL);
    gtk_box_append (GTK_BOX (content), ui->userDropDown);

    ui->passwordEntry = gtk_password_entry_new ();
    gtk_password_entry_set_show_peek_icon (GTK_PASSWORD_ENTRY (ui->passwordEntry), TRUE);
    gtk_editable_set_text (GTK_EDITABLE (ui->passwordEntry), "");
    passwordKeyController = gtk_event_controller_key_new ();
    g_signal_connect (passwordKeyController, "key-pressed", G_CALLBACK (password_key_pressed_cb), ui);
    gtk_widget_add_controller (ui->passwordEntry, passwordKeyController);
    gtk_box_append (GTK_BOX (content), ui->passwordEntry);

    ui->sessionDropDown = gtk_drop_down_new (NULL, NULL);
    gtk_box_append (GTK_BOX (content), ui->sessionDropDown);

    ui->loginButton = gtk_button_new_with_label ("Log In");
    gtk_widget_set_sensitive (ui->loginButton, FALSE);
    gtk_box_append (GTK_BOX (content), ui->loginButton);

    ui->messageLabel = gtk_label_new ("");
    gtk_widget_add_css_class (ui->messageLabel, "login-message");
    gtk_label_set_wrap (GTK_LABEL (ui->messageLabel), TRUE);
    gtk_box_append (GTK_BOX (content), ui->messageLabel);

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
    gtk_widget_set_sensitive (ui->passwordEntry, FALSE);
    gtk_widget_set_sensitive (ui->sessionDropDown, FALSE);
    gtk_widget_set_sensitive (ui->loginButton, FALSE);
    greeter_ui_apply_monitor_geometry (ui);
    ui->geometryWatchId = g_timeout_add_seconds (1, (GSourceFunc) greeter_ui_apply_monitor_geometry, ui);
    gtk_window_fullscreen (GTK_WINDOW (ui->window));
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
