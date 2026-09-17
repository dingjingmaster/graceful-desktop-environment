/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef GRACEFUL_PANEL_PANEL_WINDOW_MODEL_H
#define GRACEFUL_PANEL_PANEL_WINDOW_MODEL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef enum _GracefulPanelWindowKind GracefulPanelWindowKind;
typedef enum _GracefulPanelWindowState GracefulPanelWindowState;
typedef struct _GracefulPanelWindowInfo GracefulPanelWindowInfo;

enum _GracefulPanelWindowKind
{
    GRACEFUL_PANEL_WINDOW_KIND_NORMAL,
    GRACEFUL_PANEL_WINDOW_KIND_DESKTOP,
    GRACEFUL_PANEL_WINDOW_KIND_DOCK,
    GRACEFUL_PANEL_WINDOW_KIND_OTHER
};

enum _GracefulPanelWindowState
{
    GRACEFUL_PANEL_WINDOW_STATE_NONE = 0,
    GRACEFUL_PANEL_WINDOW_STATE_SKIP_TASKBAR = 1 << 0,
    GRACEFUL_PANEL_WINDOW_STATE_HIDDEN = 1 << 1
};

struct _GracefulPanelWindowInfo
{
    guint64 windowId;
    char* title;
    char* applicationName;
    GracefulPanelWindowKind windowType;
    guint stateFlags;
    gboolean isViewable;
    int x;
    int y;
    int width;
    int height;
    GdkTexture* iconTexture;
};

GracefulPanelWindowInfo* graceful_panel_window_info_new (guint64 windowId);
void graceful_panel_window_info_free (GracefulPanelWindowInfo* info);
gboolean graceful_panel_window_info_should_show_task (const GracefulPanelWindowInfo* info);
const char* graceful_panel_window_info_get_display_title (const GracefulPanelWindowInfo* info);
gboolean graceful_panel_window_info_list_has_same_window_ids (GPtrArray* previous, GPtrArray* current);

G_END_DECLS

#endif
