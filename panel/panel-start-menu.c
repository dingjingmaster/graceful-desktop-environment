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
#include "panel-start-menu.h"

#include "panel-app-index.h"
#include "panel-pinned-apps.h"

#define PINNED_APP_LIMIT 6
#define SEARCH_RESULT_LIMIT 12

struct _GracefulPanelStartMenu
{
    GtkPopover parentInstance;

    GracefulPanelAppIndex* appIndex;
    GracefulPanelPinnedApps* pinnedApps;
    GtkWidget* searchEntry;
    GtkWidget* pinnedGrid;
    GtkWidget* allAppsList;
};

G_DEFINE_TYPE (GracefulPanelStartMenu, graceful_panel_start_menu, GTK_TYPE_POPOVER)

static void clear_box_children (GtkWidget* box)
{
    GtkWidget* child = gtk_widget_get_first_child (box);

    while (child != NULL) {
        GtkWidget* next = gtk_widget_get_next_sibling (child);

        gtk_box_remove (GTK_BOX (box), child);
        child = next;
    }
}

static void launch_entry_and_close (GtkWidget* widget, gpointer userData)
{
    GracefulPanelStartMenu* self = GRACEFUL_PANEL_START_MENU (userData);
    GracefulPanelAppEntry* entry = g_object_get_data (G_OBJECT (widget), "app-entry");
    g_autoptr(GError) error = NULL;

    if (entry != NULL && !graceful_panel_app_entry_launch (entry, &error)) {
        g_warning ("Failed to launch %s: %s", entry->name, error->message);
    }

    gtk_popover_popdown (GTK_POPOVER (self));
}

static void launch_row_entry_and_close (GtkWidget* widget, GracefulPanelStartMenu* self)
{
    GracefulPanelAppEntry* entry = g_object_get_data (G_OBJECT (widget), "app-entry");
    g_autoptr(GError) error = NULL;

    if (entry != NULL && !graceful_panel_app_entry_launch (entry, &error)) {
        g_warning ("Failed to launch %s: %s", entry->name, error->message);
    }

    gtk_popover_popdown (GTK_POPOVER (self));
}

static GtkWidget* create_app_icon (GracefulPanelAppEntry* entry, GtkIconSize iconSize)
{
    GtkWidget* image = NULL;

    if (entry->icon != NULL) {
        image = gtk_image_new_from_gicon (entry->icon);
    }
    else {
        image = gtk_image_new_from_icon_name ("application-x-executable-symbolic");
    }

    gtk_image_set_icon_size (GTK_IMAGE (image), iconSize);

    return image;
}

static void app_entry_data_free (gpointer userData)
{
    graceful_panel_app_entry_free ((GracefulPanelAppEntry*) userData);
}

static void refresh_menu_contents (GracefulPanelStartMenu* self);
static GtkWidget* create_section_title (const char* title);

static void on_context_popover_closed (GtkPopover* popover, gpointer userData)
{
    gtk_widget_unparent (GTK_WIDGET (popover));
}

static void on_pin_menu_item_clicked (GtkButton* button, gpointer userData)
{
    GracefulPanelStartMenu* self = GRACEFUL_PANEL_START_MENU (userData);
    GracefulPanelAppEntry* entry = g_object_get_data (G_OBJECT (button), "app-entry");
    GtkWidget* popover = g_object_get_data (G_OBJECT (button), "context-popover");

    if (entry != NULL) {
        graceful_panel_pinned_apps_pin (self->pinnedApps, entry->id);
        refresh_menu_contents (self);
    }
    if (popover != NULL) {
        gtk_popover_popdown (GTK_POPOVER (popover));
    }
}

static void on_unpin_menu_item_clicked (GtkButton* button, gpointer userData)
{
    GracefulPanelStartMenu* self = GRACEFUL_PANEL_START_MENU (userData);
    GracefulPanelAppEntry* entry = g_object_get_data (G_OBJECT (button), "app-entry");
    GtkWidget* popover = g_object_get_data (G_OBJECT (button), "context-popover");

    if (entry != NULL) {
        graceful_panel_pinned_apps_unpin (self->pinnedApps, entry->id);
        refresh_menu_contents (self);
    }
    if (popover != NULL) {
        gtk_popover_popdown (GTK_POPOVER (popover));
    }
}

