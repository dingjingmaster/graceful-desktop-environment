/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */
#include "panel-tray-item.h"

#include "panel-tray-model.h"

#define TRAY_ARROW_ANIMATION_US 160000
#define TRAY_VISIBILITY_REFRESH_INTERVAL_MS 1000

struct _GracefulPanelTrayItemWidget
{
    GtkButton parentInstance;

    GtkWidget* arrowArea;
    GtkWidget* popover;
    GtkWidget* itemBox;
    guint animationTickId;
    guint visibilityTimerId;
    gint64 animationStartTimeUs;
    double animationStartAngle;
    double animationTargetAngle;
    double arrowAngle;
};

G_DEFINE_TYPE (GracefulPanelTrayItemWidget, graceful_panel_tray_item_widget, GTK_TYPE_BUTTON)

static void set_tray_arrow_target (GracefulPanelTrayItemWidget* self, double targetAngle);

static void update_tray_visibility (GracefulPanelTrayItemWidget* self)
{
    gboolean hasItems = graceful_panel_tray_model_has_items ();

    gtk_widget_set_visible (GTK_WIDGET (self), hasItems);
    if (!hasItems && gtk_widget_get_mapped (self->popover)) {
        gtk_popover_popdown (GTK_POPOVER (self->popover));
    }
}

static gboolean on_visibility_timer (gpointer userData)
{
    update_tray_visibility (GRACEFUL_PANEL_TRAY_ITEM (userData));

    return G_SOURCE_CONTINUE;
}

static void draw_tray_arrow (
    GtkDrawingArea* area,
    cairo_t* cr,
    int width,
    int height,
    gpointer userData
)
{
    GracefulPanelTrayItemWidget* self = GRACEFUL_PANEL_TRAY_ITEM (userData);
    double cx = (double) width / 2.0;
    double cy = (double) height / 2.0;

    cairo_save (cr);
    cairo_translate (cr, cx, cy);
    cairo_rotate (cr, self->arrowAngle);
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.96);
    cairo_set_line_width (cr, 2.0);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    cairo_move_to (cr, -5.0, 3.0);
    cairo_line_to (cr, 0.0, -3.0);
    cairo_line_to (cr, 5.0, 3.0);
    cairo_stroke (cr);
    cairo_restore (cr);
}

