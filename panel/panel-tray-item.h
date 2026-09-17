/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#ifndef GRACEFUL_PANEL_PANEL_TRAY_ITEM_H
#define GRACEFUL_PANEL_PANEL_TRAY_ITEM_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_PANEL_TRAY_ITEM (graceful_panel_tray_item_widget_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulPanelTrayItemWidget,
    graceful_panel_tray_item_widget,
    GRACEFUL,
    PANEL_TRAY_ITEM,
    GtkButton
)

GtkWidget* graceful_panel_tray_item_widget_new (void);

G_END_DECLS

#endif
