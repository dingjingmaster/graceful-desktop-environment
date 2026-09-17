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
        "Example Tray",
        "application-x-executable-symbolic"
    );
    g_autoptr(GracefulPanelTrayItem) copy = graceful_panel_tray_item_copy (item);

    g_assert_cmpstr (copy->id, ==, "org.example.Tray");
    g_assert_cmpstr (copy->title, ==, "Example Tray");
    g_assert_cmpstr (copy->iconName, ==, "application-x-executable-symbolic");
}

static void tray_model_reports_empty_when_no_items_exist (void)
{
    g_autoptr(GPtrArray) items = graceful_panel_tray_model_list_items ();

    g_assert_cmpuint (items->len, ==, 0);
    g_assert_false (graceful_panel_tray_model_has_items ());
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/panel/tray-model/tray-item-copies-display-fields", tray_item_copies_display_fields);
    g_test_add_func (
        "/panel/tray-model/reports-empty-when-no-items-exist",
        tray_model_reports_empty_when_no_items_exist
    );

    return g_test_run ();
}
