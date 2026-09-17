#ifndef GRACEFUL_SESSION_AUTOSTART_H
#define GRACEFUL_SESSION_AUTOSTART_H

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_SESSION_AUTOSTART (graceful_session_autostart_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulSessionAutostart,
    graceful_session_autostart,
    GRACEFUL,
    SESSION_AUTOSTART,
    GObject
)

GracefulSessionAutostart* graceful_session_autostart_new (const char* currentDesktop);
GPtrArray* graceful_session_autostart_list_commands (
    GracefulSessionAutostart* self,
    const char* const* configDirs,
    const char* pathEnv
);

G_END_DECLS

#endif
