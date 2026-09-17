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
#include "panel-volume-item.h"

struct _GracefulPanelVolumeItem
{
    GtkButton parentInstance;
};

G_DEFINE_TYPE (GracefulPanelVolumeItem, graceful_panel_volume_item, GTK_TYPE_BUTTON)

static void graceful_panel_volume_item_class_init (GracefulPanelVolumeItemClass* klass)
{
}

static void graceful_panel_volume_item_init (GracefulPanelVolumeItem* self)
{
    gtk_button_set_icon_name (GTK_BUTTON (self), "audio-volume-medium-symbolic");
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-icon-button");
    gtk_widget_set_tooltip_text (GTK_WIDGET (self), "Volume");
}

GtkWidget* graceful_panel_volume_item_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_VOLUME_ITEM, NULL);
}
