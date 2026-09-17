/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#ifndef GRACEFUL_PANEL_PANEL_WORKSPACE_MODEL_H
#define GRACEFUL_PANEL_PANEL_WORKSPACE_MODEL_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_PANEL_WORKSPACE_MODEL (graceful_panel_workspace_model_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulPanelWorkspaceModel,
    graceful_panel_workspace_model,
    GRACEFUL,
    PANEL_WORKSPACE_MODEL,
    GObject
)

GracefulPanelWorkspaceModel* graceful_panel_workspace_model_new (void);
char* graceful_panel_workspace_model_format_label (guint currentWorkspace, guint workspaceCount);
char* graceful_panel_workspace_model_get_label (GracefulPanelWorkspaceModel* self);

G_END_DECLS

#endif
