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
#include "panel-window-list.h"

#include <gdk/x11/gdkx.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

struct _GracefulPanelWindowList
{
    GObject parentInstance;
};

G_DEFINE_TYPE (GracefulPanelWindowList, graceful_panel_window_list, G_TYPE_OBJECT)

static Atom intern_atom (Display* display, const char* name)
{
    return XInternAtom (display, name, False);
}

static Display* get_xdisplay (void)
{
    GdkDisplay* display = gdk_display_get_default ();

    if (display == NULL || !GDK_IS_X11_DISPLAY (display)) {
        return NULL;
    }

    return gdk_x11_display_get_xdisplay (display);
}

static gboolean read_atom_list_property (
    Display* display,
    Window window,
    const char* propertyName,
    Atom** values,
    unsigned long* valueCount
)
{
    Atom actualType = 0;
    int actualFormat = 0;
    unsigned long itemCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char* data = NULL;

    *values = NULL;
    *valueCount = 0;

    if (XGetWindowProperty (
            display,
            window,
            intern_atom (display, propertyName),
            0,
            G_MAXLONG,
            False,
            XA_ATOM,
            &actualType,
            &actualFormat,
            &itemCount,
            &bytesAfter,
            &data
        ) != Success) {
        return FALSE;
    }

    if (actualType != XA_ATOM || actualFormat != 32 || itemCount == 0) {
        if (data != NULL) {
            XFree (data);
        }
        return FALSE;
    }

    *values = (Atom*) data;
    *valueCount = itemCount;

    return TRUE;
}

static gboolean atom_list_contains (const Atom* atoms, unsigned long atomCount, Atom expected)
{
    unsigned long i = 0;

    for (i = 0; i < atomCount; ++i) {
        if (atoms[i] == expected) {
            return TRUE;
        }
    }

    return FALSE;
}

static char* read_utf8_property (Display* display, Window window, const char* propertyName)
{
    Atom utf8String = intern_atom (display, "UTF8_STRING");
    Atom actualType = 0;
    int actualFormat = 0;
    unsigned long itemCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char* data = NULL;
    char* value = NULL;

    if (XGetWindowProperty (
            display,
            window,
            intern_atom (display, propertyName),
            0,
            G_MAXLONG,
            False,
            utf8String,
            &actualType,
            &actualFormat,
            &itemCount,
            &bytesAfter,
            &data
        ) != Success) {
        return NULL;
    }

    if (actualType == utf8String && actualFormat == 8 && itemCount > 0) {
        value = g_strndup ((const char*) data, itemCount);
    }

    if (data != NULL) {
        XFree (data);
    }

    return value;
}

static char* read_window_title (Display* display, Window window)
{
    char* title = read_utf8_property (display, window, "_NET_WM_NAME");
    char* fallbackTitle = NULL;

    if (title != NULL && title[0] != '\0') {
        return title;
    }

    g_clear_pointer (&title, g_free);

    if (XFetchName (display, window, &fallbackTitle) == 0 || fallbackTitle == NULL) {
        return NULL;
    }

    title = g_strdup (fallbackTitle);
    XFree (fallbackTitle);

    return title;
}

static char* read_application_name (Display* display, Window window)
{
    XClassHint classHint = { 0 };
    char* applicationName = NULL;

    if (XGetClassHint (display, window, &classHint) == 0) {
        return NULL;
    }

    if (classHint.res_class != NULL && classHint.res_class[0] != '\0') {
        applicationName = g_strdup (classHint.res_class);
    }
    else if (classHint.res_name != NULL && classHint.res_name[0] != '\0') {
        applicationName = g_strdup (classHint.res_name);
    }

    if (classHint.res_name != NULL) {
        XFree (classHint.res_name);
    }
    if (classHint.res_class != NULL) {
        XFree (classHint.res_class);
    }

    return applicationName;
}

