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
#ifndef GRACEFUL_PANEL_PANEL_X11_PROTOCOL_H
#define GRACEFUL_PANEL_PANEL_X11_PROTOCOL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _GracefulPanelX11Strut GracefulPanelX11Strut;
typedef struct _GracefulPanelX11Placement GracefulPanelX11Placement;

struct _GracefulPanelX11Strut
{
    unsigned long values[12];
};

struct _GracefulPanelX11Placement
{
    int x;
    int y;
    int width;
    int height;
};

void graceful_panel_x11_calculate_bottom_strut (
    GracefulPanelX11Strut* strut,
    int screenWidth,
    int screenHeight,
    int panelHeight
);
void graceful_panel_x11_calculate_bottom_placement (
    GracefulPanelX11Placement* placement,
    int screenWidth,
    int screenHeight,
    int panelHeight
);
void graceful_panel_x11_apply_dock_window (GtkWindow* window, int panelHeight);

G_END_DECLS

#endif
