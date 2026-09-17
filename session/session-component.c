/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */

#include "session-component.h"

#ifndef GRACEFUL_SESSION_BINDIR
#define GRACEFUL_SESSION_BINDIR "/usr/bin"
#endif

struct _GracefulSessionComponent
{
    GObject parentInstance;
    char* name;
    GStrv argv;
    gboolean required;
    gboolean oneshot;
};

G_DEFINE_TYPE (GracefulSessionComponent, graceful_session_component, G_TYPE_OBJECT)

static void graceful_session_component_finalize (GObject* object)
{
    GracefulSessionComponent* self = GRACEFUL_SESSION_COMPONENT (object);

    g_clear_pointer (&self->name, g_free);
    g_clear_pointer (&self->argv, g_strfreev);

    G_OBJECT_CLASS (graceful_session_component_parent_class)->finalize (object);
}

static void graceful_session_component_class_init (GracefulSessionComponentClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->finalize = graceful_session_component_finalize;
}

static void graceful_session_component_init (GracefulSessionComponent* self)
{
}

GracefulSessionComponent* graceful_session_component_new (
    const char* name,
    const char* const* argv,
    gboolean required,
    gboolean oneshot
)
{
    GracefulSessionComponent* self = g_object_new (GRACEFUL_TYPE_SESSION_COMPONENT, NULL);

    self->name = g_strdup (name != NULL ? name : "session-component");
    self->argv = g_strdupv ((GStrv) argv);
    self->required = required;
    self->oneshot = oneshot;

    return self;
}

const char* graceful_session_component_get_name (const GracefulSessionComponent* self)
{
    g_return_val_if_fail (GRACEFUL_IS_SESSION_COMPONENT ((GracefulSessionComponent*) self), NULL);

    return self->name;
}

const char* const* graceful_session_component_get_argv (const GracefulSessionComponent* self)
{
    g_return_val_if_fail (GRACEFUL_IS_SESSION_COMPONENT ((GracefulSessionComponent*) self), NULL);

    return (const char* const*) self->argv;
}

gboolean graceful_session_component_get_required (const GracefulSessionComponent* self)
{
    g_return_val_if_fail (GRACEFUL_IS_SESSION_COMPONENT ((GracefulSessionComponent*) self), FALSE);

    return self->required;
}

gboolean graceful_session_component_get_oneshot (const GracefulSessionComponent* self)
{
    g_return_val_if_fail (GRACEFUL_IS_SESSION_COMPONENT ((GracefulSessionComponent*) self), FALSE);

    return self->oneshot;
}

GPtrArray* graceful_session_component_list_new_default (void)
{
    const char* ibusArgv[] = { "ibus-daemon", "--daemonize", "--xim", NULL };
    const char* rimeArgv[] = { "ibus", "engine", "rime", NULL };
    const char* desktopArgv[] = { GRACEFUL_SESSION_BINDIR "/graceful-desktop", NULL };
    const char* panelArgv[] = { GRACEFUL_SESSION_BINDIR "/graceful-panel", NULL };
    GPtrArray* components = g_ptr_array_new_with_free_func (g_object_unref);

    g_ptr_array_add (components, graceful_session_component_new ("ibus-daemon", ibusArgv, FALSE, TRUE));
    g_ptr_array_add (components, graceful_session_component_new ("ibus-rime", rimeArgv, FALSE, TRUE));
    g_ptr_array_add (components, graceful_session_component_new ("graceful-desktop", desktopArgv, TRUE, FALSE));
    g_ptr_array_add (components, graceful_session_component_new ("graceful-panel", panelArgv, TRUE, FALSE));

    return components;
}
