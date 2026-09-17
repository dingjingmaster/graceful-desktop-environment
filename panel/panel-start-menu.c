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

#define PINNED_APP_LIMIT 6
#define SEARCH_RESULT_LIMIT 12

struct _GracefulPanelStartMenu
{
    GtkPopover parentInstance;

    GracefulPanelAppIndex* appIndex;
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

static GtkWidget* create_pinned_button (GracefulPanelStartMenu* self, GracefulPanelAppEntry* entry)
{
    GtkWidget* button = gtk_button_new ();
    GtkWidget* box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget* icon = create_app_icon (entry, GTK_ICON_SIZE_LARGE);
    GtkWidget* label = gtk_label_new (entry->name);

    gtk_widget_add_css_class (button, "start-menu-pinned-button");
    gtk_widget_add_css_class (label, "start-menu-app-label");
    gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars (GTK_LABEL (label), 12);
    gtk_box_append (GTK_BOX (box), icon);
    gtk_box_append (GTK_BOX (box), label);
    gtk_button_set_child (GTK_BUTTON (button), box);
    g_object_set_data_full (G_OBJECT (button), "app-entry", graceful_panel_app_entry_copy (entry), app_entry_data_free);
    g_signal_connect (button, "clicked", G_CALLBACK (launch_entry_and_close), self);

    return button;
}

static GtkWidget* create_app_row (GracefulPanelStartMenu* self, GracefulPanelAppEntry* entry)
{
    GtkWidget* button = gtk_button_new ();
    GtkWidget* box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* icon = create_app_icon (entry, GTK_ICON_SIZE_NORMAL);
    GtkWidget* label = gtk_label_new (entry->name);

    gtk_widget_add_css_class (button, "start-menu-app-row");
    gtk_widget_add_css_class (label, "start-menu-row-label");
    gtk_widget_set_hexpand (label, TRUE);
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
    gtk_box_append (GTK_BOX (box), icon);
    gtk_box_append (GTK_BOX (box), label);
    gtk_button_set_child (GTK_BUTTON (button), box);
    g_object_set_data_full (G_OBJECT (button), "app-entry", graceful_panel_app_entry_copy (entry), app_entry_data_free);
    g_signal_connect (button, "clicked", G_CALLBACK (launch_entry_and_close), self);

    return button;
}

static void populate_pinned_apps (GracefulPanelStartMenu* self)
{
    g_autoptr(GPtrArray) pinned = graceful_panel_app_index_get_pinned (self->appIndex, PINNED_APP_LIMIT);
    guint i = 0;

    clear_box_children (self->pinnedGrid);
    for (i = 0; i < pinned->len; ++i) {
        GracefulPanelAppEntry* entry = g_ptr_array_index (pinned, i);

        gtk_box_append (GTK_BOX (self->pinnedGrid), create_pinned_button (self, entry));
    }
}

static void populate_all_apps (GracefulPanelStartMenu* self, const char* query)
{
    g_autoptr(GPtrArray) results = graceful_panel_app_index_search (self->appIndex, query, SEARCH_RESULT_LIMIT);
    guint i = 0;

    clear_box_children (self->allAppsList);
    for (i = 0; i < results->len; ++i) {
        GracefulPanelAppEntry* entry = g_ptr_array_index (results, i);

        gtk_box_append (GTK_BOX (self->allAppsList), create_app_row (self, entry));
    }
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
    self->searchEntry = gtk_search_entry_new ();
    self->pinnedGrid = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
    self->allAppsList = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);

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

    populate_pinned_apps (self);
    populate_all_apps (self, NULL);
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
    populate_pinned_apps (self);
    populate_all_apps (self, gtk_editable_get_text (GTK_EDITABLE (self->searchEntry)));
    gtk_popover_popup (GTK_POPOVER (self));
    gtk_widget_grab_focus (self->searchEntry);
}