static GracefulPanelWindowKind read_window_kind (Display* display, Window window)
{
    Atom* atoms = NULL;
    unsigned long atomCount = 0;
    GracefulPanelWindowKind kind = GRACEFUL_PANEL_WINDOW_KIND_NORMAL;

    if (!read_atom_list_property (display, window, "_NET_WM_WINDOW_TYPE", &atoms, &atomCount)) {
        return kind;
    }

    if (atom_list_contains (atoms, atomCount, intern_atom (display, "_NET_WM_WINDOW_TYPE_DESKTOP"))) {
        kind = GRACEFUL_PANEL_WINDOW_KIND_DESKTOP;
    }
    else if (atom_list_contains (atoms, atomCount, intern_atom (display, "_NET_WM_WINDOW_TYPE_DOCK"))) {
        kind = GRACEFUL_PANEL_WINDOW_KIND_DOCK;
    }

    XFree (atoms);

    return kind;
}

static guint read_state_flags (Display* display, Window window)
{
    Atom* atoms = NULL;
    unsigned long atomCount = 0;
    guint stateFlags = GRACEFUL_PANEL_WINDOW_STATE_NONE;

    if (!read_atom_list_property (display, window, "_NET_WM_STATE", &atoms, &atomCount)) {
        return stateFlags;
    }

    if (atom_list_contains (atoms, atomCount, intern_atom (display, "_NET_WM_STATE_SKIP_TASKBAR"))) {
        stateFlags |= GRACEFUL_PANEL_WINDOW_STATE_SKIP_TASKBAR;
    }
    if (atom_list_contains (atoms, atomCount, intern_atom (display, "_NET_WM_STATE_HIDDEN"))) {
        stateFlags |= GRACEFUL_PANEL_WINDOW_STATE_HIDDEN;
    }

    XFree (atoms);

    return stateFlags;
}

static GdkTexture* create_texture_from_argb (const unsigned long* pixels, int width, int height)
{
    gsize stride = 0;
    guint8* rgba = NULL;
    int x = 0;
    int y = 0;
    GBytes* bytes = NULL;
    GdkTexture* texture = NULL;

    if (width <= 0 || height <= 0 || pixels == NULL) {
        return NULL;
    }

    stride = (gsize) width * 4;
    rgba = g_malloc0 (stride * (gsize) height);
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            guint32 argb = (guint32) pixels[(y * width) + x];
            guint8* pixel = rgba + ((y * stride) + ((gsize) x * 4));

            pixel[0] = (argb >> 16) & 0xff;
            pixel[1] = (argb >> 8) & 0xff;
            pixel[2] = argb & 0xff;
            pixel[3] = (argb >> 24) & 0xff;
        }
    }

    bytes = g_bytes_new_take (rgba, stride * (gsize) height);
    texture = gdk_memory_texture_new (width, height, GDK_MEMORY_R8G8B8A8, bytes, stride);
    g_bytes_unref (bytes);

    return texture;
}

static GdkTexture* read_window_icon (Display* display, Window window)
{
    Atom actualType = 0;
    int actualFormat = 0;
    unsigned long itemCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char* data = NULL;
    unsigned long* values = NULL;
    unsigned long offset = 0;
    unsigned long bestOffset = 0;
    int bestWidth = 0;
    int bestHeight = 0;
    int bestScore = G_MAXINT;
    GdkTexture* texture = NULL;

    if (XGetWindowProperty (
            display,
            window,
            intern_atom (display, "_NET_WM_ICON"),
            0,
            G_MAXLONG,
            False,
            XA_CARDINAL,
            &actualType,
            &actualFormat,
            &itemCount,
            &bytesAfter,
            &data
        ) != Success) {
        return NULL;
    }

    if (actualType != XA_CARDINAL || actualFormat != 32 || itemCount < 3) {
        if (data != NULL) {
            XFree (data);
        }
        return NULL;
    }

    values = (unsigned long*) data;
    while (offset + 2 < itemCount) {
        unsigned long width = values[offset];
        unsigned long height = values[offset + 1];
        unsigned long pixelCount = width * height;
        int score = 0;

        if (width == 0 || height == 0 || offset + 2 + pixelCount > itemCount) {
            break;
        }

        score = ABS ((int) width - 32) + ABS ((int) height - 32);
        if (bestWidth == 0 || score < bestScore) {
            bestOffset = offset + 2;
            bestWidth = (int) width;
            bestHeight = (int) height;
            bestScore = score;
        }

        offset += 2 + pixelCount;
    }

    if (bestWidth > 0 && bestHeight > 0) {
        texture = create_texture_from_argb (values + bestOffset, bestWidth, bestHeight);
    }

    XFree (data);

    return texture;
}

