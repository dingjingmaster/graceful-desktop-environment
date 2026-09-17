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

#include "wallpaper-transition.h"

#include <glib.h>

static void transition_advances_progress_until_complete (void)
{
    g_autoptr(GracefulWallpaperTransition) transition = graceful_wallpaper_transition_new (1000);

    graceful_wallpaper_transition_start (transition);
    g_assert_true (graceful_wallpaper_transition_get_running (transition));
    g_assert_cmpfloat_with_epsilon (graceful_wallpaper_transition_get_progress (transition), 0.0, 0.001);

    graceful_wallpaper_transition_advance (transition, 250);
    g_assert_true (graceful_wallpaper_transition_get_running (transition));
    g_assert_cmpfloat_with_epsilon (graceful_wallpaper_transition_get_progress (transition), 0.25, 0.001);

    graceful_wallpaper_transition_advance (transition, 750);
    g_assert_false (graceful_wallpaper_transition_get_running (transition));
    g_assert_cmpfloat_with_epsilon (graceful_wallpaper_transition_get_progress (transition), 1.0, 0.001);
}

static void transition_clamps_progress_when_delta_exceeds_duration (void)
{
    g_autoptr(GracefulWallpaperTransition) transition = graceful_wallpaper_transition_new (1000);

    graceful_wallpaper_transition_start (transition);
    graceful_wallpaper_transition_advance (transition, 5000);

    g_assert_false (graceful_wallpaper_transition_get_running (transition));
    g_assert_cmpfloat_with_epsilon (graceful_wallpaper_transition_get_progress (transition), 1.0, 0.001);
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/desktop/wallpaper-transition/advances-progress-until-complete",
        transition_advances_progress_until_complete
    );
    g_test_add_func (
        "/desktop/wallpaper-transition/clamps-progress-when-delta-exceeds-duration",
        transition_clamps_progress_when_delta_exceeds_duration
    );

    return g_test_run ();
}
