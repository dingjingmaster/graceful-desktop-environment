/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#include "panel-tray-model.h"

GracefulPanelTrayItem* graceful_panel_tray_item_new (const char* id, const char* title, const char* iconName)
{
    GracefulPanelTrayItem* item = g_new0 (GracefulPanelTrayItem, 1);

    item->id = g_strdup (id);
    item->title = g_strdup (title != NULL && title[0] != '\0' ? title : "Tray Item");
    item->iconName = g_strdup (iconName != NULL && iconName[0] != '\0' ? iconName : "application-x-executable-symbolic");

    return item;
}

GracefulPanelTrayItem* graceful_panel_tray_item_copy (const GracefulPanelTrayItem* item)
{
    if (item == NULL) {
        return NULL;
    }

    return graceful_panel_tray_item_new (item->id, item->title, item->iconName);
}

void graceful_panel_tray_item_free (GracefulPanelTrayItem* item)
{
    if (item == NULL) {
        return;
    }

    g_clear_pointer (&item->id, g_free);
    g_clear_pointer (&item->title, g_free);
    g_clear_pointer (&item->iconName, g_free);
    g_free (item);
}

GPtrArray* graceful_panel_tray_model_list_items (void)
{
    return g_ptr_array_new_with_free_func ((GDestroyNotify) graceful_panel_tray_item_free);
}

gboolean graceful_panel_tray_model_has_items (void)
{
    g_autoptr(GPtrArray) items = graceful_panel_tray_model_list_items ();

    return items->len > 0;
}
