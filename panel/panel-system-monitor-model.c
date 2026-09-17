/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#include "panel-system-monitor-model.h"

#include <stdio.h>

static guint64 cpu_sample_total (const GracefulPanelCpuSample* sample)
{
    return sample->user + sample->nice + sample->system + sample->idle + sample->iowait +
        sample->irq + sample->softirq + sample->steal;
}

static guint64 cpu_sample_idle (const GracefulPanelCpuSample* sample)
{
    return sample->idle + sample->iowait;
}

char* graceful_panel_monitor_format_bytes_per_second (double bytesPerSecond)
{
    static const char* units[] = { "B/s", "KB/s", "MB/s", "GB/s" };
    guint unitIndex = 0;
    double value = bytesPerSecond;

    while (value >= 1024.0 && unitIndex + 1 < G_N_ELEMENTS (units)) {
        value /= 1024.0;
        unitIndex++;
    }

    return g_strdup_printf ("%.2f %s", value, units[unitIndex]);
}

char* graceful_panel_monitor_format_percent (double percent)
{
    return g_strdup_printf ("%.2f%%", CLAMP (percent, 0.0, 100.0));
}

char* graceful_panel_monitor_format_memory_label (double usagePercent)
{
    g_autofree char* percent = graceful_panel_monitor_format_percent (usagePercent);

    return g_strdup_printf ("MEM %s", percent);
}

char* graceful_panel_monitor_format_cpu_label (double usagePercent, gboolean hasTemperature, double temperatureCelsius)
{
    g_autofree char* percent = graceful_panel_monitor_format_percent (usagePercent);

    if (hasTemperature) {
        return g_strdup_printf ("CPU %s %.2f \302\260C", percent, temperatureCelsius);
    }

    return g_strdup_printf ("CPU %s", percent);
}

gboolean graceful_panel_monitor_parse_net_dev (const char* text, GracefulPanelNetworkSample* sample)
{
    g_auto(GStrv) lines = NULL;
    guint i = 0;

    if (text == NULL || sample == NULL) {
        return FALSE;
    }

    sample->rxBytes = 0;
    sample->txBytes = 0;
    lines = g_strsplit (text, "\n", -1);
    for (i = 0; lines[i] != NULL; ++i) {
        char* colon = strchr (lines[i], ':');
        g_auto(GStrv) fields = NULL;
        g_autofree char* iface = NULL;

        if (colon == NULL) {
            continue;
        }

        iface = g_strstrip (g_strndup (lines[i], (gsize) (colon - lines[i])));
        if (g_strcmp0 (iface, "lo") == 0) {
            continue;
        }

        fields = g_strsplit_set (colon + 1, " \t", -1);
        guint compactIndex = 0;
        guint fieldIndex = 0;
        guint64 rx = 0;
        guint64 tx = 0;

        for (fieldIndex = 0; fields[fieldIndex] != NULL; ++fieldIndex) {
            if (fields[fieldIndex][0] == '\0') {
                continue;
            }
            if (compactIndex == 0) {
                rx = g_ascii_strtoull (fields[fieldIndex], NULL, 10);
            }
            else if (compactIndex == 8) {
                tx = g_ascii_strtoull (fields[fieldIndex], NULL, 10);
                break;
            }
            compactIndex++;
        }

        sample->rxBytes += rx;
        sample->txBytes += tx;
    }

    return TRUE;
}

gboolean graceful_panel_monitor_parse_proc_stat (const char* text, GracefulPanelCpuSample* sample)
{
    if (text == NULL || sample == NULL) {
        return FALSE;
    }

    return sscanf (
        text,
        "cpu %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT
        " %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT,
        &sample->user,
        &sample->nice,
        &sample->system,
        &sample->idle,
        &sample->iowait,
        &sample->irq,
        &sample->softirq,
        &sample->steal
    ) >= 4;
}

double graceful_panel_monitor_calculate_cpu_usage (
    const GracefulPanelCpuSample* previous,
    const GracefulPanelCpuSample* current
)
{
    guint64 previousTotal = 0;
    guint64 currentTotal = 0;
    guint64 totalDelta = 0;
    guint64 idleDelta = 0;

    if (previous == NULL || current == NULL) {
        return 0.0;
    }

    previousTotal = cpu_sample_total (previous);
    currentTotal = cpu_sample_total (current);
    if (currentTotal <= previousTotal) {
        return 0.0;
    }

    totalDelta = currentTotal - previousTotal;
    idleDelta = cpu_sample_idle (current) - cpu_sample_idle (previous);

    return ((double) (totalDelta - idleDelta) * 100.0) / (double) totalDelta;
}

double graceful_panel_monitor_parse_meminfo_usage_percent (const char* text)
{
    g_auto(GStrv) lines = NULL;
    guint i = 0;
    guint64 memTotal = 0;
    guint64 memAvailable = 0;

    if (text == NULL) {
        return 0.0;
    }

    lines = g_strsplit (text, "\n", -1);
    for (i = 0; lines[i] != NULL; ++i) {
        if (g_str_has_prefix (lines[i], "MemTotal:")) {
            sscanf (lines[i], "MemTotal: %" G_GUINT64_FORMAT " kB", &memTotal);
        }
        else if (g_str_has_prefix (lines[i], "MemAvailable:")) {
            sscanf (lines[i], "MemAvailable: %" G_GUINT64_FORMAT " kB", &memAvailable);
        }
    }

    if (memTotal == 0 || memAvailable > memTotal) {
        return 0.0;
    }

    return ((double) (memTotal - memAvailable) * 100.0) / (double) memTotal;
}
