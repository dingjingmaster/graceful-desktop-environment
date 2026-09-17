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

#include <glib/gstdio.h>

#define PINNED_GROUP "Pinned"
#define PINNED_KEY "applications"

struct _GracefulPanelPinnedApps
{
    GObject parentInstance;

    char* path;
    GPtrArray* ids;
    gboolean hasFile;
};

G_DEFINE_TYPE (GracefulPanelPinnedApps, graceful_panel_pinned_apps, G_TYPE_OBJECT)

static GPtrArray* create_id_array (void)
{
    return g_ptr_array_new_with_free_func (g_free);
}

static char* default_pinned_apps_path (void)
{
    return g_build_filename (g_get_user_config_dir (), "graceful", "panel", "pinned-apps.ini", NULL);
}

static gint find_id_index (GracefulPanelPinnedApps* self, const char* appId)
{
    guint i = 0;

    if (appId == NULL || appId[0] == '\0') {
        return -1;
    }

    for (i = 0; i < self->ids->len; ++i) {
        const char* id = g_ptr_array_index (self->ids, i);

        if (g_strcmp0 (id, appId) == 0) {
            return (gint) i;
        }
    }

    return -1;
}

static void save_pinned_apps (GracefulPanelPinnedApps* self)
{
    g_autoptr(GKeyFile) keyFile = g_key_file_new ();
    g_auto(GStrv) ids = NULL;
    g_autofree char* directory = NULL;
    g_autoptr(GError) error = NULL;

    ids = graceful_panel_pinned_apps_dup_ids (self);
    g_key_file_set_string_list (
        keyFile,
        PINNED_GROUP,
        PINNED_KEY,
        (const char* const*) ids,
        self->ids->len
    );

    directory = g_path_get_dirname (self->path);
    if (g_mkdir_with_parents (directory, 0755) != 0) {
        g_warning ("Failed to create pinned apps config directory: %s", directory);
        return;
    }

    if (!g_key_file_save_to_file (keyFile, self->path, &error)) {
        g_warning ("Failed to save pinned apps: %s", error->message);
        return;
    }

    self->hasFile = TRUE;
}

static void load_pinned_apps (GracefulPanelPinnedApps* self)
{
    g_autoptr(GKeyFile) keyFile = g_key_file_new ();
    g_auto(GStrv) ids = NULL;
    gsize length = 0;
    guint i = 0;

    self->hasFile = g_file_test (self->path, G_FILE_TEST_EXISTS);
    if (!self->hasFile || !g_key_file_load_from_file (keyFile, self->path, G_KEY_FILE_NONE, NULL)) {
        return;
    }

    ids = g_key_file_get_string_list (keyFile, PINNED_GROUP, PINNED_KEY, &length, NULL);
    for (i = 0; ids != NULL && i < length; ++i) {
        if (ids[i] != NULL && ids[i][0] != '\0' && find_id_index (self, ids[i]) < 0) {
            g_ptr_array_add (self->ids, g_strdup (ids[i]));
        }
    }
}

static void graceful_panel_pinned_apps_dispose (GObject* object)
{
    GracefulPanelPinnedApps* self = GRACEFUL_PANEL_PINNED_APPS (object);

    g_clear_pointer (&self->path, g_free);
    g_clear_pointer (&self->ids, g_ptr_array_unref);

    G_OBJECT_CLASS (graceful_panel_pinned_apps_parent_class)->dispose (object);
}

static void graceful_panel_pinned_apps_class_init (GracefulPanelPinnedAppsClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_pinned_apps_dispose;
}

static void graceful_panel_pinned_apps_init (GracefulPanelPinnedApps* self)
{
    self->ids = create_id_array ();
}

GracefulPanelPinnedApps* graceful_panel_pinned_apps_new (void)
{
    g_autofree char* path = default_pinned_apps_path ();

    return graceful_panel_pinned_apps_new_for_file (path);
}

GracefulPanelPinnedApps* graceful_panel_pinned_apps_new_for_file (const char* path)
{
    GracefulPanelPinnedApps* self = g_object_new (GRACEFUL_TYPE_PANEL_PINNED_APPS, NULL);

    self->path = g_strdup (path);
    load_pinned_apps (self);

    return self;
}

gboolean graceful_panel_pinned_apps_has_file (GracefulPanelPinnedApps* self)
{
    g_return_val_if_fail (GRACEFUL_IS_PANEL_PINNED_APPS (self), FALSE);

    return self->hasFile;
}

gboolean graceful_panel_pinned_apps_is_empty (GracefulPanelPinnedApps* self)
{
    g_return_val_if_fail (GRACEFUL_IS_PANEL_PINNED_APPS (self), TRUE);

    return self->ids->len == 0;
}

gboolean graceful_panel_pinned_apps_is_pinned (GracefulPanelPinnedApps* self, const char* appId)
{
    g_return_val_if_fail (GRACEFUL_IS_PANEL_PINNED_APPS (self), FALSE);

    return find_id_index (self, appId) >= 0;
}

void graceful_panel_pinned_apps_pin (GracefulPanelPinnedApps* self, const char* appId)
{
    g_return_if_fail (GRACEFUL_IS_PANEL_PINNED_APPS (self));

    if (find_id_index (self, appId) >= 0 || appId == NULL || appId[0] == '\0') {
        return;
    }

    g_ptr_array_add (self->ids, g_strdup (appId));
    save_pinned_apps (self);
}

void graceful_panel_pinned_apps_unpin (GracefulPanelPinnedApps* self, const char* appId)
{
    gint index = 0;

    g_return_if_fail (GRACEFUL_IS_PANEL_PINNED_APPS (self));

    index = find_id_index (self, appId);
    if (index < 0) {
        return;
    }

    g_ptr_array_remove_index (self->ids, (guint) index);
    save_pinned_apps (self);
}

GStrv graceful_panel_pinned_apps_dup_ids (GracefulPanelPinnedApps* self)
{
    GStrv ids = NULL;
    guint i = 0;

    g_return_val_if_fail (GRACEFUL_IS_PANEL_PINNED_APPS (self), NULL);

    ids = g_new0 (char*, self->ids->len + 1);
    for (i = 0; i < self->ids->len; ++i) {
        ids[i] = g_strdup (g_ptr_array_index (self->ids, i));
    }

    return ids;
}
