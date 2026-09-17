/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */

#ifndef SESSION_SESSION_COMPONENT_H
#define SESSION_SESSION_COMPONENT_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_SESSION_COMPONENT (graceful_session_component_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulSessionComponent,
    graceful_session_component,
    GRACEFUL,
    SESSION_COMPONENT,
    GObject
)

GracefulSessionComponent* graceful_session_component_new (
    const char* name,
    const char* const* argv,
    gboolean required,
    gboolean oneshot
);
const char* graceful_session_component_get_name (const GracefulSessionComponent* self);
const char* const* graceful_session_component_get_argv (const GracefulSessionComponent* self);
gboolean graceful_session_component_get_required (const GracefulSessionComponent* self);
gboolean graceful_session_component_get_oneshot (const GracefulSessionComponent* self);
GPtrArray* graceful_session_component_list_new_default (void);

G_END_DECLS

#endif
