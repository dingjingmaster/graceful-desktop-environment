/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#ifndef GRACEFUL_PANEL_PANEL_LAUNCHER_COMMAND_H
#define GRACEFUL_PANEL_PANEL_LAUNCHER_COMMAND_H

#include <gio/gio.h>

G_BEGIN_DECLS

char* graceful_panel_launcher_command_resolve (const char* const* candidates);
gboolean graceful_panel_launcher_command_launch (const char* const* candidates, GError** error);

G_END_DECLS

#endif
