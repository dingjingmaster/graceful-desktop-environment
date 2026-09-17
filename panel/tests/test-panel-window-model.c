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

#include <glib.h>

static void visible_task_accepts_normal_application_window (void)
{
    GracefulPanelWindowInfo info = {
        .windowId = 42,
        .title = "Terminal",
        .windowType = GRACEFUL_PANEL_WINDOW_KIND_NORMAL,
        .stateFlags = 0,
        .isViewable = TRUE
    };

    g_assert_true (graceful_panel_window_info_should_show_task (&info));
}

static void visible_task_rejects_shell_windows (void)
{
    GracefulPanelWindowInfo desktop = {
        .windowId = 1,
        .title = "Graceful Desktop",
        .windowType = GRACEFUL_PANEL_WINDOW_KIND_DESKTOP,
        .stateFlags = 0,
        .isViewable = TRUE
    };
    GracefulPanelWindowInfo dock = {
        .windowId = 2,
        .title = "Graceful Panel",
        .windowType = GRACEFUL_PANEL_WINDOW_KIND_DOCK,
        .stateFlags = 0,
        .isViewable = TRUE
    };
    GracefulPanelWindowInfo skipped = {
        .windowId = 3,
        .title = "Hidden Utility",
        .windowType = GRACEFUL_PANEL_WINDOW_KIND_NORMAL,
        .stateFlags = GRACEFUL_PANEL_WINDOW_STATE_SKIP_TASKBAR,
        .isViewable = TRUE
    };

    g_assert_false (graceful_panel_window_info_should_show_task (&desktop));
    g_assert_false (graceful_panel_window_info_should_show_task (&dock));
    g_assert_false (graceful_panel_window_info_should_show_task (&skipped));
}

static void visible_task_rejects_unmapped_windows (void)
{
    GracefulPanelWindowInfo info = {
        .windowId = 4,
        .title = "Editor",
        .windowType = GRACEFUL_PANEL_WINDOW_KIND_NORMAL,
        .stateFlags = 0,
        .isViewable = FALSE
    };

    g_assert_false (graceful_panel_window_info_should_show_task (&info));
}

static void task_title_falls_back_to_application_name (void)
{
    GracefulPanelWindowInfo info = {
        .windowId = 5,
        .title = NULL,
        .applicationName = "Browser",
        .windowType = GRACEFUL_PANEL_WINDOW_KIND_NORMAL,
        .stateFlags = 0,
        .isViewable = TRUE
    };

    g_assert_cmpstr (graceful_panel_window_info_get_display_title (&info), ==, "Browser");
}

static void task_title_uses_unknown_for_untitled_window (void)
{
    GracefulPanelWindowInfo info = {
        .windowId = 6,
        .title = NULL,
        .applicationName = NULL,
        .windowType = GRACEFUL_PANEL_WINDOW_KIND_NORMAL,
        .stateFlags = 0,
        .isViewable = TRUE
    };

    g_assert_cmpstr (graceful_panel_window_info_get_display_title (&info), ==, "Application");
}

static void window_id_list_detects_unchanged_tasks (void)
{
    g_autoptr(GPtrArray) previous = g_ptr_array_new_with_free_func (
        (GDestroyNotify) graceful_panel_window_info_free
    );
    g_autoptr(GPtrArray) current = g_ptr_array_new_with_free_func (
        (GDestroyNotify) graceful_panel_window_info_free
    );

    g_ptr_array_add (previous, graceful_panel_window_info_new (10));
    g_ptr_array_add (previous, graceful_panel_window_info_new (20));
    g_ptr_array_add (current, graceful_panel_window_info_new (10));
    g_ptr_array_add (current, graceful_panel_window_info_new (20));

    g_assert_true (graceful_panel_window_info_list_has_same_window_ids (previous, current));
}

static void window_id_list_detects_changed_tasks (void)
{
    g_autoptr(GPtrArray) previous = g_ptr_array_new_with_free_func (
        (GDestroyNotify) graceful_panel_window_info_free
    );
    g_autoptr(GPtrArray) current = g_ptr_array_new_with_free_func (
        (GDestroyNotify) graceful_panel_window_info_free
    );

    g_ptr_array_add (previous, graceful_panel_window_info_new (10));
    g_ptr_array_add (previous, graceful_panel_window_info_new (20));
    g_ptr_array_add (current, graceful_panel_window_info_new (10));
    g_ptr_array_add (current, graceful_panel_window_info_new (30));

    g_assert_false (graceful_panel_window_info_list_has_same_window_ids (previous, current));
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/panel/window-model/visible-task-accepts-normal-application-window",
        visible_task_accepts_normal_application_window
    );
    g_test_add_func (
        "/panel/window-model/visible-task-rejects-shell-windows",
        visible_task_rejects_shell_windows
    );
    g_test_add_func (
        "/panel/window-model/visible-task-rejects-unmapped-windows",
        visible_task_rejects_unmapped_windows
    );
    g_test_add_func (
        "/panel/window-model/task-title-falls-back-to-application-name",
        task_title_falls_back_to_application_name
    );
    g_test_add_func (
        "/panel/window-model/task-title-uses-unknown-for-untitled-window",
        task_title_uses_unknown_for_untitled_window
    );
    g_test_add_func (
        "/panel/window-model/window-id-list-detects-unchanged-tasks",
        window_id_list_detects_unchanged_tasks
    );
    g_test_add_func (
        "/panel/window-model/window-id-list-detects-changed-tasks",
        window_id_list_detects_changed_tasks
    );

    return g_test_run ();
}
