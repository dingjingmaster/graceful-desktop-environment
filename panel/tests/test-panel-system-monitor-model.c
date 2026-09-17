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

#include <glib.h>

static void monitor_model_formats_network_speed_units (void)
{
    g_autofree char* kb = graceful_panel_monitor_format_bytes_per_second (1536.0);
    g_autofree char* mb = graceful_panel_monitor_format_bytes_per_second (2.5 * 1024.0 * 1024.0);
    g_autofree char* gb = graceful_panel_monitor_format_bytes_per_second (3.0 * 1024.0 * 1024.0 * 1024.0);

    g_assert_cmpstr (kb, ==, "1.50 KB/s");
    g_assert_cmpstr (mb, ==, "2.50 MB/s");
    g_assert_cmpstr (gb, ==, "3.00 GB/s");
}

static void monitor_model_parses_net_dev_ignoring_loopback (void)
{
    const char* text =
        "Inter-|   Receive                                                |  Transmit\n"
        " face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n"
        "    lo: 1000 0 0 0 0 0 0 0 2000 0 0 0 0 0 0 0\n"
        "  eth0: 4096 0 0 0 0 0 0 0 8192 0 0 0 0 0 0 0\n"
        " wlan0: 1024 0 0 0 0 0 0 0 2048 0 0 0 0 0 0 0\n";
    GracefulPanelNetworkSample sample = { 0 };

    g_assert_true (graceful_panel_monitor_parse_net_dev (text, &sample));
    g_assert_cmpuint (sample.rxBytes, ==, 5120);
    g_assert_cmpuint (sample.txBytes, ==, 10240);
}

static void monitor_model_calculates_cpu_usage_between_samples (void)
{
    GracefulPanelCpuSample previous = { .user = 100, .nice = 0, .system = 100, .idle = 800 };
    GracefulPanelCpuSample current = { .user = 150, .nice = 0, .system = 150, .idle = 900 };

    g_assert_cmpfloat_with_epsilon (
        graceful_panel_monitor_calculate_cpu_usage (&previous, &current),
        50.0,
        0.001
    );
}

static void monitor_model_parses_meminfo_percent (void)
{
    const char* text =
        "MemTotal:        1000000 kB\n"
        "MemFree:          100000 kB\n"
        "MemAvailable:     250000 kB\n";

    g_assert_cmpfloat_with_epsilon (
        graceful_panel_monitor_parse_meminfo_usage_percent (text),
        75.0,
        0.001
    );
}

static void monitor_model_formats_resource_labels (void)
{
    g_autofree char* memory = graceful_panel_monitor_format_memory_label (48.5);
    g_autofree char* cpu = graceful_panel_monitor_format_cpu_label (37.25, FALSE, 0.0);
    g_autofree char* cpuWithTemperature = graceful_panel_monitor_format_cpu_label (92.0, TRUE, 61.5);

    g_assert_cmpstr (memory, ==, "MEM 48.50%");
    g_assert_cmpstr (cpu, ==, "CPU 37.25%");
    g_assert_cmpstr (cpuWithTemperature, ==, "CPU 92.00% 61.50 \302\260C");
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/panel/system-monitor/formats-network-speed-units",
        monitor_model_formats_network_speed_units
    );
    g_test_add_func (
        "/panel/system-monitor/parses-net-dev-ignoring-loopback",
        monitor_model_parses_net_dev_ignoring_loopback
    );
    g_test_add_func (
        "/panel/system-monitor/calculates-cpu-usage-between-samples",
        monitor_model_calculates_cpu_usage_between_samples
    );
    g_test_add_func (
        "/panel/system-monitor/parses-meminfo-percent",
        monitor_model_parses_meminfo_percent
    );
    g_test_add_func (
        "/panel/system-monitor/formats-resource-labels",
        monitor_model_formats_resource_labels
    );

    return g_test_run ();
}
