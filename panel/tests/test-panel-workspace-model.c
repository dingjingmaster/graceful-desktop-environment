/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */

#include "panel-workspace-model.h"

#include <glib.h>

static void workspace_model_formats_current_and_total_as_one_based_label (void)
{
    g_autofree char* label = graceful_panel_workspace_model_format_label (2, 4);

    g_assert_cmpstr (label, ==, "3/4");
}

static void workspace_model_clamps_invalid_values (void)
{
    g_autofree char* label = graceful_panel_workspace_model_format_label (8, 0);

    g_assert_cmpstr (label, ==, "1/1");
}

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func (
        "/panel/workspace-model/formats-current-and-total-as-one-based-label",
        workspace_model_formats_current_and_total_as_one_based_label
    );
    g_test_add_func (
        "/panel/workspace-model/clamps-invalid-values",
        workspace_model_clamps_invalid_values
    );

    return g_test_run ();
}
