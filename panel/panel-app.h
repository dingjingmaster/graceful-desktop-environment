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
#ifndef GRACEFUL_PANEL_PANEL_APP_H
#define GRACEFUL_PANEL_PANEL_APP_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_PANEL_APP (graceful_panel_app_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulPanelApp,
    graceful_panel_app,
    GRACEFUL,
    PANEL_APP,
    GObject
)

GracefulPanelApp* graceful_panel_app_new (void);
int graceful_panel_app_run (GracefulPanelApp* app, int argc, char** argv);

G_END_DECLS

#endif
