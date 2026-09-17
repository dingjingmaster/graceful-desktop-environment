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
#include "panel-task-area.h"

#include "panel-window-list.h"

#define TASK_REFRESH_INTERVAL_MS 1000
#define TASK_PREVIEW_MAX_WIDTH 280
#define TASK_PREVIEW_MAX_HEIGHT 180

struct _GracefulPanelTaskArea
{
    GtkBox parentInstance;

    GracefulPanelWindowList* windowList;
    GPtrArray* currentTasks;
    guint refreshTimerId;
};

G_DEFINE_TYPE (GracefulPanelTaskArea, graceful_panel_task_area, GTK_TYPE_BOX)

static void preview_popover_data_free (gpointer userData);

static void clear_task_buttons (GracefulPanelTaskArea* self)
{
    GtkWidget* child = gtk_widget_get_first_child (GTK_WIDGET (self));

    while (child != NULL) {
        GtkWidget* next = gtk_widget_get_next_sibling (child);
        GtkWidget* popover = g_object_steal_data (G_OBJECT (child), "preview-popover");

        if (popover != NULL) {
            preview_popover_data_free (popover);
        }

        gtk_box_remove (GTK_BOX (self), child);
        child = next;
    }
}

static GtkWidget* create_task_icon (GracefulPanelWindowInfo* info)
{
    GtkWidget* icon = NULL;

    if (info->iconTexture != NULL) {
        icon = gtk_image_new_from_paintable (GDK_PAINTABLE (info->iconTexture));
    }
    else {
        icon = gtk_image_new_from_icon_name ("application-x-executable-symbolic");
    }

    gtk_image_set_icon_size (GTK_IMAGE (icon), GTK_ICON_SIZE_LARGE);

    return icon;
}

static void task_button_data_free (gpointer userData)
{
    graceful_panel_window_info_free ((GracefulPanelWindowInfo*) userData);
}

static void preview_popover_data_free (gpointer userData)
{
    GtkWidget* popover = GTK_WIDGET (userData);

    gtk_popover_popdown (GTK_POPOVER (popover));
    if (gtk_widget_get_parent (popover) != NULL) {
        gtk_widget_unparent (popover);
    }
    g_object_unref (popover);
}

static void set_preview_fallback (GtkWidget* box, GracefulPanelWindowInfo* info)
{
    GtkWidget* icon = create_task_icon (info);
    GtkWidget* label = gtk_label_new (graceful_panel_window_info_get_display_title (info));

    gtk_widget_add_css_class (icon, "panel-task-preview-fallback-icon");
    gtk_widget_add_css_class (label, "panel-task-preview-title");
    gtk_box_append (GTK_BOX (box), icon);
    gtk_box_append (GTK_BOX (box), label);
}

static void on_task_button_enter (GtkEventControllerMotion* controller, double x, double y, gpointer userData)
{
    GtkWidget* button = GTK_WIDGET (gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)));
    GracefulPanelTaskArea* self = GRACEFUL_PANEL_TASK_AREA (userData);
    GracefulPanelWindowInfo* info = g_object_get_data (G_OBJECT (button), "window-info");
    GtkWidget* popover = g_object_get_data (G_OBJECT (button), "preview-popover");
    GtkWidget* box = NULL;
    g_autoptr(GdkTexture) preview = NULL;

    if (info == NULL || popover == NULL) {
        return;
    }

    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class (box, "panel-task-preview");
    preview = graceful_panel_window_list_capture_preview (
        self->windowList,
        info->windowId,
        TASK_PREVIEW_MAX_WIDTH,
        TASK_PREVIEW_MAX_HEIGHT
    );

    if (preview != NULL) {
        GtkWidget* picture = gtk_picture_new_for_paintable (GDK_PAINTABLE (preview));
        GtkWidget* label = gtk_label_new (graceful_panel_window_info_get_display_title (info));

        gtk_picture_set_content_fit (GTK_PICTURE (picture), GTK_CONTENT_FIT_CONTAIN);
        gtk_widget_set_size_request (picture, TASK_PREVIEW_MAX_WIDTH, 120);
        gtk_widget_add_css_class (picture, "panel-task-preview-picture");
        gtk_widget_add_css_class (label, "panel-task-preview-title");
        gtk_box_append (GTK_BOX (box), picture);
        gtk_box_append (GTK_BOX (box), label);
    }
    else {
        set_preview_fallback (box, info);
    }

    gtk_popover_set_child (GTK_POPOVER (popover), box);
    gtk_popover_popup (GTK_POPOVER (popover));
}

