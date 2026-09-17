/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#include "panel-workspace-model.h"

#include <gdk/x11/gdkx.h>
#include <X11/Xatom.h>

struct _GracefulPanelWorkspaceModel
{
    GObject parentInstance;
};

G_DEFINE_TYPE (GracefulPanelWorkspaceModel, graceful_panel_workspace_model, G_TYPE_OBJECT)

static gboolean read_root_cardinal (const char* propertyName, guint* value)
{
    GdkDisplay* gdkDisplay = gdk_display_get_default ();
    Display* display = NULL;
    Atom property = None;
    Atom actualType = None;
    int actualFormat = 0;
    unsigned long nitems = 0;
    unsigned long bytesAfter = 0;
    unsigned char* data = NULL;
    gboolean ok = FALSE;

    if (gdkDisplay == NULL || !GDK_IS_X11_DISPLAY (gdkDisplay)) {
        return FALSE;
    }

    display = gdk_x11_display_get_xdisplay (gdkDisplay);
    property = XInternAtom (display, propertyName, False);
    if (XGetWindowProperty (
            display,
            DefaultRootWindow (display),
            property,
            0,
            1,
            False,
            XA_CARDINAL,
            &actualType,
            &actualFormat,
            &nitems,
            &bytesAfter,
            &data
        ) == Success &&
        data != NULL &&
        actualType == XA_CARDINAL &&
        actualFormat == 32 &&
        nitems == 1) {
        *value = (guint) (*(unsigned long*) data);
        ok = TRUE;
    }

    if (data != NULL) {
        XFree (data);
    }

    return ok;
}

static void graceful_panel_workspace_model_class_init (GracefulPanelWorkspaceModelClass* klass)
{
}

static void graceful_panel_workspace_model_init (GracefulPanelWorkspaceModel* self)
{
}

GracefulPanelWorkspaceModel* graceful_panel_workspace_model_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_WORKSPACE_MODEL, NULL);
}

char* graceful_panel_workspace_model_format_label (guint currentWorkspace, guint workspaceCount)
{
    if (workspaceCount == 0 || currentWorkspace >= workspaceCount) {
        currentWorkspace = 0;
        workspaceCount = 1;
    }

    return g_strdup_printf ("%u/%u", currentWorkspace + 1, workspaceCount);
}

char* graceful_panel_workspace_model_get_label (GracefulPanelWorkspaceModel* self)
{
    guint currentWorkspace = 0;
    guint workspaceCount = 1;

    g_return_val_if_fail (GRACEFUL_IS_PANEL_WORKSPACE_MODEL (self), NULL);

    read_root_cardinal ("_NET_CURRENT_DESKTOP", &currentWorkspace);
    read_root_cardinal ("_NET_NUMBER_OF_DESKTOPS", &workspaceCount);

    return graceful_panel_workspace_model_format_label (currentWorkspace, workspaceCount);
}
