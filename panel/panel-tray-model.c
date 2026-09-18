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

static GPtrArray* gsTrayItems = NULL;

static GPtrArray* graceful_panel_tray_model_get_items (void)
{
    if (gsTrayItems == NULL) {
        gsTrayItems = g_ptr_array_new_with_free_func ((GDestroyNotify) graceful_panel_tray_item_free);
    }

    return gsTrayItems;
}

static gint graceful_panel_tray_model_find_item (const char* id)
{
    GPtrArray* items = graceful_panel_tray_model_get_items ();

    for (guint i = 0; i < items->len; i++) {
        GracefulPanelTrayItem* item = g_ptr_array_index (items, i);

        if (g_strcmp0 (item->id, id) == 0) {
            return (gint) i;
        }
    }

    return -1;
}

GracefulPanelTrayItem* graceful_panel_tray_item_new (
    const char* id,
    const char* busName,
    const char* objectPath,
    const char* title,
    const char* iconName,
    const char* menuPath
)
{
    GracefulPanelTrayItem* item = g_new0 (GracefulPanelTrayItem, 1);

    item->id = g_strdup (id);
    item->busName = g_strdup (busName != NULL ? busName : "");
    item->objectPath = g_strdup (objectPath != NULL ? objectPath : "");
    item->title = g_strdup (title != NULL && title[0] != '\0' ? title : "Tray Item");
    item->iconName = g_strdup (iconName != NULL && iconName[0] != '\0' ? iconName : "application-x-executable-symbolic");
    item->menuPath = g_strdup (menuPath != NULL ? menuPath : "");
    item->xembedWindow = 0;

    return item;
}

GracefulPanelTrayItem* graceful_panel_tray_item_copy (const GracefulPanelTrayItem* item)
{
    GracefulPanelTrayItem* copy = NULL;

    if (item == NULL) {
        return NULL;
    }

    copy = graceful_panel_tray_item_new (
        item->id,
        item->busName,
        item->objectPath,
        item->title,
        item->iconName,
        item->menuPath
    );
    copy->xembedWindow = item->xembedWindow;

    return copy;
}

void graceful_panel_tray_item_free (GracefulPanelTrayItem* item)
{
    if (item == NULL) {
        return;
    }

    g_clear_pointer (&item->id, g_free);
    g_clear_pointer (&item->busName, g_free);
    g_clear_pointer (&item->objectPath, g_free);
    g_clear_pointer (&item->title, g_free);
    g_clear_pointer (&item->iconName, g_free);
    g_clear_pointer (&item->menuPath, g_free);
    g_free (item);
}

GPtrArray* graceful_panel_tray_model_list_items (void)
{
    GPtrArray* source = graceful_panel_tray_model_get_items ();
    GPtrArray* items = g_ptr_array_new_with_free_func ((GDestroyNotify) graceful_panel_tray_item_free);

    for (guint i = 0; i < source->len; i++) {
        g_ptr_array_add (items, graceful_panel_tray_item_copy (g_ptr_array_index (source, i)));
    }

    return items;
}

GPtrArray* graceful_panel_tray_model_list_status_notifier_items (void)
{
    GPtrArray* source = graceful_panel_tray_model_get_items ();
    GPtrArray* items = g_ptr_array_new_with_free_func ((GDestroyNotify) graceful_panel_tray_item_free);

    for (guint i = 0; i < source->len; i++) {
        GracefulPanelTrayItem* item = g_ptr_array_index (source, i);

        if (item->xembedWindow == 0) {
            g_ptr_array_add (items, graceful_panel_tray_item_copy (item));
        }
    }

    return items;
}

gboolean graceful_panel_tray_model_has_items (void)
{
    g_autoptr(GPtrArray) items = graceful_panel_tray_model_list_items ();

    return items->len > 0;
}

void graceful_panel_tray_model_upsert_item (
    const char* id,
    const char* busName,
    const char* objectPath,
    const char* title,
    const char* iconName,
    const char* menuPath
)
{
    GPtrArray* items = graceful_panel_tray_model_get_items ();
    GracefulPanelTrayItem* item = NULL;
    gint index = -1;

    if (id == NULL || id[0] == '\0') {
        return;
    }

    item = graceful_panel_tray_item_new (id, busName, objectPath, title, iconName, menuPath);
    index = graceful_panel_tray_model_find_item (id);
    if (index >= 0) {
        g_ptr_array_remove_index (items, (guint) index);
    }

    g_ptr_array_add (items, item);
}

void graceful_panel_tray_model_upsert_xembed_item (guint64 window, const char* title)
{
    GPtrArray* items = graceful_panel_tray_model_get_items ();
    GracefulPanelTrayItem* item = NULL;
    g_autofree char* id = NULL;
    gint index = -1;

    if (window == 0) {
        return;
    }

    id = g_strdup_printf ("xembed:0x%" G_GINT64_MODIFIER "x", window);
    item = graceful_panel_tray_item_new (
        id,
        "",
        "",
        title,
        "application-x-executable-symbolic",
        ""
    );
    item->xembedWindow = window;

    index = graceful_panel_tray_model_find_item (id);
    if (index >= 0) {
        g_ptr_array_remove_index (items, (guint) index);
    }

    g_ptr_array_add (items, item);
}

void graceful_panel_tray_model_remove_item (const char* id)
{
    GPtrArray* items = graceful_panel_tray_model_get_items ();
    gint index = -1;

    if (id == NULL || id[0] == '\0') {
        return;
    }

    index = graceful_panel_tray_model_find_item (id);
    if (index >= 0) {
        g_ptr_array_remove_index (items, (guint) index);
    }
}

void graceful_panel_tray_model_remove_xembed_item (guint64 window)
{
    g_autofree char* id = NULL;

    if (window == 0) {
        return;
    }

    id = g_strdup_printf ("xembed:0x%" G_GINT64_MODIFIER "x", window);
    graceful_panel_tray_model_remove_item (id);
}

void graceful_panel_tray_model_clear (void)
{
    if (gsTrayItems != NULL) {
        g_ptr_array_set_size (gsTrayItems, 0);
    }
}
