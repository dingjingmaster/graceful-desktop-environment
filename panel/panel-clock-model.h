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
#ifndef GRACEFUL_PANEL_PANEL_CLOCK_MODEL_H
#define GRACEFUL_PANEL_PANEL_CLOCK_MODEL_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_PANEL_CLOCK_MODEL (graceful_panel_clock_model_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulPanelClockModel,
    graceful_panel_clock_model,
    GRACEFUL,
    PANEL_CLOCK_MODEL,
    GObject
)

GracefulPanelClockModel* graceful_panel_clock_model_new (void);
char* graceful_panel_clock_model_format_time (GracefulPanelClockModel* model, GDateTime* time);
char* graceful_panel_clock_model_format_date (GracefulPanelClockModel* model, GDateTime* time);

G_END_DECLS

#endif
