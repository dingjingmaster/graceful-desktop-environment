#ifndef GRACEFUL_PANEL_STATUS_NOTIFIER_WATCHER_H
#define GRACEFUL_PANEL_STATUS_NOTIFIER_WATCHER_H

#include "panel-tray-model.h"

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_PANEL_STATUS_NOTIFIER_WATCHER (graceful_panel_status_notifier_watcher_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulPanelStatusNotifierWatcher,
    graceful_panel_status_notifier_watcher,
    GRACEFUL,
    PANEL_STATUS_NOTIFIER_WATCHER,
    GObject
)

typedef struct _GracefulPanelStatusNotifierAddress GracefulPanelStatusNotifierAddress;
typedef struct _GracefulPanelTrayMenuItem GracefulPanelTrayMenuItem;

struct _GracefulPanelStatusNotifierAddress
{
    char* id;
    char* busName;
    char* objectPath;
};

struct _GracefulPanelTrayMenuItem
{
    int id;
    char* label;
    gboolean enabled;
    gboolean visible;
};

GracefulPanelStatusNotifierWatcher* graceful_panel_status_notifier_watcher_new (void);
void graceful_panel_status_notifier_watcher_start (GracefulPanelStatusNotifierWatcher* self);
void graceful_panel_status_notifier_item_activate (const GracefulPanelTrayItem* item, int x, int y);
void graceful_panel_status_notifier_item_context_menu (const GracefulPanelTrayItem* item, int x, int y);
GPtrArray* graceful_panel_status_notifier_item_load_menu (const GracefulPanelTrayItem* item);
GPtrArray* graceful_panel_status_notifier_menu_items_from_layout (GVariant* layout);
void graceful_panel_status_notifier_menu_item_click (const GracefulPanelTrayItem* item, int menuItemId);
GracefulPanelStatusNotifierAddress* graceful_panel_status_notifier_address_new (
    const char* service,
    const char* sender
);
void graceful_panel_status_notifier_address_free (GracefulPanelStatusNotifierAddress* address);
void graceful_panel_tray_menu_item_free (GracefulPanelTrayMenuItem* item);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GracefulPanelStatusNotifierAddress, graceful_panel_status_notifier_address_free)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GracefulPanelTrayMenuItem, graceful_panel_tray_menu_item_free)

G_END_DECLS

#endif
