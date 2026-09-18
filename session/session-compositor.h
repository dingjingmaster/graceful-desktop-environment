#ifndef SESSION_SESSION_COMPOSITOR_H
#define SESSION_SESSION_COMPOSITOR_H

#include <gio/gio.h>

G_BEGIN_DECLS

gboolean graceful_session_compositor_argv_matches (const char* const* argv, const char* sessionCommand);
gboolean graceful_session_compositor_terminate_parent (const char* sessionCommand, GError** error);

G_END_DECLS

#endif
