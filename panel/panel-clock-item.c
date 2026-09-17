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
#include "panel-clock-item.h"

#include "panel-clock-model.h"

struct _GracefulPanelClockItem
{
    GtkBox parentInstance;

    GracefulPanelClockModel* model;
    GtkWidget* label;
    guint timerId;
};

G_DEFINE_TYPE (GracefulPanelClockItem, graceful_panel_clock_item, GTK_TYPE_BOX)

static void graceful_panel_clock_item_update (GracefulPanelClockItem* self)
{
    g_autoptr(GDateTime) now = g_date_time_new_now_local ();
    g_autofree char* text = graceful_panel_clock_model_format_time (self->model, now);

    gtk_label_set_text (GTK_LABEL (self->label), text);
}

static gboolean on_clock_timer (gpointer userData)
{
    graceful_panel_clock_item_update (GRACEFUL_PANEL_CLOCK_ITEM (userData));

    return G_SOURCE_CONTINUE;
}

static void graceful_panel_clock_item_dispose (GObject* object)
{
    GracefulPanelClockItem* self = GRACEFUL_PANEL_CLOCK_ITEM (object);

    if (self->timerId != 0) {
        g_source_remove (self->timerId);
        self->timerId = 0;
    }

    g_clear_object (&self->model);

    G_OBJECT_CLASS (graceful_panel_clock_item_parent_class)->dispose (object);
}

static void graceful_panel_clock_item_class_init (GracefulPanelClockItemClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_clock_item_dispose;
}

static void graceful_panel_clock_item_init (GracefulPanelClockItem* self)
{
    self->model = graceful_panel_clock_model_new ();
    self->label = gtk_label_new (NULL);
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-clock-item");
    gtk_widget_add_css_class (self->label, "panel-item-label");
    gtk_box_append (GTK_BOX (self), self->label);

    graceful_panel_clock_item_update (self);
    self->timerId = g_timeout_add_seconds (30, on_clock_timer, self);
}

GtkWidget* graceful_panel_clock_item_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_CLOCK_ITEM, "orientation", GTK_ORIENTATION_HORIZONTAL, NULL);
}
