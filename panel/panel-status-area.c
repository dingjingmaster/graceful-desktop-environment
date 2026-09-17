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
#include "panel-status-area.h"

#include "panel-clock-item.h"
#include "panel-cpu-item.h"
#include "panel-memory-item.h"
#include "panel-net-speed-item.h"
#include "panel-power-item.h"
#include "panel-tray-item.h"
#include "panel-workspace-item.h"

struct _GracefulPanelStatusArea
{
    GtkBox parentInstance;
};

G_DEFINE_TYPE (GracefulPanelStatusArea, graceful_panel_status_area, GTK_TYPE_BOX)

static void graceful_panel_status_area_class_init (GracefulPanelStatusAreaClass* klass)
{
}

static void graceful_panel_status_area_init (GracefulPanelStatusArea* self)
{
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-status-area");
    gtk_box_set_spacing (GTK_BOX (self), 4);
    gtk_box_append (GTK_BOX (self), graceful_panel_workspace_item_new ());
    gtk_box_append (GTK_BOX (self), graceful_panel_tray_item_widget_new ());
    gtk_box_append (GTK_BOX (self), graceful_panel_net_speed_item_new ());
    gtk_box_append (GTK_BOX (self), graceful_panel_cpu_item_new ());
    gtk_box_append (GTK_BOX (self), graceful_panel_memory_item_new ());
    gtk_box_append (GTK_BOX (self), graceful_panel_power_item_new ());
    gtk_box_append (GTK_BOX (self), graceful_panel_clock_item_new ());
}

GtkWidget* graceful_panel_status_area_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_STATUS_AREA, "orientation", GTK_ORIENTATION_HORIZONTAL, NULL);
}
