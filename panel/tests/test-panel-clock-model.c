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

#include "panel-clock-model.h"

#include <glib.h>

static void clock_model_formats_time_with_seconds (void)
{
    g_autoptr(GracefulPanelClockModel) model = graceful_panel_clock_model_new ();
    g_autoptr(GDateTime) time = g_date_time_new_local (2026, 9, 17, 8, 5, 9);
    g_autofree char* label = graceful_panel_clock_model_format_time (model, time);

    g_assert_cmpstr (label, ==, "08:05:09");
}

static void clock_model_formats_date_with_weekday (void)
{
    g_autoptr(GracefulPanelClockModel) model = graceful_panel_clock_model_new ();
    g_autoptr(GDateTime) time = g_date_time_new_local (2026, 9, 17, 8, 5, 9);
    g_autofree char* label = graceful_panel_clock_model_format_date (model, time);

    g_assert_cmpstr (label, ==, "2026/09/17 周四");
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/panel/clock-model/formats-time-with-seconds",
        clock_model_formats_time_with_seconds
    );
    g_test_add_func (
        "/panel/clock-model/formats-date-with-weekday",
        clock_model_formats_date_with_weekday
    );

    return g_test_run ();
}
