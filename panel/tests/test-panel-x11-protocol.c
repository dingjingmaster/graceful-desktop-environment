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

#include "panel-x11-protocol.h"

#include <glib.h>

static void bottom_strut_reserves_bottom_edge (void)
{
    GracefulPanelX11Strut strut = { { 0 } };

    graceful_panel_x11_calculate_bottom_strut (&strut, 1920, 1080, 40);

    g_assert_cmpuint (strut.values[0], ==, 0);
    g_assert_cmpuint (strut.values[1], ==, 0);
    g_assert_cmpuint (strut.values[2], ==, 0);
    g_assert_cmpuint (strut.values[3], ==, 40);
    g_assert_cmpuint (strut.values[10], ==, 0);
    g_assert_cmpuint (strut.values[11], ==, 1919);
}

static void bottom_placement_tracks_screen_size (void)
{
    GracefulPanelX11Placement placement = { 0 };

    graceful_panel_x11_calculate_bottom_placement (&placement, 1920, 1080, 40);

    g_assert_cmpint (placement.x, ==, 0);
    g_assert_cmpint (placement.y, ==, 1040);
    g_assert_cmpint (placement.width, ==, 1920);
    g_assert_cmpint (placement.height, ==, 40);

    graceful_panel_x11_calculate_bottom_placement (&placement, 1280, 720, 48);

    g_assert_cmpint (placement.x, ==, 0);
    g_assert_cmpint (placement.y, ==, 672);
    g_assert_cmpint (placement.width, ==, 1280);
    g_assert_cmpint (placement.height, ==, 48);
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/panel/x11-protocol/bottom-strut-reserves-bottom-edge", bottom_strut_reserves_bottom_edge);
    g_test_add_func ("/panel/x11-protocol/bottom-placement-tracks-screen-size", bottom_placement_tracks_screen_size);

    return g_test_run ();
}
