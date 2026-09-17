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

#include <glib.h>

static void app_entry_matches_name_case_insensitively (void)
{
    g_autoptr(GracefulPanelAppEntry) entry = graceful_panel_app_entry_new (
        "org.example.Terminal.desktop",
        "Terminal",
        "Use the command line",
        NULL,
        NULL
    );

    g_assert_true (graceful_panel_app_entry_matches (entry, "term"));
    g_assert_true (graceful_panel_app_entry_matches (entry, "TERMINAL"));
}

static void app_entry_matches_description (void)
{
    g_autoptr(GracefulPanelAppEntry) entry = graceful_panel_app_entry_new (
        "org.example.Editor.desktop",
        "Text Editor",
        "Write documents and notes",
        NULL,
        NULL
    );

    g_assert_true (graceful_panel_app_entry_matches (entry, "notes"));
}

static void app_entry_treats_empty_query_as_match (void)
{
    g_autoptr(GracefulPanelAppEntry) entry = graceful_panel_app_entry_new (
        "org.example.Files.desktop",
        "Files",
        "Browse folders",
        NULL,
        NULL
    );

    g_assert_true (graceful_panel_app_entry_matches (entry, ""));
    g_assert_true (graceful_panel_app_entry_matches (entry, NULL));
}

static void app_entry_rejects_unrelated_query (void)
{
    g_autoptr(GracefulPanelAppEntry) entry = graceful_panel_app_entry_new (
        "org.example.Settings.desktop",
        "Settings",
        "Configure the system",
        NULL,
        NULL
    );

    g_assert_false (graceful_panel_app_entry_matches (entry, "browser"));
}

static void app_entry_maps_desktop_categories_to_menu_category (void)
{
    g_assert_cmpstr (
        graceful_panel_app_entry_menu_category_from_desktop_categories ("Network;WebBrowser;"),
        ==,
        "Internet"
    );
    g_assert_cmpstr (
        graceful_panel_app_entry_menu_category_from_desktop_categories ("Development;IDE;"),
        ==,
        "Development"
    );
    g_assert_cmpstr (
        graceful_panel_app_entry_menu_category_from_desktop_categories ("AudioVideo;Player;"),
        ==,
        "Multimedia"
    );
    g_assert_cmpstr (
        graceful_panel_app_entry_menu_category_from_desktop_categories ("Unknown;"),
        ==,
        "Other"
    );
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/panel/app-entry/matches-name-case-insensitively",
        app_entry_matches_name_case_insensitively
    );
    g_test_add_func ("/panel/app-entry/matches-description", app_entry_matches_description);
    g_test_add_func (
        "/panel/app-entry/treats-empty-query-as-match",
        app_entry_treats_empty_query_as_match
    );
    g_test_add_func ("/panel/app-entry/rejects-unrelated-query", app_entry_rejects_unrelated_query);
    g_test_add_func (
        "/panel/app-entry/maps-desktop-categories-to-menu-category",
        app_entry_maps_desktop_categories_to_menu_category
    );

    return g_test_run ();
}