static GPtrArray* read_client_list (Display* display, Window root)
{
    Atom actualType = 0;
    int actualFormat = 0;
    unsigned long itemCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char* data = NULL;
    GPtrArray* windows = NULL;
    unsigned long i = 0;

    windows = g_ptr_array_new ();
    if (XGetWindowProperty (
            display,
            root,
            intern_atom (display, "_NET_CLIENT_LIST"),
            0,
            G_MAXLONG,
            False,
            XA_WINDOW,
            &actualType,
            &actualFormat,
            &itemCount,
            &bytesAfter,
            &data
        ) != Success) {
        return windows;
    }

    if (actualType == XA_WINDOW && actualFormat == 32) {
        unsigned long* values = (unsigned long*) data;

        for (i = 0; i < itemCount; ++i) {
            g_ptr_array_add (windows, GUINT_TO_POINTER ((guint) values[i]));
        }
    }

    if (data != NULL) {
        XFree (data);
    }

    return windows;
}

static GracefulPanelWindowInfo* read_window_info (Display* display, Window window)
{
    XWindowAttributes attributes = { 0 };
    GracefulPanelWindowInfo* info = graceful_panel_window_info_new ((guint64) window);
    Window child = 0;
    int x = 0;
    int y = 0;

    if (XGetWindowAttributes (display, window, &attributes) == 0) {
        info->isViewable = FALSE;
        return info;
    }

    info->isViewable = attributes.map_state == IsViewable;
    info->width = attributes.width;
    info->height = attributes.height;
    if (XTranslateCoordinates (display, window, DefaultRootWindow (display), 0, 0, &x, &y, &child) != 0) {
        info->x = x;
        info->y = y;
    }

    info->title = read_window_title (display, window);
    info->applicationName = read_application_name (display, window);
    info->windowType = read_window_kind (display, window);
    info->stateFlags = read_state_flags (display, window);
    info->iconTexture = read_window_icon (display, window);

    return info;
}

static GdkTexture* create_preview_texture (XImage* image, int targetWidth, int targetHeight)
{
    gsize stride = 0;
    guint8* rgba = NULL;
    int x = 0;
    int y = 0;
    GBytes* bytes = NULL;
    GdkTexture* texture = NULL;

    stride = (gsize) targetWidth * 4;
    rgba = g_malloc0 (stride * (gsize) targetHeight);

    for (y = 0; y < targetHeight; ++y) {
        int sourceY = (y * image->height) / targetHeight;

        for (x = 0; x < targetWidth; ++x) {
            int sourceX = (x * image->width) / targetWidth;
            unsigned long pixelValue = XGetPixel (image, sourceX, sourceY);
            guint8* pixel = rgba + ((y * stride) + ((gsize) x * 4));

            pixel[0] = (pixelValue & image->red_mask) >> 16;
            pixel[1] = (pixelValue & image->green_mask) >> 8;
            pixel[2] = pixelValue & image->blue_mask;
            pixel[3] = 0xff;
        }
    }

    bytes = g_bytes_new_take (rgba, stride * (gsize) targetHeight);
    texture = gdk_memory_texture_new (targetWidth, targetHeight, GDK_MEMORY_R8G8B8A8, bytes, stride);
    g_bytes_unref (bytes);

    return texture;
}

