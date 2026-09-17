/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#include "panel-cpu-item.h"

#include "panel-system-monitor-model.h"

struct _GracefulPanelCpuItem
{
    GtkBox parentInstance;

    GtkWidget* label;
    GracefulPanelCpuSample previous;
    gboolean hasPrevious;
    guint timerId;
};

G_DEFINE_TYPE (GracefulPanelCpuItem, graceful_panel_cpu_item, GTK_TYPE_BOX)

static gboolean read_cpu_sample (GracefulPanelCpuSample* sample)
{
    g_autofree char* text = NULL;

    if (!g_file_get_contents ("/proc/stat", &text, NULL, NULL)) {
        return FALSE;
    }

    return graceful_panel_monitor_parse_proc_stat (text, sample);
}

static gboolean read_temperature_from_dir (const char* directory, double* celsius)
{
    g_autoptr(GDir) dir = g_dir_open (directory, 0, NULL);
    const char* name = NULL;

    if (dir == NULL) {
        return FALSE;
    }

    while ((name = g_dir_read_name (dir)) != NULL) {
        g_autofree char* path = NULL;
        g_autofree char* text = NULL;
        gint64 milliCelsius = 0;

        if (!g_str_has_prefix (name, "temp") && g_strcmp0 (name, "temp") != 0) {
            continue;
        }
        if (!g_str_has_suffix (name, "_input") && g_strcmp0 (name, "temp") != 0) {
            continue;
        }

        path = g_build_filename (directory, name, NULL);
        if (!g_file_get_contents (path, &text, NULL, NULL)) {
            continue;
        }

        milliCelsius = g_ascii_strtoll (text, NULL, 10);
        if (milliCelsius > 0) {
            *celsius = (double) milliCelsius / 1000.0;
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean read_cpu_temperature (double* celsius)
{
    guint i = 0;

    for (i = 0; i < 16; ++i) {
        g_autofree char* path = g_strdup_printf ("/sys/class/thermal/thermal_zone%u", i);
        if (read_temperature_from_dir (path, celsius)) {
            return TRUE;
        }
    }

    for (i = 0; i < 32; ++i) {
        g_autofree char* path = g_strdup_printf ("/sys/class/hwmon/hwmon%u", i);
        if (read_temperature_from_dir (path, celsius)) {
            return TRUE;
        }
    }

    return FALSE;
}

static void update_cpu_item (GracefulPanelCpuItem* self)
{
    GracefulPanelCpuSample current = { 0 };
    double usage = 0.0;
    double temperature = 0.0;
    g_autofree char* usagePercent = NULL;
    g_autofree char* labelText = NULL;

    if (read_cpu_sample (&current)) {
        if (self->hasPrevious) {
            usage = graceful_panel_monitor_calculate_cpu_usage (&self->previous, &current);
        }
        self->previous = current;
        self->hasPrevious = TRUE;
    }

    usagePercent = graceful_panel_monitor_format_percent (usage);
    if (read_cpu_temperature (&temperature)) {
        labelText = g_strdup_printf ("CPU %s %.2f \302\260C", usagePercent, temperature);
    }
    else {
        labelText = g_strdup_printf ("CPU %s", usagePercent);
    }
    gtk_label_set_text (GTK_LABEL (self->label), labelText);
}

static gboolean on_cpu_timer (gpointer userData)
{
    update_cpu_item (GRACEFUL_PANEL_CPU_ITEM (userData));

    return G_SOURCE_CONTINUE;
}

static void graceful_panel_cpu_item_dispose (GObject* object)
{
    GracefulPanelCpuItem* self = GRACEFUL_PANEL_CPU_ITEM (object);

    if (self->timerId != 0) {
        g_source_remove (self->timerId);
        self->timerId = 0;
    }

    G_OBJECT_CLASS (graceful_panel_cpu_item_parent_class)->dispose (object);
}

static void graceful_panel_cpu_item_class_init (GracefulPanelCpuItemClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_cpu_item_dispose;
}

static void graceful_panel_cpu_item_init (GracefulPanelCpuItem* self)
{
    self->label = gtk_label_new ("CPU 0.00%");
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-monitor-item");
    gtk_widget_add_css_class (self->label, "panel-monitor-primary-label");
    gtk_box_append (GTK_BOX (self), self->label);
    gtk_widget_set_valign (GTK_WIDGET (self), GTK_ALIGN_CENTER);

    update_cpu_item (self);
    self->timerId = g_timeout_add_seconds (1, on_cpu_timer, self);
}

GtkWidget* graceful_panel_cpu_item_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_CPU_ITEM, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
}
