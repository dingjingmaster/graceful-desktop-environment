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
#ifndef GRACEFUL_PANEL_PANEL_APP_ENTRY_H
#define GRACEFUL_PANEL_PANEL_APP_ENTRY_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _GracefulPanelAppEntry GracefulPanelAppEntry;

struct _GracefulPanelAppEntry
{
    char* id;
    char* name;
    char* description;
    GIcon* icon;
    GAppInfo* appInfo;
};

GracefulPanelAppEntry* graceful_panel_app_entry_new (
    const char* id,
    const char* name,
    const char* description,
    GIcon* icon,
    GAppInfo* appInfo
);
GracefulPanelAppEntry* graceful_panel_app_entry_copy (const GracefulPanelAppEntry* entry);
void graceful_panel_app_entry_free (GracefulPanelAppEntry* entry);
gboolean graceful_panel_app_entry_matches (const GracefulPanelAppEntry* entry, const char* query);
gboolean graceful_panel_app_entry_launch (const GracefulPanelAppEntry* entry, GError** error);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GracefulPanelAppEntry, graceful_panel_app_entry_free)

G_END_DECLS

#endif
