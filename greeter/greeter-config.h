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
#ifndef GRACEFUL_GREETER_GREETER_CONFIG_H
#define GRACEFUL_GREETER_GREETER_CONFIG_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GRACEFUL_TYPE_GREETER_CONFIG (graceful_greeter_config_get_type ())

G_DECLARE_FINAL_TYPE (
    GracefulGreeterConfig,
    graceful_greeter_config,
    GRACEFUL,
    GREETER_CONFIG,
    GObject
)

GracefulGreeterConfig* graceful_greeter_config_new (void);
const char* graceful_greeter_config_get_background (GracefulGreeterConfig* config);

G_END_DECLS

#endif
