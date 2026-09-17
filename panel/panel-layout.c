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
#include "panel-layout.h"

#include "panel-launcher-box.h"
#include "panel-menu-button.h"
#include "panel-status-area.h"
#include "panel-task-area.h"

struct _GracefulPanelLayout
{
    GtkBox parentInstance;
};

G_DEFINE_TYPE (GracefulPanelLayout, graceful_panel_layout, GTK_TYPE_BOX)

static void graceful_panel_layout_class_init (GracefulPanelLayoutClass* klass)
{
}

static void graceful_panel_layout_init (GracefulPanelLayout* self)
{
    GtkWidget* startBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget* taskArea = graceful_panel_task_area_new ();
    GtkWidget* statusArea = graceful_panel_status_area_new ();

    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-layout");
    gtk_widget_add_css_class (startBox, "panel-start-area");
    gtk_widget_set_hexpand (taskArea, TRUE);

    gtk_box_append (GTK_BOX (startBox), graceful_panel_menu_button_new ());
    gtk_box_append (GTK_BOX (startBox), graceful_panel_launcher_box_new ());

    gtk_box_append (GTK_BOX (self), startBox);
    gtk_box_append (GTK_BOX (self), taskArea);
    gtk_box_append (GTK_BOX (self), statusArea);
}

GtkWidget* graceful_panel_layout_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_LAYOUT, "orientation", GTK_ORIENTATION_HORIZONTAL, NULL);
}
