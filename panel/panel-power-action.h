/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#ifndef GRACEFUL_PANEL_PANEL_POWER_ACTION_H
#define GRACEFUL_PANEL_PANEL_POWER_ACTION_H

#include <gio/gio.h>

G_BEGIN_DECLS

typedef enum
{
    GRACEFUL_PANEL_POWER_ACTION_SHUTDOWN,
    GRACEFUL_PANEL_POWER_ACTION_REBOOT,
    GRACEFUL_PANEL_POWER_ACTION_LOGOUT,
    GRACEFUL_PANEL_POWER_ACTION_LOCK
} GracefulPanelPowerAction;

const char* graceful_panel_power_action_get_label (GracefulPanelPowerAction action);
GPtrArray* graceful_panel_power_action_build_candidates (GracefulPanelPowerAction action);
gboolean graceful_panel_power_action_run (GracefulPanelPowerAction action, GError** error);

G_END_DECLS

#endif
