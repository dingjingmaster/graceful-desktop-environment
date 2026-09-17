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

#include <string.h>

struct _GracefulSessionAutostart
{
    GObject parentInstance;
    char* currentDesktop;
};

G_DEFINE_TYPE (GracefulSessionAutostart, graceful_session_autostart, G_TYPE_OBJECT)

static void graceful_session_autostart_finalize (GObject* object)
{
    GracefulSessionAutostart* self = GRACEFUL_SESSION_AUTOSTART (object);

    g_clear_pointer (&self->currentDesktop, g_free);

    G_OBJECT_CLASS (graceful_session_autostart_parent_class)->finalize (object);
}

static void graceful_session_autostart_class_init (GracefulSessionAutostartClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->finalize = graceful_session_autostart_finalize;
}

static void graceful_session_autostart_init (GracefulSessionAutostart* self)
{
}

static gboolean graceful_session_autostart_string_list_contains (char** values, const char* needle)
{
    for (gsize i = 0; values != NULL && values[i] != NULL; i++) {
        if (g_strcmp0 (values[i], needle) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean graceful_session_autostart_key_matches_desktop (
    GKeyFile* keyFile,
    const char* key,
    const char* currentDesktop,
    gboolean defaultValue
)
{
    g_auto(GStrv) values = NULL;
    g_autoptr(GError) error = NULL;

    values = g_key_file_get_string_list (keyFile, G_KEY_FILE_DESKTOP_GROUP, key, NULL, &error);
    if (error != NULL) {
        return defaultValue;
    }

    return graceful_session_autostart_string_list_contains (values, currentDesktop);
}

static gboolean graceful_session_autostart_try_exec_available (const char* tryExec, const char* pathEnv)
{
    g_auto(GStrv) paths = NULL;
    const char* searchPath = NULL;

    if (tryExec == NULL || tryExec[0] == '\0') {
        return TRUE;
    }

    if (g_path_is_absolute (tryExec)) {
        return g_file_test (tryExec, G_FILE_TEST_IS_EXECUTABLE);
    }

    searchPath = pathEnv != NULL ? pathEnv : g_getenv ("PATH");
    paths = g_strsplit (searchPath != NULL ? searchPath : "", G_SEARCHPATH_SEPARATOR_S, -1);
    for (gsize i = 0; paths != NULL && paths[i] != NULL; i++) {
        g_autofree char* candidate = NULL;

        if (paths[i][0] == '\0') {
            continue;
        }

        candidate = g_build_filename (paths[i], tryExec, NULL);
        if (g_file_test (candidate, G_FILE_TEST_IS_EXECUTABLE)) {
            return TRUE;
        }
    }

    return FALSE;
}

static char* graceful_session_autostart_expand_exec (const char* exec)
{
    GString* expanded = g_string_new (NULL);

    for (gsize i = 0; exec != NULL && exec[i] != '\0'; i++) {
        if (exec[i] != '%') {
            g_string_append_c (expanded, exec[i]);
            continue;
        }

        i++;
        if (exec[i] == '\0') {
            break;
        }

        if (exec[i] == '%') {
            g_string_append_c (expanded, '%');
        }
    }

    return g_string_free (expanded, FALSE);
}

static GStrv graceful_session_autostart_parse_exec (const char* exec)
{
    g_autofree char* expanded = NULL;
    g_auto(GStrv) argv = NULL;
    g_autoptr(GError) error = NULL;

    if (exec == NULL || exec[0] == '\0') {
        return NULL;
    }

    expanded = graceful_session_autostart_expand_exec (exec);
    if (!g_shell_parse_argv (expanded, NULL, &argv, &error)) {
        return NULL;
    }

    return g_steal_pointer (&argv);
}

static gboolean graceful_session_autostart_should_run (
    GracefulSessionAutostart* self,
    GKeyFile* keyFile,
    const char* pathEnv
)
{
    g_autofree char* type = NULL;
    g_autofree char* tryExec = NULL;

    if (g_key_file_get_boolean (keyFile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_HIDDEN, NULL)) {
        return FALSE;
    }

    type = g_key_file_get_string (keyFile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TYPE, NULL);
    if (g_strcmp0 (type, G_KEY_FILE_DESKTOP_TYPE_APPLICATION) != 0) {
        return FALSE;
    }

    if (!graceful_session_autostart_key_matches_desktop (keyFile, "OnlyShowIn", self->currentDesktop, TRUE)) {
        return FALSE;
    }

    if (graceful_session_autostart_key_matches_desktop (keyFile, "NotShowIn", self->currentDesktop, FALSE)) {
        return FALSE;
    }

    if (g_key_file_has_key (keyFile, G_KEY_FILE_DESKTOP_GROUP, "X-GNOME-Autostart-enabled", NULL)
        && !g_key_file_get_boolean (keyFile, G_KEY_FILE_DESKTOP_GROUP, "X-GNOME-Autostart-enabled", NULL)) {
        return FALSE;
    }

    tryExec = g_key_file_get_string (keyFile, G_KEY_FILE_DESKTOP_GROUP, "TryExec", NULL);
    return graceful_session_autostart_try_exec_available (tryExec, pathEnv);
}

static void graceful_session_autostart_collect_dir (GHashTable* desktopFiles, const char* configDir)
{
    g_autofree char* autostartDir = NULL;
    g_autoptr(GDir) dir = NULL;
    const char* name = NULL;

    if (configDir == NULL || configDir[0] == '\0') {
        return;
    }

    autostartDir = g_build_filename (configDir, "autostart", NULL);
    dir = g_dir_open (autostartDir, 0, NULL);
    if (dir == NULL) {
        return;
    }

    while ((name = g_dir_read_name (dir)) != NULL) {
        g_autofree char* path = NULL;

        if (!g_str_has_suffix (name, ".desktop")) {
            continue;
        }

        path = g_build_filename (autostartDir, name, NULL);
        g_hash_table_replace (desktopFiles, g_strdup (name), g_steal_pointer (&path));
    }
}

GracefulSessionAutostart* graceful_session_autostart_new (const char* currentDesktop)
{
    GracefulSessionAutostart* self = g_object_new (GRACEFUL_TYPE_SESSION_AUTOSTART, NULL);

    self->currentDesktop = g_strdup (currentDesktop != NULL && currentDesktop[0] != '\0' ? currentDesktop : "Graceful");

    return self;
}

GPtrArray* graceful_session_autostart_list_commands (
    GracefulSessionAutostart* self,
    const char* const* configDirs,
    const char* pathEnv
)
{
    g_autoptr(GHashTable) desktopFiles = NULL;
    g_autoptr(GList) keys = NULL;
    GPtrArray* commands = NULL;

    g_return_val_if_fail (GRACEFUL_IS_SESSION_AUTOSTART (self), NULL);

    desktopFiles = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    if (configDirs != NULL) {
        gsize dirCount = g_strv_length ((GStrv) configDirs);

        for (gsize i = dirCount; i > 0; i--) {
            graceful_session_autostart_collect_dir (desktopFiles, configDirs[i - 1]);
        }
    }

    keys = g_hash_table_get_keys (desktopFiles);
    commands = g_ptr_array_new_with_free_func ((GDestroyNotify) g_strfreev);

    for (GList* node = keys; node != NULL; node = node->next) {
        const char* desktopId = node->data;
        const char* path = g_hash_table_lookup (desktopFiles, desktopId);
        g_autoptr(GKeyFile) keyFile = g_key_file_new ();
        g_autofree char* exec = NULL;
        GStrv argv = NULL;

        if (!g_key_file_load_from_file (keyFile, path, G_KEY_FILE_NONE, NULL)) {
            continue;
        }

        if (!graceful_session_autostart_should_run (self, keyFile, pathEnv)) {
            continue;
        }

        exec = g_key_file_get_string (keyFile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
        argv = graceful_session_autostart_parse_exec (exec);
        if (argv == NULL || argv[0] == NULL) {
            g_strfreev (argv);
            continue;
        }

        g_ptr_array_add (commands, argv);
    }

    return commands;
}
