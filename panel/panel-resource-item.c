/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#include "panel-resource-item.h"

#include "panel-system-monitor-model.h"

struct _GracefulPanelResourceItem
{
    GtkBox parentInstance;

    GtkWidget* memoryLabel;
    GtkWidget* cpuLabel;
    GracefulPanelCpuSample previousCpu;
    gboolean hasPreviousCpu;
    guint timerId;
};

G_DEFINE_TYPE (GracefulPanelResourceItem, graceful_panel_resource_item, GTK_TYPE_BOX)

static gboolean read_cpu_sample (GracefulPanelCpuSample* sample)
{
    g_autofree char* text = NULL;

    if (!g_file_get_contents ("/proc/stat", &text, NULL, NULL)) {
        return FALSE;
    }

    return graceful_panel_monitor_parse_proc_stat (text, sample);
}

static void update_memory_label (GracefulPanelResourceItem* self)
{
    g_autofree char* text = NULL;
    g_autofree char* label = NULL;

    if (!g_file_get_contents ("/proc/meminfo", &text, NULL, NULL)) {
        gtk_label_set_text (GTK_LABEL (self->memoryLabel), "MEM --");
        return;
    }

    label = graceful_panel_monitor_format_memory_label (
        graceful_panel_monitor_parse_meminfo_usage_percent (text)
    );
    gtk_label_set_text (GTK_LABEL (self->memoryLabel), label);
}

static void update_cpu_label (GracefulPanelResourceItem* self)
{
    GracefulPanelCpuSample current = { 0 };
    double usage = 0.0;
    g_autofree char* label = NULL;

    if (read_cpu_sample (&current)) {
        if (self->hasPreviousCpu) {
            usage = graceful_panel_monitor_calculate_cpu_usage (&self->previousCpu, &current);
        }
        self->previousCpu = current;
        self->hasPreviousCpu = TRUE;
    }

    label = graceful_panel_monitor_format_cpu_label (usage, FALSE, 0.0);
    gtk_label_set_text (GTK_LABEL (self->cpuLabel), label);
}

static void update_resource_item (GracefulPanelResourceItem* self)
{
    update_memory_label (self);
    update_cpu_label (self);
}

static gboolean on_resource_timer (gpointer userData)
{
    update_resource_item (GRACEFUL_PANEL_RESOURCE_ITEM (userData));

    return G_SOURCE_CONTINUE;
}

static void graceful_panel_resource_item_dispose (GObject* object)
{
    GracefulPanelResourceItem* self = GRACEFUL_PANEL_RESOURCE_ITEM (object);

    if (self->timerId != 0) {
        g_source_remove (self->timerId);
        self->timerId = 0;
    }

    G_OBJECT_CLASS (graceful_panel_resource_item_parent_class)->dispose (object);
}

static void graceful_panel_resource_item_class_init (GracefulPanelResourceItemClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_resource_item_dispose;
}

static void graceful_panel_resource_item_init (GracefulPanelResourceItem* self)
{
    self->memoryLabel = gtk_label_new ("MEM 0.00%");
    self->cpuLabel = gtk_label_new ("CPU 0.00%");
    gtk_label_set_width_chars (GTK_LABEL (self->memoryLabel), 11);
    gtk_label_set_width_chars (GTK_LABEL (self->cpuLabel), 11);
    gtk_label_set_xalign (GTK_LABEL (self->memoryLabel), 0.0);
    gtk_label_set_xalign (GTK_LABEL (self->cpuLabel), 0.0);
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-monitor-item");
    gtk_widget_add_css_class (self->memoryLabel, "panel-monitor-primary-label");
    gtk_widget_add_css_class (self->cpuLabel, "panel-monitor-primary-label");
    gtk_box_append (GTK_BOX (self), self->memoryLabel);
    gtk_box_append (GTK_BOX (self), self->cpuLabel);
    gtk_widget_set_valign (GTK_WIDGET (self), GTK_ALIGN_CENTER);

    update_resource_item (self);
    self->timerId = g_timeout_add_seconds (1, on_resource_timer, self);
}

GtkWidget* graceful_panel_resource_item_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_RESOURCE_ITEM, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
}