static gboolean on_arrow_animation_tick (
    GtkWidget* widget,
    GdkFrameClock* frameClock,
    gpointer userData
)
{
    GracefulPanelTrayItemWidget* self = GRACEFUL_PANEL_TRAY_ITEM (userData);
    gint64 elapsedUs = g_get_monotonic_time () - self->animationStartTimeUs;
    double progress = CLAMP ((double) elapsedUs / (double) TRAY_ARROW_ANIMATION_US, 0.0, 1.0);
    double eased = 1.0 - ((1.0 - progress) * (1.0 - progress));

    self->arrowAngle = self->animationStartAngle +
        ((self->animationTargetAngle - self->animationStartAngle) * eased);
    gtk_widget_queue_draw (self->arrowArea);

    if (progress >= 1.0) {
        self->arrowAngle = self->animationTargetAngle;
        self->animationTickId = 0;
        gtk_widget_queue_draw (self->arrowArea);
        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}

static void stop_arrow_animation (GracefulPanelTrayItemWidget* self)
{
    if (self->animationTickId != 0) {
        gtk_widget_remove_tick_callback (self->arrowArea, self->animationTickId);
        self->animationTickId = 0;
    }
}

static void set_tray_arrow_target (GracefulPanelTrayItemWidget* self, double targetAngle)
{
    stop_arrow_animation (self);
    self->animationStartAngle = self->arrowAngle;
    self->animationTargetAngle = targetAngle;
    self->animationStartTimeUs = g_get_monotonic_time ();
    self->animationTickId = gtk_widget_add_tick_callback (
        self->arrowArea,
        on_arrow_animation_tick,
        self,
        NULL
    );
}

static void clear_box_children (GtkWidget* box)
{
    GtkWidget* child = gtk_widget_get_first_child (box);

    while (child != NULL) {
        GtkWidget* next = gtk_widget_get_next_sibling (child);

        gtk_box_remove (GTK_BOX (box), child);
        child = next;
    }
}

static GtkWidget* create_tray_row (GracefulPanelTrayItem* item)
{
    GtkWidget* row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget* iconBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* icon = gtk_image_new_from_icon_name (item->iconName);
    GtkWidget* label = gtk_label_new (item->title);

    gtk_widget_add_css_class (row, "panel-tray-row");
    gtk_widget_add_css_class (iconBox, "panel-tray-icon-box");
    gtk_widget_add_css_class (label, "panel-tray-label");
    gtk_widget_set_size_request (iconBox, 18, 18);
    gtk_image_set_icon_size (GTK_IMAGE (icon), GTK_ICON_SIZE_NORMAL);
    gtk_widget_set_hexpand (label, TRUE);
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
    gtk_box_append (GTK_BOX (iconBox), icon);
    gtk_box_append (GTK_BOX (row), iconBox);
    gtk_box_append (GTK_BOX (row), label);

    return row;
}

static void populate_tray_menu (GracefulPanelTrayItemWidget* self)
{
    g_autoptr(GPtrArray) items = graceful_panel_tray_model_list_items ();
    guint i = 0;

    clear_box_children (self->itemBox);
    if (items->len == 0) {
        GtkWidget* empty = gtk_label_new ("No tray items");

        gtk_widget_add_css_class (empty, "panel-tray-empty-label");
        gtk_box_append (GTK_BOX (self->itemBox), empty);
        return;
    }

    for (i = 0; i < items->len; ++i) {
        gtk_box_append (GTK_BOX (self->itemBox), create_tray_row (g_ptr_array_index (items, i)));
    }
}

static GtkWidget* create_tray_popover (GracefulPanelTrayItemWidget* self)
{
    GtkWidget* popover = gtk_popover_new ();

    self->itemBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class (popover, "panel-tray-popover");
    gtk_widget_add_css_class (self->itemBox, "panel-tray-menu");
    gtk_popover_set_child (GTK_POPOVER (popover), self->itemBox);
    gtk_popover_set_has_arrow (GTK_POPOVER (popover), FALSE);
    gtk_popover_set_position (GTK_POPOVER (popover), GTK_POS_TOP);
    gtk_widget_set_parent (popover, GTK_WIDGET (self));

    return popover;
}

static void on_tray_button_clicked (GtkButton* button, gpointer userData)
{
    GracefulPanelTrayItemWidget* self = GRACEFUL_PANEL_TRAY_ITEM (button);

    if (!graceful_panel_tray_model_has_items ()) {
        gtk_widget_set_visible (GTK_WIDGET (self), FALSE);
        return;
    }

    populate_tray_menu (self);
    gtk_popover_popup (GTK_POPOVER (self->popover));
    set_tray_arrow_target (self, G_PI);
}

static void on_tray_popover_closed (GtkPopover* popover, gpointer userData)
{
    set_tray_arrow_target (GRACEFUL_PANEL_TRAY_ITEM (userData), 0.0);
}

static void graceful_panel_tray_item_widget_dispose (GObject* object)
{
    GracefulPanelTrayItemWidget* self = GRACEFUL_PANEL_TRAY_ITEM (object);

    stop_arrow_animation (self);
    if (self->visibilityTimerId != 0) {
        g_source_remove (self->visibilityTimerId);
        self->visibilityTimerId = 0;
    }

    if (self->popover != NULL) {
        gtk_popover_popdown (GTK_POPOVER (self->popover));
        if (gtk_widget_get_parent (self->popover) != NULL) {
            gtk_widget_unparent (self->popover);
        }
        g_clear_object (&self->popover);
    }

    G_OBJECT_CLASS (graceful_panel_tray_item_widget_parent_class)->dispose (object);
}

static void graceful_panel_tray_item_widget_class_init (GracefulPanelTrayItemWidgetClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_tray_item_widget_dispose;
}

static void graceful_panel_tray_item_widget_init (GracefulPanelTrayItemWidget* self)
{
    self->arrowArea = gtk_drawing_area_new ();
    gtk_widget_set_size_request (self->arrowArea, 18, 18);
    gtk_drawing_area_set_draw_func (
        GTK_DRAWING_AREA (self->arrowArea),
        draw_tray_arrow,
        self,
        NULL
    );
    gtk_button_set_child (GTK_BUTTON (self), self->arrowArea);
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-icon-button");
    gtk_widget_add_css_class (GTK_WIDGET (self), "panel-tray-button");
    gtk_widget_set_tooltip_text (GTK_WIDGET (self), "Tray");
    self->popover = create_tray_popover (self);
    g_object_ref_sink (self->popover);
    g_signal_connect (self->popover, "closed", G_CALLBACK (on_tray_popover_closed), self);
    g_signal_connect (self, "clicked", G_CALLBACK (on_tray_button_clicked), NULL);
    update_tray_visibility (self);
    self->visibilityTimerId = g_timeout_add (
        TRAY_VISIBILITY_REFRESH_INTERVAL_MS,
        on_visibility_timer,
        self
    );
}

GtkWidget* graceful_panel_tray_item_widget_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_TRAY_ITEM, NULL);
}
