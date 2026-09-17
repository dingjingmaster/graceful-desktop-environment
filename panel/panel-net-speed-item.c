/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#include "panel-net-speed-item.h"

#include "panel-system-monitor-model.h"

struct _GracefulPanelNetSpeedItem
{
    GtkBox parentInstance;

    GtkWidget* uploadLabel;
    GtkWidget* downloadLabel;
    GracefulPanelNetworkSample previous;
    gboolean hasPrevious;
    gint64 previousTimeUs;
    guint timerId;
};

G_DEFINE_TYPE (GracefulPanelNetSpeedItem, graceful_panel_net_speed_item, GTK_TYPE_BOX)

static gboolean read_net_sample (GracefulPanelNetworkSample* sample)
{
    g_autofree char* text = NULL;

    if (!g_file_get_contents ("/proc/net/dev", &text, NULL, NULL)) {
        return FALSE;
    }

    return graceful_panel_monitor_parse_net_dev (text, sample);
}

static void update_net_speed (GracefulPanelNetSpeedItem* self)
{
    GracefulPanelNetworkSample current = { 0 };
    gint64 nowUs = g_get_monotonic_time ();
    double elapsedSeconds = 1.0;
    double upload = 0.0;
    double download = 0.0;
    g_autofree char* uploadText = NULL;
    g_autofree char* downloadText = NULL;
    g_autofree char* uploadLabel = NULL;
    g_autofree char* downloadLabel = NULL;

    if (!read_net_sample (&current)) {
        gtk_label_set_text (GTK_LABEL (self->uploadLabel), "\342\206\221 --");
        gtk_label_set_text (GTK_LABEL (self->downloadLabel), "\342\206\223 --");
        return;
    }

    if (self->hasPrevious && nowUs > self->previousTimeUs) {
        elapsedSeconds = (double) (nowUs - self->previousTimeUs) / 1000000.0;
        if (current.txBytes >= self->previous.txBytes) {
            upload = (double) (current.txBytes - self->previous.txBytes) / elapsedSeconds;
        }
        if (current.rxBytes >= self->previous.rxBytes) {
            download = (double) (current.rxBytes - self->previous.rxBytes) / elapsedSeconds;
        }
    }

    uploadText = graceful_panel_monitor_format_bytes_per_second (upload);
    downloadText = graceful_panel_monitor_format_bytes_per_second (download);
    uploadLabel = g_strdup_printf ("\342\206\221 %s", uploadText);
    downloadLabel = g_strdup_printf ("\342\206\223 %s", downloadText);
    gtk_label_set_text (GTK_LABEL (self->uploadLabel), uploadLabel);
    gtk_label_set_text (GTK_LABEL (self->downloadLabel), downloadLabel);

    self->previous = current;
    self->previousTimeUs = nowUs;
    self->hasPrevious = TRUE;
}

static gboolean on_net_speed_timer (gpointer userData)
{
    update_net_speed (GRACEFUL_PANEL_NET_SPEED_ITEM (userData));

    return G_SOURCE_CONTINUE;
}

static void graceful_panel_net_speed_item_dispose (GObject* object)
{
    GracefulPanelNetSpeedItem* self = GRACEFUL_PANEL_NET_SPEED_ITEM (object);

    if (self->timerId != 0) {
        g_source_remove (self->timerId);
        self->timerId = 0;
    }

    G_OBJECT_CLASS (graceful_panel_net_speed_item_parent_class)->dispose (object);
}

static void graceful_panel_net_speed_item_class_init (GracefulPanelNetSpeedItemClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_net_speed_item_dispose;
}

static void graceful_panel_net_speed_item_init (GracefulPanelNetSpeedItem* self)
{
    self->uploadLabel = gtk_label_new ("\342\206\221 0.00 B/s");
    self->downloadLabel = gtk_label_new ("\342\206\223 0.00 B/s");
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-monitor-item");
    gtk_widget_add_css_class (self->uploadLabel, "panel-monitor-primary-label");
    gtk_widget_add_css_class (self->downloadLabel, "panel-monitor-primary-label");
    gtk_box_append (GTK_BOX (self), self->uploadLabel);
    gtk_box_append (GTK_BOX (self), self->downloadLabel);
    gtk_widget_set_valign (GTK_WIDGET (self), GTK_ALIGN_CENTER);

    update_net_speed (self);
    self->timerId = g_timeout_add_seconds (1, on_net_speed_timer, self);
}

GtkWidget* graceful_panel_net_speed_item_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_NET_SPEED_ITEM, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
}
