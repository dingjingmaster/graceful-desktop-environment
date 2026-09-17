/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#include "panel-launcher-command.h"

char* graceful_panel_launcher_command_resolve (const char* const* candidates)
{
    guint i = 0;

    if (candidates == NULL) {
        return NULL;
    }

    for (i = 0; candidates[i] != NULL; ++i) {
        g_autofree char* path = g_find_program_in_path (candidates[i]);

        if (path != NULL) {
            return g_strdup (candidates[i]);
        }
    }

    return NULL;
}

gboolean graceful_panel_launcher_command_launch (const char* const* candidates, GError** error)
{
    g_autofree char* command = graceful_panel_launcher_command_resolve (candidates);
    g_autoptr(GAppInfo) appInfo = NULL;

    if (command == NULL) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, "No launcher candidate was found");
        return FALSE;
    }

    appInfo = g_app_info_create_from_commandline (
        command,
        NULL,
        G_APP_INFO_CREATE_NONE,
        error
    );
    if (appInfo == NULL) {
        return FALSE;
    }

    return g_app_info_launch (appInfo, NULL, NULL, error);
}