static void graceful_panel_window_list_class_init (GracefulPanelWindowListClass* klass)
{
}

static void graceful_panel_window_list_init (GracefulPanelWindowList* self)
{
}

GracefulPanelWindowList* graceful_panel_window_list_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_WINDOW_LIST, NULL);
}

GPtrArray* graceful_panel_window_list_collect (GracefulPanelWindowList* self)
{
    Display* display = NULL;
    Window root = 0;
    g_autoptr(GPtrArray) clientWindows = NULL;
    GPtrArray* tasks = NULL;
    guint i = 0;

    g_return_val_if_fail (GRACEFUL_IS_PANEL_WINDOW_LIST (self), NULL);

    tasks = g_ptr_array_new_with_free_func ((GDestroyNotify) graceful_panel_window_info_free);
    display = get_xdisplay ();
    if (display == NULL) {
        return tasks;
    }

    root = DefaultRootWindow (display);
    clientWindows = read_client_list (display, root);
    for (i = 0; i < clientWindows->len; ++i) {
        Window window = (Window) GPOINTER_TO_UINT (g_ptr_array_index (clientWindows, i));
        GracefulPanelWindowInfo* info = read_window_info (display, window);

        if (graceful_panel_window_info_should_show_task (info)) {
            g_ptr_array_add (tasks, info);
        }
        else {
            graceful_panel_window_info_free (info);
        }
    }

    return tasks;
}

GdkTexture* graceful_panel_window_list_capture_preview (
    GracefulPanelWindowList* self,
    guint64 windowId,
    int maxWidth,
    int maxHeight
)
{
    GdkDisplay* gdkDisplay = NULL;
    Display* display = NULL;
    Window window = (Window) windowId;
    Window root = 0;
    Window child = 0;
    XWindowAttributes rootAttributes = { 0 };
    XWindowAttributes attributes = { 0 };
    XImage* image = NULL;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int targetWidth = 0;
    int targetHeight = 0;
    GdkTexture* texture = NULL;

    g_return_val_if_fail (GRACEFUL_IS_PANEL_WINDOW_LIST (self), NULL);

    gdkDisplay = gdk_display_get_default ();
    display = get_xdisplay ();
    if (display == NULL || window == 0 || maxWidth <= 0 || maxHeight <= 0) {
        return NULL;
    }

    root = DefaultRootWindow (display);
    if (XGetWindowAttributes (display, root, &rootAttributes) == 0 ||
        XGetWindowAttributes (display, window, &attributes) == 0 ||
        XTranslateCoordinates (display, window, root, 0, 0, &x, &y, &child) == 0) {
        return NULL;
    }

    width = CLAMP (attributes.width, 1, MAX (rootAttributes.width - x, 1));
    height = CLAMP (attributes.height, 1, MAX (rootAttributes.height - y, 1));
    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (width <= 0 || height <= 0) {
        return NULL;
    }

    targetWidth = MIN (width, maxWidth);
    targetHeight = MAX (1, (height * targetWidth) / width);
    if (targetHeight > maxHeight) {
        targetHeight = maxHeight;
        targetWidth = MAX (1, (width * targetHeight) / height);
    }

    gdk_x11_display_error_trap_push (gdkDisplay);
    image = XGetImage (display, root, x, y, (unsigned int) width, (unsigned int) height, AllPlanes, ZPixmap);
    XSync (display, False);
    if (gdk_x11_display_error_trap_pop (gdkDisplay) != 0) {
        if (image != NULL) {
            XDestroyImage (image);
        }
        return NULL;
    }

    if (image == NULL) {
        return NULL;
    }

    texture = create_preview_texture (image, targetWidth, targetHeight);
    XDestroyImage (image);

    return texture;
}