static void show_app_context_menu (
    GracefulPanelStartMenu* self,
    GtkWidget* relativeTo,
    GracefulPanelAppEntry* entry,
    gboolean pinned,
    double x,
    double y
)
{
    GdkRectangle pointingTo = { (int) x, (int) y, 1, 1 };
    GtkWidget* popover = gtk_popover_new ();
    GtkWidget* box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* item = gtk_button_new_with_label (pinned ? "Unpin from menu" : "Pin to menu");

    gtk_widget_add_css_class (popover, "start-menu-context-popover");
    gtk_widget_add_css_class (box, "start-menu-context");
    gtk_widget_add_css_class (item, "start-menu-context-item");
    gtk_box_append (GTK_BOX (box), item);
    gtk_popover_set_child (GTK_POPOVER (popover), box);
    gtk_popover_set_has_arrow (GTK_POPOVER (popover), FALSE);
    gtk_popover_set_pointing_to (GTK_POPOVER (popover), &pointingTo);
    gtk_popover_set_position (GTK_POPOVER (popover), GTK_POS_BOTTOM);
    gtk_widget_set_parent (popover, relativeTo);
    g_object_set_data_full (G_OBJECT (item), "app-entry", graceful_panel_app_entry_copy (entry), app_entry_data_free);
    g_object_set_data (G_OBJECT (item), "context-popover", popover);
    g_signal_connect (
        item,
        "clicked",
        G_CALLBACK (pinned ? on_unpin_menu_item_clicked : on_pin_menu_item_clicked),
        self
    );
    g_signal_connect (popover, "closed", G_CALLBACK (on_context_popover_closed), NULL);
    gtk_popover_popup (GTK_POPOVER (popover));
}

static void on_pinned_button_pressed (
    GtkGestureClick* gesture,
    int pressCount,
    double x,
    double y,
    gpointer userData
)
{
    GracefulPanelStartMenu* self = GRACEFUL_PANEL_START_MENU (userData);
    GtkWidget* button = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture));
    GracefulPanelAppEntry* entry = g_object_get_data (G_OBJECT (button), "app-entry");

    if (entry != NULL) {
        show_app_context_menu (self, button, entry, TRUE, x, y);
    }
}

static void on_app_row_primary_pressed (
    GtkGestureClick* gesture,
    int pressCount,
    double x,
    double y,
    gpointer userData
)
{
    GracefulPanelStartMenu* self = GRACEFUL_PANEL_START_MENU (userData);
    GtkWidget* row = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture));

    if (pressCount == 1) {
        launch_row_entry_and_close (row, self);
    }
}

static void on_app_row_secondary_pressed (
    GtkGestureClick* gesture,
    int pressCount,
    double x,
    double y,
    gpointer userData
)
{
    GracefulPanelStartMenu* self = GRACEFUL_PANEL_START_MENU (userData);
    GtkWidget* row = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture));
    GracefulPanelAppEntry* entry = g_object_get_data (G_OBJECT (row), "app-entry");

    if (entry != NULL) {
        show_app_context_menu (self, row, entry, FALSE, x, y);
    }
}

static GtkWidget* create_pinned_button (GracefulPanelStartMenu* self, GracefulPanelAppEntry* entry)
{
    GtkWidget* button = gtk_button_new ();
    GtkWidget* box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget* icon = create_app_icon (entry, GTK_ICON_SIZE_LARGE);
    GtkWidget* label = gtk_label_new (entry->name);
    GtkGesture* secondaryClick = gtk_gesture_click_new ();

    gtk_widget_add_css_class (button, "start-menu-pinned-button");
    gtk_widget_add_css_class (label, "start-menu-app-label");
    gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars (GTK_LABEL (label), 12);
    gtk_box_append (GTK_BOX (box), icon);
    gtk_box_append (GTK_BOX (box), label);
    gtk_button_set_child (GTK_BUTTON (button), box);
    g_object_set_data_full (G_OBJECT (button), "app-entry", graceful_panel_app_entry_copy (entry), app_entry_data_free);
    g_signal_connect (button, "clicked", G_CALLBACK (launch_entry_and_close), self);
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (secondaryClick), GDK_BUTTON_SECONDARY);
    g_signal_connect (secondaryClick, "pressed", G_CALLBACK (on_pinned_button_pressed), self);
    gtk_widget_add_controller (button, GTK_EVENT_CONTROLLER (secondaryClick));

    return button;
}

