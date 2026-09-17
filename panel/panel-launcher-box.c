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
#include "panel-launcher-box.h"

struct _GracefulPanelLauncherBox
{
    GtkBox parentInstance;
};

G_DEFINE_TYPE (GracefulPanelLauncherBox, graceful_panel_launcher_box, GTK_TYPE_BOX)

static GtkWidget* create_launcher_button (const char* iconName, const char* tooltip)
{
    GtkWidget* button = gtk_button_new_from_icon_name (iconName);

    gtk_widget_add_css_class (button, "panel-icon-button");
    gtk_widget_set_tooltip_text (button, tooltip);

    return button;
}

static void graceful_panel_launcher_box_class_init (GracefulPanelLauncherBoxClass* klass)
{
}

static void graceful_panel_launcher_box_init (GracefulPanelLauncherBox* self)
{
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-launcher-box");
    gtk_box_set_spacing (GTK_BOX (self), 4);
    gtk_box_append (GTK_BOX (self), create_launcher_button ("utilities-terminal-symbolic", "Terminal"));
    gtk_box_append (GTK_BOX (self), create_launcher_button ("folder-symbolic", "Files"));
    gtk_box_append (GTK_BOX (self), create_launcher_button ("emblem-system-symbolic", "Settings"));
}

GtkWidget* graceful_panel_launcher_box_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_LAUNCHER_BOX, "orientation", GTK_ORIENTATION_HORIZONTAL, NULL);
}
