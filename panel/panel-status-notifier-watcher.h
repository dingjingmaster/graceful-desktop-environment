#ifndef GRACEFUL_PANEL_STATUS_NOTIFIER_WATCHER_H
#define GRACEFUL_PANEL_STATUS_NOTIFIER_WATCHER_H

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

struct _GracefulPanelStatusNotifierAddress
{
    char* id;
    char* busName;
    char* objectPath;
};

GracefulPanelStatusNotifierWatcher* graceful_panel_status_notifier_watcher_new (void);
void graceful_panel_status_notifier_watcher_start (GracefulPanelStatusNotifierWatcher* self);
GracefulPanelStatusNotifierAddress* graceful_panel_status_notifier_address_new (
    const char* service,
    const char* sender
);
void graceful_panel_status_notifier_address_free (GracefulPanelStatusNotifierAddress* address);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GracefulPanelStatusNotifierAddress, graceful_panel_status_notifier_address_free)

G_END_DECLS

#endif