static GtkWidget* create_app_row (GracefulPanelStartMenu* self, GracefulPanelAppEntry* entry)
{
    GtkWidget* button = gtk_button_new ();
    GtkWidget* box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* icon = create_app_icon (entry, GTK_ICON_SIZE_NORMAL);
    GtkWidget* label = gtk_label_new (entry->name);
    GtkGesture* primaryClick = gtk_gesture_click_new ();
    GtkGesture* secondaryClick = gtk_gesture_click_new ();

    gtk_widget_add_css_class (button, "start-menu-app-row");
    gtk_widget_add_css_class (label, "start-menu-row-label");
    gtk_widget_set_hexpand (label, TRUE);
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
    gtk_box_append (GTK_BOX (box), icon);
    gtk_box_append (GTK_BOX (box), label);
    gtk_button_set_child (GTK_BUTTON (button), box);
    g_object_set_data_full (G_OBJECT (button), "app-entry", graceful_panel_app_entry_copy (entry), app_entry_data_free);
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (primaryClick), GDK_BUTTON_PRIMARY);
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (secondaryClick), GDK_BUTTON_SECONDARY);
    g_signal_connect (primaryClick, "pressed", G_CALLBACK (on_app_row_primary_pressed), self);
    g_signal_connect (secondaryClick, "pressed", G_CALLBACK (on_app_row_secondary_pressed), self);
    gtk_widget_add_controller (button, GTK_EVENT_CONTROLLER (primaryClick));
    gtk_widget_add_controller (button, GTK_EVENT_CONTROLLER (secondaryClick));

    return button;
}

static void ensure_default_pins (GracefulPanelStartMenu* self)
{
    g_autoptr(GPtrArray) defaultPinned = NULL;
    guint i = 0;

    if (graceful_panel_pinned_apps_has_file (self->pinnedApps) ||
        !graceful_panel_pinned_apps_is_empty (self->pinnedApps)) {
        return;
    }

    defaultPinned = graceful_panel_app_index_get_pinned (self->appIndex, PINNED_APP_LIMIT);
    for (i = 0; i < defaultPinned->len; ++i) {
        GracefulPanelAppEntry* entry = g_ptr_array_index (defaultPinned, i);

        graceful_panel_pinned_apps_pin (self->pinnedApps, entry->id);
    }
}

static void populate_pinned_apps (GracefulPanelStartMenu* self)
{
    g_auto(GStrv) pinnedIds = NULL;
    guint i = 0;

    clear_box_children (self->pinnedGrid);
    pinnedIds = graceful_panel_pinned_apps_dup_ids (self->pinnedApps);
    for (i = 0; pinnedIds != NULL && pinnedIds[i] != NULL; ++i) {
        g_autoptr(GracefulPanelAppEntry) entry = graceful_panel_app_index_find_by_id (
            self->appIndex,
            pinnedIds[i]
        );

        if (entry != NULL) {
            gtk_box_append (GTK_BOX (self->pinnedGrid), create_pinned_button (self, entry));
        }
    }
}

static gboolean query_is_empty (const char* query)
{
    return query == NULL || query[0] == '\0';
}

static const char* const menuCategories[] = {
    "Internet",
    "Office",
    "Development",
    "Graphics",
    "Multimedia",
    "Games",
    "System",
    "Utilities",
    "Other"
};

static void append_category_apps (GracefulPanelStartMenu* self, const char* category, GPtrArray* entries)
{
    gboolean hasCategory = FALSE;
    guint i = 0;

    for (i = 0; i < entries->len; ++i) {
        GracefulPanelAppEntry* entry = g_ptr_array_index (entries, i);

        if (g_strcmp0 (entry->menuCategory, category) == 0) {
            if (!hasCategory) {
                gtk_box_append (GTK_BOX (self->allAppsList), create_section_title (category));
                hasCategory = TRUE;
            }
            gtk_box_append (GTK_BOX (self->allAppsList), create_app_row (self, entry));
        }
    }
}

static void populate_all_apps (GracefulPanelStartMenu* self, const char* query)
{
    g_autoptr(GPtrArray) results = graceful_panel_app_index_search (
        self->appIndex,
        query,
        query_is_empty (query) ? 0 : SEARCH_RESULT_LIMIT
    );
    guint i = 0;

    clear_box_children (self->allAppsList);
    if (!query_is_empty (query)) {
        for (i = 0; i < results->len; ++i) {
            GracefulPanelAppEntry* entry = g_ptr_array_index (results, i);

            gtk_box_append (GTK_BOX (self->allAppsList), create_app_row (self, entry));
        }
        return;
    }

    for (i = 0; i < G_N_ELEMENTS (menuCategories); ++i) {
        append_category_apps (self, menuCategories[i], results);
    }
}

