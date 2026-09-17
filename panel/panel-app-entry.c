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
#include "panel-app-entry.h"

static gboolean text_contains_query (const char* text, const char* query)
{
    g_autofree char* foldedText = NULL;
    g_autofree char* foldedQuery = NULL;

    if (text == NULL || text[0] == '\0') {
        return FALSE;
    }

    foldedText = g_utf8_casefold (text, -1);
    foldedQuery = g_utf8_casefold (query, -1);

    return strstr (foldedText, foldedQuery) != NULL;
}

static gboolean categories_contain (const char* categories, const char* category)
{
    g_autofree char* needle = NULL;

    if (categories == NULL || categories[0] == '\0') {
        return FALSE;
    }

    needle = g_strdup_printf (";%s;", category);

    if (g_str_has_prefix (categories, category) &&
        (categories[strlen (category)] == ';' || categories[strlen (category)] == '\0')) {
        return TRUE;
    }

    return strstr (categories, needle) != NULL;
}

GracefulPanelAppEntry* graceful_panel_app_entry_new (
    const char* id,
    const char* name,
    const char* description,
    GIcon* icon,
    GAppInfo* appInfo
)
{
    return graceful_panel_app_entry_new_with_categories (id, name, description, NULL, icon, appInfo);
}

GracefulPanelAppEntry* graceful_panel_app_entry_new_with_categories (
    const char* id,
    const char* name,
    const char* description,
    const char* desktopCategories,
    GIcon* icon,
    GAppInfo* appInfo
)
{
    GracefulPanelAppEntry* entry = g_new0 (GracefulPanelAppEntry, 1);

    entry->id = g_strdup (id);
    entry->name = g_strdup (name != NULL && name[0] != '\0' ? name : "Application");
    entry->description = g_strdup (description);
    entry->desktopCategories = g_strdup (desktopCategories);
    entry->menuCategory = g_strdup (
        graceful_panel_app_entry_menu_category_from_desktop_categories (desktopCategories)
    );
    entry->icon = icon != NULL ? g_object_ref (icon) : NULL;
    entry->appInfo = appInfo != NULL ? g_object_ref (appInfo) : NULL;

    return entry;
}

GracefulPanelAppEntry* graceful_panel_app_entry_copy (const GracefulPanelAppEntry* entry)
{
    if (entry == NULL) {
        return NULL;
    }

    return graceful_panel_app_entry_new_with_categories (
        entry->id,
        entry->name,
        entry->description,
        entry->desktopCategories,
        entry->icon,
        entry->appInfo
    );
}

void graceful_panel_app_entry_free (GracefulPanelAppEntry* entry)
{
    if (entry == NULL) {
        return;
    }

    g_clear_pointer (&entry->id, g_free);
    g_clear_pointer (&entry->name, g_free);
    g_clear_pointer (&entry->description, g_free);
    g_clear_pointer (&entry->desktopCategories, g_free);
    g_clear_pointer (&entry->menuCategory, g_free);
    g_clear_object (&entry->icon);
    g_clear_object (&entry->appInfo);
    g_free (entry);
}

gboolean graceful_panel_app_entry_matches (const GracefulPanelAppEntry* entry, const char* query)
{
    if (entry == NULL) {
        return FALSE;
    }

    if (query == NULL || query[0] == '\0') {
        return TRUE;
    }

    return text_contains_query (entry->name, query) ||
        text_contains_query (entry->description, query) ||
        text_contains_query (entry->id, query);
}

gboolean graceful_panel_app_entry_launch (const GracefulPanelAppEntry* entry, GError** error)
{
    if (entry == NULL || entry->appInfo == NULL) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, "Application entry cannot be launched");
        return FALSE;
    }

    return g_app_info_launch (entry->appInfo, NULL, NULL, error);
}

const char* graceful_panel_app_entry_menu_category_from_desktop_categories (const char* categories)
{
    if (categories_contain (categories, "Network")) {
        return "Internet";
    }
    if (categories_contain (categories, "Office")) {
        return "Office";
    }
    if (categories_contain (categories, "Development")) {
        return "Development";
    }
    if (categories_contain (categories, "Graphics")) {
        return "Graphics";
    }
    if (categories_contain (categories, "AudioVideo") || categories_contain (categories, "Audio") ||
        categories_contain (categories, "Video")) {
        return "Multimedia";
    }
    if (categories_contain (categories, "Game")) {
        return "Games";
    }
    if (categories_contain (categories, "Settings") || categories_contain (categories, "System")) {
        return "System";
    }
    if (categories_contain (categories, "Utility")) {
        return "Utilities";
    }

    return "Other";
}