static void on_task_button_leave (GtkEventControllerMotion* controller, gpointer userData)
{
    GtkWidget* button = GTK_WIDGET (gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)));
    GtkWidget* popover = g_object_get_data (G_OBJECT (button), "preview-popover");

    if (popover != NULL) {
        gtk_popover_popdown (GTK_POPOVER (popover));
    }
}

static GtkWidget* create_task_button (GracefulPanelTaskArea* self, GracefulPanelWindowInfo* info)
{
    GtkWidget* button = gtk_button_new ();
    GtkWidget* icon = create_task_icon (info);
    GtkWidget* popover = gtk_popover_new ();
    GtkEventController* motion = gtk_event_controller_motion_new ();

    gtk_widget_add_css_class (button, "panel-task-button");
    gtk_button_set_child (GTK_BUTTON (button), icon);

    gtk_popover_set_position (GTK_POPOVER (popover), GTK_POS_TOP);
    gtk_popover_set_has_arrow (GTK_POPOVER (popover), FALSE);
    gtk_popover_set_autohide (GTK_POPOVER (popover), FALSE);
    g_object_ref_sink (popover);
    gtk_widget_set_parent (popover, button);

    g_object_set_data_full (G_OBJECT (button), "window-info", info, task_button_data_free);
    g_object_set_data_full (G_OBJECT (button), "preview-popover", popover, preview_popover_data_free);
    g_signal_connect (motion, "enter", G_CALLBACK (on_task_button_enter), self);
    g_signal_connect (motion, "leave", G_CALLBACK (on_task_button_leave), self);
    gtk_widget_add_controller (button, motion);

    return button;
}

static GPtrArray* create_task_id_snapshot (GPtrArray* tasks)
{
    GPtrArray* snapshot = g_ptr_array_new_with_free_func ((GDestroyNotify) graceful_panel_window_info_free);
    guint i = 0;

    for (i = 0; i < tasks->len; ++i) {
        GracefulPanelWindowInfo* info = g_ptr_array_index (tasks, i);

        g_ptr_array_add (snapshot, graceful_panel_window_info_new (info->windowId));
    }

    return snapshot;
}

static void refresh_task_buttons (GracefulPanelTaskArea* self)
{
    GPtrArray* tasks = NULL;
    GPtrArray* snapshot = NULL;
    guint i = 0;

    tasks = graceful_panel_window_list_collect (self->windowList);
    if (graceful_panel_window_info_list_has_same_window_ids (self->currentTasks, tasks)) {
        g_ptr_array_unref (tasks);
        return;
    }

    clear_task_buttons (self);
    snapshot = create_task_id_snapshot (tasks);
    g_clear_pointer (&self->currentTasks, g_ptr_array_unref);
    self->currentTasks = snapshot;

    for (i = 0; i < tasks->len; ++i) {
        GracefulPanelWindowInfo* info = g_ptr_array_index (tasks, i);
        GtkWidget* button = create_task_button (self, info);

        g_ptr_array_index (tasks, i) = NULL;
        gtk_box_append (GTK_BOX (self), button);
    }
    g_ptr_array_set_size (tasks, 0);
    g_ptr_array_unref (tasks);
}

static gboolean on_refresh_timer (gpointer userData)
{
    GracefulPanelTaskArea* self = GRACEFUL_PANEL_TASK_AREA (userData);

    refresh_task_buttons (self);

    return G_SOURCE_CONTINUE;
}

static void graceful_panel_task_area_dispose (GObject* object)
{
    GracefulPanelTaskArea* self = GRACEFUL_PANEL_TASK_AREA (object);

    if (self->refreshTimerId != 0) {
        g_source_remove (self->refreshTimerId);
        self->refreshTimerId = 0;
    }

    clear_task_buttons (self);
    g_clear_pointer (&self->currentTasks, g_ptr_array_unref);
    g_clear_object (&self->windowList);

    G_OBJECT_CLASS (graceful_panel_task_area_parent_class)->dispose (object);
}

static void graceful_panel_task_area_class_init (GracefulPanelTaskAreaClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_task_area_dispose;
}

static void graceful_panel_task_area_init (GracefulPanelTaskArea* self)
{
    self->windowList = graceful_panel_window_list_new ();

    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-task-area");
    gtk_widget_set_hexpand (GTK_WIDGET (self), TRUE);
    gtk_widget_set_halign (GTK_WIDGET (self), GTK_ALIGN_CENTER);
    gtk_box_set_spacing (GTK_BOX (self), 6);

    refresh_task_buttons (self);
    self->refreshTimerId = g_timeout_add (TASK_REFRESH_INTERVAL_MS, on_refresh_timer, self);
}

GtkWidget* graceful_panel_task_area_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_TASK_AREA, "orientation", GTK_ORIENTATION_HORIZONTAL, NULL);
}
