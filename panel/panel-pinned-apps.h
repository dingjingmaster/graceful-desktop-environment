/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#ifndef GRACEFUL_PANEL_PANEL_PINNED_APPS_H
#define GRACEFUL_PANEL_PANEL_PINNED_APPS_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_PANEL_PINNED_APPS (graceful_panel_pinned_apps_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulPanelPinnedApps,
    graceful_panel_pinned_apps,
    GRACEFUL,
    PANEL_PINNED_APPS,
    GObject
)

GracefulPanelPinnedApps* graceful_panel_pinned_apps_new (void);
GracefulPanelPinnedApps* graceful_panel_pinned_apps_new_for_file (const char* path);
gboolean graceful_panel_pinned_apps_has_file (GracefulPanelPinnedApps* self);
gboolean graceful_panel_pinned_apps_is_empty (GracefulPanelPinnedApps* self);
gboolean graceful_panel_pinned_apps_is_pinned (GracefulPanelPinnedApps* self, const char* appId);
void graceful_panel_pinned_apps_pin (GracefulPanelPinnedApps* self, const char* appId);
void graceful_panel_pinned_apps_unpin (GracefulPanelPinnedApps* self, const char* appId);
GStrv graceful_panel_pinned_apps_dup_ids (GracefulPanelPinnedApps* self);

G_END_DECLS

#endif
