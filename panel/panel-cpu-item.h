/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#ifndef GRACEFUL_PANEL_PANEL_CPU_ITEM_H
#define GRACEFUL_PANEL_PANEL_CPU_ITEM_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_PANEL_CPU_ITEM (graceful_panel_cpu_item_get_type ())

G_DECLARE_FINAL_TYPE (GracefulPanelCpuItem, graceful_panel_cpu_item, GRACEFUL, PANEL_CPU_ITEM, GtkBox)

GtkWidget* graceful_panel_cpu_item_new (void);

G_END_DECLS

#endif
