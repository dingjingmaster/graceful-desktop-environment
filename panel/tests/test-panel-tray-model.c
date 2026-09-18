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

#include <glib.h>

static void tray_item_copies_display_fields (void)
{
    g_autoptr(GracefulPanelTrayItem) item = graceful_panel_tray_item_new (
        "org.example.Tray",
        "org.example.Tray",
        "/StatusNotifierItem",
        "Example Tray",
        "application-x-executable-symbolic",
        "/Menu"
    );
    g_autoptr(GracefulPanelTrayItem) copy = graceful_panel_tray_item_copy (item);

    g_assert_cmpstr (copy->id, ==, "org.example.Tray");
    g_assert_cmpstr (copy->busName, ==, "org.example.Tray");
    g_assert_cmpstr (copy->objectPath, ==, "/StatusNotifierItem");
    g_assert_cmpstr (copy->title, ==, "Example Tray");
    g_assert_cmpstr (copy->iconName, ==, "application-x-executable-symbolic");
    g_assert_cmpstr (copy->menuPath, ==, "/Menu");
}

static void tray_model_reports_empty_when_no_items_exist (void)
{
    g_autoptr(GPtrArray) items = NULL;

    graceful_panel_tray_model_clear ();
    items = graceful_panel_tray_model_list_items ();
    g_assert_cmpuint (items->len, ==, 0);
    g_assert_false (graceful_panel_tray_model_has_items ());
}

static void tray_model_upserts_and_removes_items_by_id (void)
{
    g_autoptr(GPtrArray) items = NULL;
    GracefulPanelTrayItem* item = NULL;

    graceful_panel_tray_model_clear ();
    graceful_panel_tray_model_upsert_item (
        "org.example.Status",
        "org.example.Status",
        "/StatusNotifierItem",
        "First",
        "first-icon",
        "/FirstMenu"
    );
    graceful_panel_tray_model_upsert_item (
        "org.example.Status",
        "org.example.Status",
        "/StatusNotifierItem",
        "Second",
        "second-icon",
        "/SecondMenu"
    );

    items = graceful_panel_tray_model_list_items ();
    g_assert_cmpuint (items->len, ==, 1);
    item = g_ptr_array_index (items, 0);
    g_assert_cmpstr (item->id, ==, "org.example.Status");
    g_assert_cmpstr (item->busName, ==, "org.example.Status");
    g_assert_cmpstr (item->objectPath, ==, "/StatusNotifierItem");
    g_assert_cmpstr (item->title, ==, "Second");
    g_assert_cmpstr (item->iconName, ==, "second-icon");
    g_assert_cmpstr (item->menuPath, ==, "/SecondMenu");
    g_assert_true (graceful_panel_tray_model_has_items ());

    graceful_panel_tray_model_remove_item ("org.example.Status");
    g_assert_false (graceful_panel_tray_model_has_items ());
}

static void tray_model_upserts_and_removes_xembed_items_by_window (void)
{
    g_autoptr(GPtrArray) items = NULL;
    GracefulPanelTrayItem* item = NULL;

    graceful_panel_tray_model_clear ();
    graceful_panel_tray_model_upsert_xembed_item (0x2400012, "Wendun");
    graceful_panel_tray_model_upsert_xembed_item (0x2400012, "Wendun Client");

    items = graceful_panel_tray_model_list_items ();
    g_assert_cmpuint (items->len, ==, 1);
    item = g_ptr_array_index (items, 0);
    g_assert_cmpstr (item->id, ==, "xembed:0x2400012");
    g_assert_cmpstr (item->title, ==, "Wendun Client");
    g_assert_cmpstr (item->iconName, ==, "application-x-executable-symbolic");
    g_assert_cmpuint (item->xembedWindow, ==, 0x2400012);
    g_assert_true (graceful_panel_tray_model_has_items ());

    graceful_panel_tray_model_remove_xembed_item (0x2400012);
    g_assert_false (graceful_panel_tray_model_has_items ());
}

static void tray_model_lists_status_notifier_items_without_xembed_items (void)
{
    g_autoptr(GPtrArray) items = NULL;
    GracefulPanelTrayItem* item = NULL;

    graceful_panel_tray_model_clear ();
    graceful_panel_tray_model_upsert_xembed_item (0x2400012, "Legacy Tray");
    graceful_panel_tray_model_upsert_item (
        ":1.42/StatusNotifierItem",
        ":1.42",
        "/StatusNotifierItem",
        "Modern Tray",
        "modern-icon",
        "/Menu"
    );

    items = graceful_panel_tray_model_list_status_notifier_items ();
    g_assert_cmpuint (items->len, ==, 1);
    item = g_ptr_array_index (items, 0);
    g_assert_cmpstr (item->id, ==, ":1.42/StatusNotifierItem");
    g_assert_cmpstr (item->title, ==, "Modern Tray");
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/panel/tray-model/tray-item-copies-display-fields", tray_item_copies_display_fields);
    g_test_add_func (
        "/panel/tray-model/reports-empty-when-no-items-exist",
        tray_model_reports_empty_when_no_items_exist
    );
    g_test_add_func (
        "/panel/tray-model/upserts-and-removes-items-by-id",
        tray_model_upserts_and_removes_items_by_id
    );
    g_test_add_func (
        "/panel/tray-model/upserts-and-removes-xembed-items-by-window",
        tray_model_upserts_and_removes_xembed_items_by_window
    );
    g_test_add_func (
        "/panel/tray-model/lists-status-notifier-items-without-xembed-items",
        tray_model_lists_status_notifier_items_without_xembed_items
    );

    return g_test_run ();
}
