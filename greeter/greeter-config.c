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
#include "greeter-config.h"

#define GRACEFUL_GREETER_DEFAULT_CONFIG "/etc/lightdm/graceful-greeter.conf"

struct _GracefulGreeterConfig
{
    GObject parentInstance;

    char* background;
};

G_DEFINE_TYPE (GracefulGreeterConfig, graceful_greeter_config, G_TYPE_OBJECT)

static char* normalize_config_text (const char* text)
{
    char* normalized = NULL;

    if (text == NULL) {
        return NULL;
    }

    normalized = g_strdup (text);
    g_strstrip (normalized);

    if (normalized[0] == '\0') {
        g_free (normalized);
        return NULL;
    }

    return normalized;
}

static const char* get_config_path (void)
{
    const char* configPath = g_getenv ("GRACEFUL_GREETER_CONFIG");

    return configPath != NULL && configPath[0] != '\0' ? configPath : GRACEFUL_GREETER_DEFAULT_CONFIG;
}

static char* load_background_from_file (const char* configPath)
{
    g_autoptr(GKeyFile) keyFile = g_key_file_new ();
    g_autoptr(GError) error = NULL;
    g_autofree char* background = NULL;

    if (!g_key_file_load_from_file (keyFile, configPath, G_KEY_FILE_NONE, &error)) {
        return NULL;
    }

    background = g_key_file_get_string (keyFile, "Greeter", "Background", NULL);

    return normalize_config_text (background);
}

static void graceful_greeter_config_finalize (GObject* object)
{
    GracefulGreeterConfig* self = GRACEFUL_GREETER_CONFIG (object);

    g_free (self->background);

    G_OBJECT_CLASS (graceful_greeter_config_parent_class)->finalize (object);
}

static void graceful_greeter_config_class_init (GracefulGreeterConfigClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->finalize = graceful_greeter_config_finalize;
}

static void graceful_greeter_config_init (GracefulGreeterConfig* self)
{
    const char* backgroundOverride = g_getenv ("GRACEFUL_GREETER_BACKGROUND");

    self->background = normalize_config_text (backgroundOverride);

    if (self->background == NULL) {
        self->background = load_background_from_file (get_config_path ());
    }
}

GracefulGreeterConfig* graceful_greeter_config_new (void)
{
    return g_object_new (GRACEFUL_TYPE_GREETER_CONFIG, NULL);
}

const char* graceful_greeter_config_get_background (GracefulGreeterConfig* config)
{
    g_return_val_if_fail (GRACEFUL_IS_GREETER_CONFIG (config), NULL);

    return config->background;
}
