/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#include "panel-memory-item.h"

#include "panel-system-monitor-model.h"

struct _GracefulPanelMemoryItem
{
    GtkBox parentInstance;

    GtkWidget* label;
    guint timerId;
};

G_DEFINE_TYPE (GracefulPanelMemoryItem, graceful_panel_memory_item, GTK_TYPE_BOX)

static void update_memory_item (GracefulPanelMemoryItem* self)
{
    g_autofree char* text = NULL;
    g_autofree char* percent = NULL;
    g_autofree char* label = NULL;

    if (!g_file_get_contents ("/proc/meminfo", &text, NULL, NULL)) {
        gtk_label_set_text (GTK_LABEL (self->label), "MEM --");
        return;
    }

    percent = graceful_panel_monitor_format_percent (
        graceful_panel_monitor_parse_meminfo_usage_percent (text)
    );
    label = g_strdup_printf ("MEM %s", percent);
    gtk_label_set_text (GTK_LABEL (self->label), label);
}

static gboolean on_memory_timer (gpointer userData)
{
    update_memory_item (GRACEFUL_PANEL_MEMORY_ITEM (userData));

    return G_SOURCE_CONTINUE;
}

static void graceful_panel_memory_item_dispose (GObject* object)
{
    GracefulPanelMemoryItem* self = GRACEFUL_PANEL_MEMORY_ITEM (object);

    if (self->timerId != 0) {
        g_source_remove (self->timerId);
        self->timerId = 0;
    }

    G_OBJECT_CLASS (graceful_panel_memory_item_parent_class)->dispose (object);
}

static void graceful_panel_memory_item_class_init (GracefulPanelMemoryItemClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_memory_item_dispose;
}

static void graceful_panel_memory_item_init (GracefulPanelMemoryItem* self)
{
    self->label = gtk_label_new ("MEM 0.00%");
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-monitor-item");
    gtk_widget_add_css_class (self->label, "panel-monitor-primary-label");
    gtk_box_append (GTK_BOX (self), self->label);
    gtk_widget_set_valign (GTK_WIDGET (self), GTK_ALIGN_CENTER);

    update_memory_item (self);
    self->timerId = g_timeout_add_seconds (1, on_memory_timer, self);
}

GtkWidget* graceful_panel_memory_item_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_MEMORY_ITEM, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
}
