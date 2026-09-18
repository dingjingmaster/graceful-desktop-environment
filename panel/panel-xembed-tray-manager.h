#ifndef GRACEFUL_PANEL_PANEL_XEMBED_TRAY_MANAGER_H
#define GRACEFUL_PANEL_PANEL_XEMBED_TRAY_MANAGER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_PANEL_XEMBED_TRAY_MANAGER (graceful_panel_xembed_tray_manager_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulPanelXEmbedTrayManager,
    graceful_panel_xembed_tray_manager,
    GRACEFUL,
    PANEL_XEMBED_TRAY_MANAGER,
    GObject
)

GracefulPanelXEmbedTrayManager* graceful_panel_xembed_tray_manager_new (void);
void graceful_panel_xembed_tray_manager_start (GracefulPanelXEmbedTrayManager* self);
void graceful_panel_xembed_tray_manager_show_item_for_widget (guint64 window, GtkWidget* widget);
void graceful_panel_xembed_tray_manager_hide_item (guint64 window);
void graceful_panel_xembed_tray_manager_hide_all (void);

G_END_DECLS

#endif
