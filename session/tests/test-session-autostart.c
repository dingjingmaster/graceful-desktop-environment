/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */

#include "session-autostart.h"

#include <glib.h>
#include <glib/gstdio.h>

static void write_file_checked (const char* path, const char* contents)
{
    g_autoptr(GError) error = NULL;

    g_assert_true (g_file_set_contents (path, contents, -1, &error));
    g_assert_no_error (error);
}

static char* build_autostart_file (const char* dir, const char* name)
{
    return g_build_filename (dir, "autostart", name, NULL);
}

static void autostart_filters_desktop_files_and_expands_exec (void)
{
    g_autoptr(GError) error = NULL;
    g_autofree char* root = g_dir_make_tmp ("graceful-autostart-XXXXXX", &error);
    g_autofree char* systemConfig = g_build_filename (root, "system", NULL);
    g_autofree char* userConfig = g_build_filename (root, "user", NULL);
    g_autofree char* systemAutostart = g_build_filename (systemConfig, "autostart", NULL);
    g_autofree char* userAutostart = g_build_filename (userConfig, "autostart", NULL);
    g_autofree char* binDir = g_build_filename (root, "bin", NULL);
    g_autofree char* helper = g_build_filename (binDir, "helper", NULL);
    g_autofree char* keep = build_autostart_file (systemConfig, "keep.desktop");
    g_autofree char* hidden = build_autostart_file (systemConfig, "hidden.desktop");
    g_autofree char* onlyOther = build_autostart_file (systemConfig, "only-other.desktop");
    g_autofree char* notGraceful = build_autostart_file (systemConfig, "not-graceful.desktop");
    g_autofree char* missingTryExec = build_autostart_file (systemConfig, "missing-tryexec.desktop");
    g_autofree char* overriddenSystem = build_autostart_file (systemConfig, "override.desktop");
    g_autofree char* overriddenUser = build_autostart_file (userConfig, "override.desktop");
    const char* configDirs[3] = { userConfig, systemConfig, NULL };
    g_autoptr(GracefulSessionAutostart) autostart = graceful_session_autostart_new ("Graceful");
    g_autoptr(GPtrArray) commands = NULL;
    const char* const* argv = NULL;

    g_assert_no_error (error);
    g_assert_cmpint (g_mkdir_with_parents (systemAutostart, 0755), ==, 0);
    g_assert_cmpint (g_mkdir_with_parents (userAutostart, 0755), ==, 0);
    g_assert_cmpint (g_mkdir_with_parents (binDir, 0755), ==, 0);
    write_file_checked (helper, "#!/bin/sh\nexit 0\n");
    g_assert_cmpint (g_chmod (helper, 0755), ==, 0);

    write_file_checked (
        keep,
        "[Desktop Entry]\nType=Application\nName=Keep\nTryExec=helper\nExec=helper --name %u %%\nOnlyShowIn=Graceful;GNOME;\n"
    );
    write_file_checked (hidden, "[Desktop Entry]\nType=Application\nName=Hidden\nExec=helper\nHidden=true\n");
    write_file_checked (onlyOther, "[Desktop Entry]\nType=Application\nName=Other\nExec=helper\nOnlyShowIn=GNOME;\n");
    write_file_checked (notGraceful, "[Desktop Entry]\nType=Application\nName=No\nExec=helper\nNotShowIn=Graceful;\n");
    write_file_checked (
        missingTryExec,
        "[Desktop Entry]\nType=Application\nName=Missing\nTryExec=does-not-exist\nExec=helper\n"
    );
    write_file_checked (overriddenSystem, "[Desktop Entry]\nType=Application\nName=System\nExec=helper --system\n");
    write_file_checked (overriddenUser, "[Desktop Entry]\nType=Application\nName=User\nExec=helper --user\nHidden=true\n");

    commands = graceful_session_autostart_list_commands (
        autostart,
        configDirs,
        binDir
    );

    g_assert_cmpuint (commands->len, ==, 1);
    argv = g_ptr_array_index (commands, 0);
    g_assert_cmpstr (argv[0], ==, "helper");
    g_assert_cmpstr (argv[1], ==, "--name");
    g_assert_cmpstr (argv[2], ==, "%");
    g_assert_null (argv[3]);
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/session/autostart/filters-desktop-files-and-expands-exec",
        autostart_filters_desktop_files_and_expands_exec
    );

    return g_test_run ();
}
