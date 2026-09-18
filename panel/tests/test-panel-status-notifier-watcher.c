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

static GVariant* create_menu_layout_child (int id, const char* label, gboolean enabled, gboolean separator)
{
    GVariantBuilder properties;
    GVariantBuilder children;

    g_variant_builder_init (&properties, G_VARIANT_TYPE ("a{sv}"));
    if (label != NULL) {
        g_variant_builder_add (&properties, "{sv}", "label", g_variant_new_string (label));
    }
    g_variant_builder_add (&properties, "{sv}", "enabled", g_variant_new_boolean (enabled));
    if (separator) {
        g_variant_builder_add (&properties, "{sv}", "type", g_variant_new_string ("separator"));
    }
    g_variant_builder_init (&children, G_VARIANT_TYPE ("av"));

    return g_variant_ref_sink (
        g_variant_new (
            "(i@a{sv}@av)",
            id,
            g_variant_builder_end (&properties),
            g_variant_builder_end (&children)
        )
    );
}

static void menu_layout_parses_variant_wrapped_properties (void)
{
    g_autoptr(GVariant) disabled = create_menu_layout_child (6, "There are updates", FALSE, FALSE);
    g_autoptr(GVariant) separator = create_menu_layout_child (7, NULL, TRUE, TRUE);
    g_autoptr(GVariant) install = create_menu_layout_child (2, "Install Now", TRUE, FALSE);
    GVariantBuilder rootProperties;
    GVariantBuilder rootChildren;
    g_autoptr(GVariant) layout = NULL;
    g_autoptr(GPtrArray) items = NULL;
    GracefulPanelTrayMenuItem* item = NULL;

    g_variant_builder_init (&rootProperties, G_VARIANT_TYPE ("a{sv}"));
    g_variant_builder_add (&rootProperties, "{sv}", "children-display", g_variant_new_string ("submenu"));
    g_variant_builder_init (&rootChildren, G_VARIANT_TYPE ("av"));
    g_variant_builder_add (&rootChildren, "v", disabled);
    g_variant_builder_add (&rootChildren, "v", separator);
    g_variant_builder_add (&rootChildren, "v", install);
    layout = g_variant_ref_sink (
        g_variant_new (
            "(i@a{sv}@av)",
            0,
            g_variant_builder_end (&rootProperties),
            g_variant_builder_end (&rootChildren)
        )
    );

    items = graceful_panel_status_notifier_menu_items_from_layout (layout);
    g_assert_cmpuint (items->len, ==, 2);

    item = g_ptr_array_index (items, 0);
    g_assert_cmpint (item->id, ==, 6);
    g_assert_cmpstr (item->label, ==, "There are updates");
    g_assert_false (item->enabled);
    g_assert_true (item->visible);

    item = g_ptr_array_index (items, 1);
    g_assert_cmpint (item->id, ==, 2);
    g_assert_cmpstr (item->label, ==, "Install Now");
    g_assert_true (item->enabled);
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
    g_test_add_func (
        "/panel/status-notifier-watcher/menu-layout-parses-variant-wrapped-properties",
        menu_layout_parses_variant_wrapped_properties
    );

    return g_test_run ();
}
