/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */

#include "panel-status-notifier-watcher.h"

#include <glib.h>

static void address_uses_sender_for_object_path_registration (void)
{
    g_autoptr(GracefulPanelStatusNotifierAddress) address =
        graceful_panel_status_notifier_address_new ("/StatusNotifierItem", ":1.42");

    g_assert_cmpstr (address->busName, ==, ":1.42");
    g_assert_cmpstr (address->objectPath, ==, "/StatusNotifierItem");
    g_assert_cmpstr (address->id, ==, ":1.42/StatusNotifierItem");
}

static void address_uses_default_path_for_bus_name_registration (void)
{
    g_autoptr(GracefulPanelStatusNotifierAddress) address =
        graceful_panel_status_notifier_address_new ("org.example.Tray", ":1.42");

    g_assert_cmpstr (address->busName, ==, "org.example.Tray");
    g_assert_cmpstr (address->objectPath, ==, "/StatusNotifierItem");
    g_assert_cmpstr (address->id, ==, "org.example.Tray/StatusNotifierItem");
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/panel/status-notifier-watcher/address-uses-sender-for-object-path-registration",
        address_uses_sender_for_object_path_registration
    );
    g_test_add_func (
        "/panel/status-notifier-watcher/address-uses-default-path-for-bus-name-registration",
        address_uses_default_path_for_bus_name_registration
    );

    return g_test_run ();
}