static void refresh_menu_contents (GracefulPanelStartMenu* self)
{
    ensure_default_pins (self);
    populate_pinned_apps (self);
    populate_all_apps (self, gtk_editable_get_text (GTK_EDITABLE (self->searchEntry)));
}

static void on_search_changed (GtkEditable* editable, gpointer userData)
{
    GracefulPanelStartMenu* self = GRACEFUL_PANEL_START_MENU (userData);

    populate_all_apps (self, gtk_editable_get_text (editable));
}

static GtkWidget* create_section_title (const char* title)
{
    GtkWidget* label = gtk_label_new (title);

    gtk_widget_add_css_class (label, "start-menu-section-title");
    gtk_widget_set_halign (label, GTK_ALIGN_START);

    return label;
}

static void graceful_panel_start_menu_dispose (GObject* object)
{
    GracefulPanelStartMenu* self = GRACEFUL_PANEL_START_MENU (object);

    g_clear_object (&self->appIndex);
    g_clear_object (&self->pinnedApps);

    G_OBJECT_CLASS (graceful_panel_start_menu_parent_class)->dispose (object);
}

static void graceful_panel_start_menu_class_init (GracefulPanelStartMenuClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_start_menu_dispose;
}

static void graceful_panel_start_menu_init (GracefulPanelStartMenu* self)
{
    GtkWidget* root = gtk_box_new (GTK_ORIENTATION_VERTICAL, 14);
    GtkWidget* scrolled = gtk_scrolled_window_new ();

    self->appIndex = graceful_panel_app_index_new ();
    self->pinnedApps = graceful_panel_pinned_apps_new ();
    self->searchEntry = gtk_search_entry_new ();
    self->pinnedGrid = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
    self->allAppsList = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);

    gtk_widget_add_css_class (GTK_WIDGET (self), "start-menu-popover");
    gtk_widget_add_css_class (root, "start-menu-root");
    gtk_widget_add_css_class (self->searchEntry, "start-menu-search");
    gtk_widget_add_css_class (self->pinnedGrid, "start-menu-pinned-grid");
    gtk_widget_add_css_class (self->allAppsList, "start-menu-app-list");
    gtk_widget_set_size_request (root, 460, 520);
    gtk_widget_set_size_request (scrolled, 420, 220);
    gtk_popover_set_has_arrow (GTK_POPOVER (self), FALSE);
    gtk_popover_set_position (GTK_POPOVER (self), GTK_POS_TOP);
    gtk_search_entry_set_placeholder_text (GTK_SEARCH_ENTRY (self->searchEntry), "Search apps");
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), self->allAppsList);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand (scrolled, TRUE);

    gtk_box_append (GTK_BOX (root), self->searchEntry);
    gtk_box_append (GTK_BOX (root), create_section_title ("Pinned"));
    gtk_box_append (GTK_BOX (root), self->pinnedGrid);
    gtk_box_append (GTK_BOX (root), create_section_title ("All apps"));
    gtk_box_append (GTK_BOX (root), scrolled);

    gtk_popover_set_child (GTK_POPOVER (self), root);
    g_signal_connect (self->searchEntry, "search-changed", G_CALLBACK (on_search_changed), self);

    refresh_menu_contents (self);
}

GtkWidget* graceful_panel_start_menu_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_START_MENU, NULL);
}

void graceful_panel_start_menu_popup_for_button (GracefulPanelStartMenu* self, GtkWidget* button)
{
    g_return_if_fail (GRACEFUL_IS_PANEL_START_MENU (self));
    g_return_if_fail (GTK_IS_WIDGET (button));

    if (gtk_widget_get_parent (GTK_WIDGET (self)) == NULL) {
        gtk_widget_set_parent (GTK_WIDGET (self), button);
    }

    graceful_panel_app_index_reload (self->appIndex);
    refresh_menu_contents (self);
    gtk_popover_popup (GTK_POPOVER (self));
    gtk_widget_grab_focus (self->searchEntry);
}
