/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#include "panel-workspace-item.h"

#include "panel-workspace-model.h"

#define WORKSPACE_REFRESH_INTERVAL_MS 500

struct _GracefulPanelWorkspaceItem
{
    GtkButton parentInstance;

    GracefulPanelWorkspaceModel* model;
    guint timerId;
};

G_DEFINE_TYPE (GracefulPanelWorkspaceItem, graceful_panel_workspace_item, GTK_TYPE_BUTTON)

static void update_workspace_label (GracefulPanelWorkspaceItem* self)
{
    g_autofree char* label = graceful_panel_workspace_model_get_label (self->model);

    gtk_button_set_label (GTK_BUTTON (self), label);
}

static gboolean on_workspace_timer (gpointer userData)
{
    update_workspace_label (GRACEFUL_PANEL_WORKSPACE_ITEM (userData));

    return G_SOURCE_CONTINUE;
}

static void graceful_panel_workspace_item_dispose (GObject* object)
{
    GracefulPanelWorkspaceItem* self = GRACEFUL_PANEL_WORKSPACE_ITEM (object);

    if (self->timerId != 0) {
        g_source_remove (self->timerId);
        self->timerId = 0;
    }
    g_clear_object (&self->model);

    G_OBJECT_CLASS (graceful_panel_workspace_item_parent_class)->dispose (object);
}

static void graceful_panel_workspace_item_class_init (GracefulPanelWorkspaceItemClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_workspace_item_dispose;
}

static void graceful_panel_workspace_item_init (GracefulPanelWorkspaceItem* self)
{
    self->model = graceful_panel_workspace_model_new ();
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-workspace-button");
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-icon-button");
    gtk_widget_set_tooltip_text (GTK_WIDGET (self), "Workspace");
    update_workspace_label (self);
    self->timerId = g_timeout_add (WORKSPACE_REFRESH_INTERVAL_MS, on_workspace_timer, self);
}

GtkWidget* graceful_panel_workspace_item_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_WORKSPACE_ITEM, NULL);
}
