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
#include "panel-window-model.h"

GracefulPanelWindowInfo* graceful_panel_window_info_new (guint64 windowId)
{
    GracefulPanelWindowInfo* info = g_new0 (GracefulPanelWindowInfo, 1);

    info->windowId = windowId;
    info->windowType = GRACEFUL_PANEL_WINDOW_KIND_NORMAL;
    info->isViewable = TRUE;

    return info;
}

void graceful_panel_window_info_free (GracefulPanelWindowInfo* info)
{
    if (info == NULL) {
        return;
    }

    g_clear_pointer (&info->title, g_free);
    g_clear_pointer (&info->applicationName, g_free);
    g_clear_object (&info->iconTexture);
    g_free (info);
}

gboolean graceful_panel_window_info_should_show_task (const GracefulPanelWindowInfo* info)
{
    if (info == NULL || !info->isViewable) {
        return FALSE;
    }

    if (info->windowType == GRACEFUL_PANEL_WINDOW_KIND_DESKTOP ||
        info->windowType == GRACEFUL_PANEL_WINDOW_KIND_DOCK) {
        return FALSE;
    }

    if ((info->stateFlags & GRACEFUL_PANEL_WINDOW_STATE_SKIP_TASKBAR) != 0) {
        return FALSE;
    }

    return TRUE;
}

const char* graceful_panel_window_info_get_display_title (const GracefulPanelWindowInfo* info)
{
    if (info == NULL) {
        return "Application";
    }

    if (info->title != NULL && info->title[0] != '\0') {
        return info->title;
    }

    if (info->applicationName != NULL && info->applicationName[0] != '\0') {
        return info->applicationName;
    }

    return "Application";
}

gboolean graceful_panel_window_info_list_has_same_window_ids (GPtrArray* previous, GPtrArray* current)
{
    guint i = 0;

    if (previous == NULL || current == NULL || previous->len != current->len) {
        return FALSE;
    }

    for (i = 0; i < previous->len; ++i) {
        GracefulPanelWindowInfo* previousInfo = g_ptr_array_index (previous, i);
        GracefulPanelWindowInfo* currentInfo = g_ptr_array_index (current, i);

        if (previousInfo == NULL || currentInfo == NULL || previousInfo->windowId != currentInfo->windowId) {
            return FALSE;
        }
    }

    return TRUE;
}
