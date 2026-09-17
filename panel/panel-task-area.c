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

struct _GracefulPanelTaskArea
{
    GtkBox parentInstance;
};

G_DEFINE_TYPE (GracefulPanelTaskArea, graceful_panel_task_area, GTK_TYPE_BOX)

static void graceful_panel_task_area_class_init (GracefulPanelTaskAreaClass* klass)
{
}

static void graceful_panel_task_area_init (GracefulPanelTaskArea* self)
{
    GtkWidget* label = gtk_label_new ("No active windows");

    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-task-area");
    gtk_widget_add_css_class (label, "panel-muted-label");
    gtk_widget_set_hexpand (GTK_WIDGET (self), TRUE);
    gtk_widget_set_halign (GTK_WIDGET (self), GTK_ALIGN_CENTER);
    gtk_box_append (GTK_BOX (self), label);
}

GtkWidget* graceful_panel_task_area_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_TASK_AREA, "orientation", GTK_ORIENTATION_HORIZONTAL, NULL);
}
