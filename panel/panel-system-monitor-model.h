/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#ifndef GRACEFUL_PANEL_PANEL_SYSTEM_MONITOR_MODEL_H
#define GRACEFUL_PANEL_PANEL_SYSTEM_MONITOR_MODEL_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct
{
    guint64 rxBytes;
    guint64 txBytes;
} GracefulPanelNetworkSample;

typedef struct
{
    guint64 user;
    guint64 nice;
    guint64 system;
    guint64 idle;
    guint64 iowait;
    guint64 irq;
    guint64 softirq;
    guint64 steal;
} GracefulPanelCpuSample;

char* graceful_panel_monitor_format_bytes_per_second (double bytesPerSecond);
char* graceful_panel_monitor_format_percent (double percent);
gboolean graceful_panel_monitor_parse_net_dev (const char* text, GracefulPanelNetworkSample* sample);
gboolean graceful_panel_monitor_parse_proc_stat (const char* text, GracefulPanelCpuSample* sample);
double graceful_panel_monitor_calculate_cpu_usage (
    const GracefulPanelCpuSample* previous,
    const GracefulPanelCpuSample* current
);
double graceful_panel_monitor_parse_meminfo_usage_percent (const char* text);

G_END_DECLS

#endif
