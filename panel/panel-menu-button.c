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
#include "panel-menu-button.h"

#include "panel-start-menu.h"

#ifndef GRACEFUL_PANEL_LOGO_SOURCE_PATH
#define GRACEFUL_PANEL_LOGO_SOURCE_PATH "data/2.png"
#endif

#ifndef GRACEFUL_PANEL_LOGO_INSTALL_PATH
#define GRACEFUL_PANEL_LOGO_INSTALL_PATH "/usr/local/share/graceful/panel/menu-logo.png"
#endif

struct _GracefulPanelMenuButton
{
    GtkButton parentInstance;

    GtkWidget* startMenu;
};

G_DEFINE_TYPE (GracefulPanelMenuButton, graceful_panel_menu_button, GTK_TYPE_BUTTON)

static GtkWidget* create_menu_logo (void)
{
    GtkWidget* image = NULL;
    const char* logoPath = NULL;

    if (g_file_test (GRACEFUL_PANEL_LOGO_INSTALL_PATH, G_FILE_TEST_EXISTS)) {
        logoPath = GRACEFUL_PANEL_LOGO_INSTALL_PATH;
    }
    else if (g_file_test (GRACEFUL_PANEL_LOGO_SOURCE_PATH, G_FILE_TEST_EXISTS)) {
        logoPath = GRACEFUL_PANEL_LOGO_SOURCE_PATH;
    }

    if (logoPath != NULL) {
        g_autoptr(GFile) file = g_file_new_for_path (logoPath);

        image = gtk_picture_new_for_file (file);
        gtk_picture_set_content_fit (GTK_PICTURE (image), GTK_CONTENT_FIT_CONTAIN);
        gtk_widget_set_size_request (image, 24, 24);
        gtk_widget_add_css_class (image, "panel-menu-logo");
    }
    else {
        image = gtk_image_new_from_icon_name ("start-here-symbolic");
    }

    return image;
}

static void graceful_panel_menu_button_dispose (GObject* object)
{
    GracefulPanelMenuButton* self = GRACEFUL_PANEL_MENU_BUTTON (object);

    if (self->startMenu != NULL) {
        gtk_popover_popdown (GTK_POPOVER (self->startMenu));
        if (gtk_widget_get_parent (self->startMenu) != NULL) {
            gtk_widget_unparent (self->startMenu);
        }
        g_clear_object (&self->startMenu);
    }

    G_OBJECT_CLASS (graceful_panel_menu_button_parent_class)->dispose (object);
}

static void graceful_panel_menu_button_class_init (GracefulPanelMenuButtonClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_menu_button_dispose;
}

static void on_menu_button_clicked (GtkButton* button, gpointer userData)
{
    GracefulPanelMenuButton* self = GRACEFUL_PANEL_MENU_BUTTON (button);

    graceful_panel_menu_button_popup (self);
}

static void on_menu_button_pressed (
    GtkGestureClick* gesture,
    int pressCount,
    double x,
    double y,
    gpointer userData
)
{
    GracefulPanelMenuButton* self = GRACEFUL_PANEL_MENU_BUTTON (userData);

    graceful_panel_menu_button_popup (self);
}

static void graceful_panel_menu_button_init (GracefulPanelMenuButton* self)
{
    gtk_button_set_child (GTK_BUTTON (self), create_menu_logo ());
    gtk_widget_set_size_request (GTK_WIDGET (self), 36, 34);
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-menu-button");

    self->startMenu = graceful_panel_start_menu_new ();
    g_object_ref_sink (self->startMenu);
    g_signal_connect (self, "clicked", G_CALLBACK (on_menu_button_clicked), NULL);
    GtkGesture* click = gtk_gesture_click_new ();
    gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (click), GTK_PHASE_CAPTURE);
    g_signal_connect (click, "pressed", G_CALLBACK (on_menu_button_pressed), self);
    gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (click));
}

GtkWidget* graceful_panel_menu_button_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_MENU_BUTTON, NULL);
}

void graceful_panel_menu_button_popup (GracefulPanelMenuButton* self)
{
    g_return_if_fail (GRACEFUL_IS_PANEL_MENU_BUTTON (self));

    graceful_panel_start_menu_popup_for_button (GRACEFUL_PANEL_START_MENU (self->startMenu), GTK_WIDGET (self));
}
