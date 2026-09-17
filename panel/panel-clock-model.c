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

struct _GracefulPanelClockModel
{
    GObject parentInstance;
};

G_DEFINE_TYPE (GracefulPanelClockModel, graceful_panel_clock_model, G_TYPE_OBJECT)

static void graceful_panel_clock_model_class_init (GracefulPanelClockModelClass* klass)
{
}

static void graceful_panel_clock_model_init (GracefulPanelClockModel* self)
{
}

GracefulPanelClockModel* graceful_panel_clock_model_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_CLOCK_MODEL, NULL);
}

char* graceful_panel_clock_model_format_time (GracefulPanelClockModel* model, GDateTime* time)
{
    g_return_val_if_fail (GRACEFUL_IS_PANEL_CLOCK_MODEL (model), NULL);
    g_return_val_if_fail (time != NULL, NULL);

    return g_date_time_format (time, "%H:%M:%S");
}

char* graceful_panel_clock_model_format_date (GracefulPanelClockModel* model, GDateTime* time)
{
    static const char* weekdays[] = { "周一", "周二", "周三", "周四", "周五", "周六", "周日" };
    int dayOfWeek = 0;

    g_return_val_if_fail (GRACEFUL_IS_PANEL_CLOCK_MODEL (model), NULL);
    g_return_val_if_fail (time != NULL, NULL);

    dayOfWeek = g_date_time_get_day_of_week (time);

    return g_strdup_printf (
        "%04d/%02d/%02d %s",
        g_date_time_get_year (time),
        g_date_time_get_month (time),
        g_date_time_get_day_of_month (time),
        weekdays[dayOfWeek - 1]
    );
}
