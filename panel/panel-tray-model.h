/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#ifndef GRACEFUL_PANEL_PANEL_TRAY_MODEL_H
#define GRACEFUL_PANEL_PANEL_TRAY_MODEL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _GracefulPanelTrayItem GracefulPanelTrayItem;

struct _GracefulPanelTrayItem
{
    char* id;
    char* busName;
    char* objectPath;
    char* title;
    char* iconName;
    char* menuPath;
    guint64 xembedWindow;
};

GracefulPanelTrayItem* graceful_panel_tray_item_new (
    const char* id,
    const char* busName,
    const char* objectPath,
    const char* title,
    const char* iconName,
    const char* menuPath
);
GracefulPanelTrayItem* graceful_panel_tray_item_copy (const GracefulPanelTrayItem* item);
void graceful_panel_tray_item_free (GracefulPanelTrayItem* item);
GPtrArray* graceful_panel_tray_model_list_items (void);
GPtrArray* graceful_panel_tray_model_list_status_notifier_items (void);
gboolean graceful_panel_tray_model_has_items (void);
void graceful_panel_tray_model_upsert_item (
    const char* id,
    const char* busName,
    const char* objectPath,
    const char* title,
    const char* iconName,
    const char* menuPath
);
void graceful_panel_tray_model_upsert_xembed_item (
    guint64 window,
    const char* title
);
void graceful_panel_tray_model_remove_item (const char* id);
void graceful_panel_tray_model_remove_xembed_item (guint64 window);
void graceful_panel_tray_model_clear (void);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GracefulPanelTrayItem, graceful_panel_tray_item_free)

G_END_DECLS

#endif
