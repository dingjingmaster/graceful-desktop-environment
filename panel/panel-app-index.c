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
#include "panel-app-index.h"

struct _GracefulPanelAppIndex
{
    GObject parentInstance;

    GPtrArray* entries;
};

G_DEFINE_TYPE (GracefulPanelAppIndex, graceful_panel_app_index, G_TYPE_OBJECT)

static gint compare_app_entries (gconstpointer left, gconstpointer right)
{
    const GracefulPanelAppEntry* leftEntry = *((const GracefulPanelAppEntry* const*) left);
    const GracefulPanelAppEntry* rightEntry = *((const GracefulPanelAppEntry* const*) right);

    return g_utf8_collate (leftEntry->name, rightEntry->name);
}

static GPtrArray* create_entry_array (void)
{
    return g_ptr_array_new_with_free_func ((GDestroyNotify) graceful_panel_app_entry_free);
}

static gboolean app_info_should_show (GAppInfo* appInfo)
{
    return g_app_info_should_show (appInfo) && g_app_info_get_name (appInfo) != NULL;
}

static GracefulPanelAppEntry* create_entry_from_app_info (GAppInfo* appInfo)
{
    return graceful_panel_app_entry_new (
        g_app_info_get_id (appInfo),
        g_app_info_get_name (appInfo),
        g_app_info_get_description (appInfo),
        g_app_info_get_icon (appInfo),
        appInfo
    );
}

static void graceful_panel_app_index_dispose (GObject* object)
{
    GracefulPanelAppIndex* self = GRACEFUL_PANEL_APP_INDEX (object);

    g_clear_pointer (&self->entries, g_ptr_array_unref);

    G_OBJECT_CLASS (graceful_panel_app_index_parent_class)->dispose (object);
}

static void graceful_panel_app_index_class_init (GracefulPanelAppIndexClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_app_index_dispose;
}

static void graceful_panel_app_index_init (GracefulPanelAppIndex* self)
{
    self->entries = create_entry_array ();
    graceful_panel_app_index_reload (self);
}

GracefulPanelAppIndex* graceful_panel_app_index_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_APP_INDEX, NULL);
}

void graceful_panel_app_index_reload (GracefulPanelAppIndex* self)
{
    GList* apps = NULL;
    GList* iter = NULL;

    g_return_if_fail (GRACEFUL_IS_PANEL_APP_INDEX (self));

    g_ptr_array_set_size (self->entries, 0);
    apps = g_app_info_get_all ();
    for (iter = apps; iter != NULL; iter = iter->next) {
        GAppInfo* appInfo = G_APP_INFO (iter->data);

        if (app_info_should_show (appInfo)) {
            g_ptr_array_add (self->entries, create_entry_from_app_info (appInfo));
        }
    }
    g_list_free_full (apps, g_object_unref);

    g_ptr_array_sort (self->entries, compare_app_entries);
}

GPtrArray* graceful_panel_app_index_search (GracefulPanelAppIndex* self, const char* query, guint maxResults)
{
    GPtrArray* results = NULL;
    guint i = 0;

    g_return_val_if_fail (GRACEFUL_IS_PANEL_APP_INDEX (self), NULL);

    results = create_entry_array ();
    for (i = 0; i < self->entries->len; ++i) {
        GracefulPanelAppEntry* entry = g_ptr_array_index (self->entries, i);

        if (graceful_panel_app_entry_matches (entry, query)) {
            g_ptr_array_add (results, graceful_panel_app_entry_copy (entry));
            if (maxResults > 0 && results->len >= maxResults) {
                break;
            }
        }
    }

    return results;
}

GPtrArray* graceful_panel_app_index_get_pinned (GracefulPanelAppIndex* self, guint maxResults)
{
    static const char* pinnedQueries[] = {
        "terminal",
        "files",
        "settings",
        "text editor",
        "software",
        "system monitor"
    };
    GPtrArray* pinned = NULL;
    guint i = 0;

    g_return_val_if_fail (GRACEFUL_IS_PANEL_APP_INDEX (self), NULL);

    pinned = create_entry_array ();
    for (i = 0; i < G_N_ELEMENTS (pinnedQueries); ++i) {
        g_autoptr(GPtrArray) matches = graceful_panel_app_index_search (self, pinnedQueries[i], 1);

        if (matches->len > 0) {
            GracefulPanelAppEntry* entry = g_ptr_array_index (matches, 0);

            g_ptr_array_add (pinned, graceful_panel_app_entry_copy (entry));
            if (maxResults > 0 && pinned->len >= maxResults) {
                break;
            }
        }
    }

    if (pinned->len == 0) {
        g_autoptr(GPtrArray) fallback = graceful_panel_app_index_search (self, NULL, maxResults);

        for (i = 0; i < fallback->len; ++i) {
            GracefulPanelAppEntry* entry = g_ptr_array_index (fallback, i);

            g_ptr_array_add (pinned, graceful_panel_app_entry_copy (entry));
        }
    }

    return pinned;
}
