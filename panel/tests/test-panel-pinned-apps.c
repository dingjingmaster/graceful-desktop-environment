/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */

#include "panel-pinned-apps.h"

#include <glib.h>
#include <glib/gstdio.h>

static void pinned_apps_persists_pin_and_unpin (void)
{
    g_autofree char* directory = g_dir_make_tmp ("graceful-panel-pins-XXXXXX", NULL);
    g_autofree char* path = g_build_filename (directory, "pinned.ini", NULL);
    g_autoptr(GracefulPanelPinnedApps) pinned = graceful_panel_pinned_apps_new_for_file (path);

    g_assert_false (graceful_panel_pinned_apps_is_pinned (pinned, "org.example.Terminal.desktop"));

    graceful_panel_pinned_apps_pin (pinned, "org.example.Terminal.desktop");
    g_assert_true (graceful_panel_pinned_apps_is_pinned (pinned, "org.example.Terminal.desktop"));

    {
        g_autoptr(GracefulPanelPinnedApps) restored = graceful_panel_pinned_apps_new_for_file (path);

        g_assert_true (
            graceful_panel_pinned_apps_is_pinned (restored, "org.example.Terminal.desktop")
        );
        graceful_panel_pinned_apps_unpin (restored, "org.example.Terminal.desktop");
        g_assert_false (
            graceful_panel_pinned_apps_is_pinned (restored, "org.example.Terminal.desktop")
        );
    }

    {
        g_autoptr(GracefulPanelPinnedApps) restored = graceful_panel_pinned_apps_new_for_file (path);

        g_assert_false (
            graceful_panel_pinned_apps_is_pinned (restored, "org.example.Terminal.desktop")
        );
    }

    g_remove (path);
    g_rmdir (directory);
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/panel/pinned-apps/persists-pin-and-unpin",
        pinned_apps_persists_pin_and_unpin
    );

    return g_test_run ();
}
