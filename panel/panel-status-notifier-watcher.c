/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies or substantial portions of the Software.
 */

#include "panel-status-notifier-watcher.h"

#include "panel-tray-model.h"

#define STATUS_NOTIFIER_WATCHER_NAME "org.kde.StatusNotifierWatcher"
#define STATUS_NOTIFIER_WATCHER_PATH "/StatusNotifierWatcher"
#define STATUS_NOTIFIER_WATCHER_INTERFACE "org.kde.StatusNotifierWatcher"
#define STATUS_NOTIFIER_ITEM_INTERFACE "org.kde.StatusNotifierItem"

struct _GracefulPanelStatusNotifierWatcher
{
    GObject parentInstance;
    GDBusConnection* connection;
    GDBusNodeInfo* introspection;
    GHashTable* watchIds;
    guint nameOwnerId;
    guint objectId;
    gboolean hostRegistered;
};

typedef struct _TrayPropertiesRequest TrayPropertiesRequest;

struct _TrayPropertiesRequest
{
    char* id;
    char* busName;
    char* objectPath;
};

G_DEFINE_TYPE (GracefulPanelStatusNotifierWatcher, graceful_panel_status_notifier_watcher, G_TYPE_OBJECT)

static const char* gsWatcherIntrospection =
    "<node>"
    "  <interface name='org.kde.StatusNotifierWatcher'>"
    "    <method name='RegisterStatusNotifierItem'>"
    "      <arg type='s' name='service' direction='in'/>"
    "    </method>"
    "    <method name='RegisterStatusNotifierHost'>"
    "      <arg type='s' name='service' direction='in'/>"
    "    </method>"
    "    <property name='RegisteredStatusNotifierItems' type='as' access='read'/>"
    "    <property name='IsStatusNotifierHostRegistered' type='b' access='read'/>"
    "    <property name='ProtocolVersion' type='i' access='read'/>"
    "    <signal name='StatusNotifierItemRegistered'>"
    "      <arg type='s' name='service'/>"
    "    </signal>"
    "    <signal name='StatusNotifierItemUnregistered'>"
    "      <arg type='s' name='service'/>"
    "    </signal>"
    "    <signal name='StatusNotifierHostRegistered'/>"
    "  </interface>"
    "</node>";

static void tray_properties_request_free (TrayPropertiesRequest* request)
{
    if (request == NULL) {
        return;
    }

    g_clear_pointer (&request->id, g_free);
    g_clear_pointer (&request->busName, g_free);
    g_clear_pointer (&request->objectPath, g_free);
    g_free (request);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (TrayPropertiesRequest, tray_properties_request_free)

static GVariant* graceful_panel_status_notifier_watcher_registered_items_variant (void)
{
    g_autoptr(GPtrArray) items = graceful_panel_tray_model_list_status_notifier_items ();
    GVariantBuilder builder;

    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));
    for (guint i = 0; i < items->len; i++) {
        GracefulPanelTrayItem* item = g_ptr_array_index (items, i);

        g_variant_builder_add (&builder, "s", item->id);
    }

    return g_variant_builder_end (&builder);
}

GracefulPanelStatusNotifierAddress* graceful_panel_status_notifier_address_new (
    const char* service,
    const char* sender
)
{
    GracefulPanelStatusNotifierAddress* address = NULL;

    if (service == NULL || service[0] == '\0') {
        return NULL;
    }

    address = g_new0 (GracefulPanelStatusNotifierAddress, 1);
    if (service[0] == '/') {
        address->busName = g_strdup (sender);
        address->objectPath = g_strdup (service);
    }
    else {
        address->busName = g_strdup (service);
        address->objectPath = g_strdup ("/StatusNotifierItem");
    }

    address->id = g_strdup_printf ("%s%s", address->busName, address->objectPath);

    return address;
}

void graceful_panel_status_notifier_address_free (GracefulPanelStatusNotifierAddress* address)
{
    if (address == NULL) {
        return;
    }

    g_clear_pointer (&address->id, g_free);
    g_clear_pointer (&address->busName, g_free);
    g_clear_pointer (&address->objectPath, g_free);
    g_free (address);
}

void graceful_panel_tray_menu_item_free (GracefulPanelTrayMenuItem* item)
{
    if (item == NULL) {
        return;
    }

    g_clear_pointer (&item->label, g_free);
    g_free (item);
}

static void emit_item_signal (GracefulPanelStatusNotifierWatcher* self, const char* signalName, const char* id)
{
    if (self->connection == NULL) {
        return;
    }

    g_dbus_connection_emit_signal (
        self->connection,
        NULL,
        STATUS_NOTIFIER_WATCHER_PATH,
        STATUS_NOTIFIER_WATCHER_INTERFACE,
        signalName,
        g_variant_new ("(s)", id),
        NULL
    );
}

static void on_registered_name_vanished (GDBusConnection* connection, const char* name, gpointer userData)
{
    GracefulPanelStatusNotifierWatcher* self = GRACEFUL_PANEL_STATUS_NOTIFIER_WATCHER (userData);
    GHashTableIter iter;
    gpointer key = NULL;
    gpointer value = NULL;

    g_hash_table_iter_init (&iter, self->watchIds);
    while (g_hash_table_iter_next (&iter, &key, &value)) {
        const char* id = key;

        if (g_str_has_prefix (id, name)) {
            graceful_panel_tray_model_remove_item (id);
            emit_item_signal (self, "StatusNotifierItemUnregistered", id);
            g_hash_table_iter_remove (&iter);
            return;
        }
    }
}

static void on_registered_name_appeared (GDBusConnection* connection, const char* name, const char* nameOwner, gpointer userData)
{
}

static void watch_registered_item (GracefulPanelStatusNotifierWatcher* self, GracefulPanelStatusNotifierAddress* address)
{
    guint watchId = 0;

    if (g_hash_table_contains (self->watchIds, address->id)) {
        return;
    }

    watchId = g_bus_watch_name_on_connection (
        self->connection,
        address->busName,
        G_BUS_NAME_WATCHER_FLAGS_NONE,
        on_registered_name_appeared,
        on_registered_name_vanished,
        self,
        NULL
    );
    g_hash_table_insert (self->watchIds, g_strdup (address->id), GUINT_TO_POINTER (watchId));
}

static char* lookup_string_property (GVariant* properties, const char* key)
{
    g_autoptr(GVariant) value = NULL;
    g_autoptr(GVariant) unwrapped = NULL;
    GVariant* effective = NULL;
    char* text = NULL;

    value = g_variant_lookup_value (properties, key, NULL);
    if (value == NULL) {
        return NULL;
    }

    if (g_variant_is_of_type (value, G_VARIANT_TYPE_VARIANT)) {
        unwrapped = g_variant_get_variant (value);
    }

    effective = unwrapped != NULL ? unwrapped : value;
    if (g_variant_is_of_type (effective, G_VARIANT_TYPE_STRING)) {
        text = g_strdup (g_variant_get_string (effective, NULL));
    }

    return text;
}

static char* lookup_object_path_property (GVariant* properties, const char* key)
{
    g_autoptr(GVariant) value = NULL;
    g_autoptr(GVariant) unwrapped = NULL;
    GVariant* effective = NULL;
    char* text = NULL;

    value = g_variant_lookup_value (properties, key, NULL);
    if (value == NULL) {
        return NULL;
    }

    if (g_variant_is_of_type (value, G_VARIANT_TYPE_VARIANT)) {
        unwrapped = g_variant_get_variant (value);
    }

    effective = unwrapped != NULL ? unwrapped : value;
    if (g_variant_is_of_type (effective, G_VARIANT_TYPE_OBJECT_PATH)) {
        text = g_strdup (g_variant_get_string (effective, NULL));
    }

    return text;
}

static gboolean lookup_bool_property (GVariant* properties, const char* key, gboolean fallback)
{
    g_autoptr(GVariant) value = NULL;
    g_autoptr(GVariant) unwrapped = NULL;
    GVariant* effective = NULL;
    gboolean result = fallback;

    value = g_variant_lookup_value (properties, key, NULL);
    if (value == NULL) {
        return fallback;
    }

    if (g_variant_is_of_type (value, G_VARIANT_TYPE_VARIANT)) {
        unwrapped = g_variant_get_variant (value);
    }

    effective = unwrapped != NULL ? unwrapped : value;
    if (g_variant_is_of_type (effective, G_VARIANT_TYPE_BOOLEAN)) {
        result = g_variant_get_boolean (effective);
    }

    return result;
}

static void on_item_action_done (GObject* sourceObject, GAsyncResult* result, gpointer userData)
{
    GDBusConnection* connection = G_DBUS_CONNECTION (sourceObject);
    g_autoptr(GVariant) reply = NULL;
    g_autoptr(GError) error = NULL;

    reply = g_dbus_connection_call_finish (connection, result, &error);
    if (reply == NULL) {
        g_warning ("StatusNotifierItem action failed: %s", error->message);
    }
}

static void call_status_notifier_item_method (const GracefulPanelTrayItem* item, const char* methodName, int x, int y)
{
    g_autoptr(GDBusConnection) connection = NULL;
    g_autoptr(GError) error = NULL;

    if (item == NULL || item->busName == NULL || item->busName[0] == '\0') {
        return;
    }

    if (item->objectPath == NULL || item->objectPath[0] != '/') {
        return;
    }

    connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
    if (connection == NULL) {
        g_warning ("Failed to connect to session bus for tray action: %s", error->message);
        return;
    }

    g_dbus_connection_call (
        connection,
        item->busName,
        item->objectPath,
        STATUS_NOTIFIER_ITEM_INTERFACE,
        methodName,
        g_variant_new ("(ii)", x, y),
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        800,
        NULL,
        on_item_action_done,
        NULL
    );
}

void graceful_panel_status_notifier_item_activate (const GracefulPanelTrayItem* item, int x, int y)
{
    call_status_notifier_item_method (item, "Activate", x, y);
}

void graceful_panel_status_notifier_item_context_menu (const GracefulPanelTrayItem* item, int x, int y)
{
    call_status_notifier_item_method (item, "ContextMenu", x, y);
}

static GracefulPanelTrayMenuItem* tray_menu_item_from_layout (GVariant* layout)
{
    GVariant* properties = NULL;
    GracefulPanelTrayMenuItem* item = NULL;
    gboolean enabled = TRUE;
    gboolean visible = TRUE;
    int id = 0;
    g_autofree char* label = NULL;
    g_autofree char* type = NULL;

    if (layout == NULL || g_variant_n_children (layout) < 3) {
        return NULL;
    }

    g_variant_get_child (layout, 0, "i", &id);
    properties = g_variant_get_child_value (layout, 1);
    label = lookup_string_property (properties, "label");
    type = lookup_string_property (properties, "type");
    enabled = lookup_bool_property (properties, "enabled", TRUE);
    visible = lookup_bool_property (properties, "visible", TRUE);

    if (g_strcmp0 (type, "separator") == 0 || label == NULL || label[0] == '\0') {
        g_variant_unref (properties);
        return NULL;
    }

    item = g_new0 (GracefulPanelTrayMenuItem, 1);
    item->id = id;
    item->label = g_steal_pointer (&label);
    item->enabled = enabled;
    item->visible = visible;
    g_variant_unref (properties);

    return item;
}

static void append_tray_menu_children (GPtrArray* items, GVariant* layout)
{
    g_autoptr(GVariant) children = NULL;
    gsize childCount = 0;

    if (layout == NULL || g_variant_n_children (layout) < 3) {
        return;
    }

    children = g_variant_get_child_value (layout, 2);
    childCount = g_variant_n_children (children);
    for (gsize i = 0; i < childCount; i++) {
        g_autoptr(GVariant) childValue = g_variant_get_child_value (children, i);
        g_autoptr(GVariant) childLayout = NULL;
        GracefulPanelTrayMenuItem* item = NULL;

        if (g_variant_is_of_type (childValue, G_VARIANT_TYPE_VARIANT)) {
            childLayout = g_variant_get_variant (childValue);
        }
        else {
            childLayout = g_variant_ref (childValue);
        }

        item = tray_menu_item_from_layout (childLayout);
        if (item != NULL) {
            g_ptr_array_add (items, item);
        }
    }
}

GPtrArray* graceful_panel_status_notifier_menu_items_from_layout (GVariant* layout)
{
    GPtrArray* items = g_ptr_array_new_with_free_func ((GDestroyNotify) graceful_panel_tray_menu_item_free);

    append_tray_menu_children (items, layout);

    return items;
}

GPtrArray* graceful_panel_status_notifier_item_load_menu (const GracefulPanelTrayItem* item)
{
    g_autoptr(GDBusConnection) connection = NULL;
    g_autoptr(GVariant) reply = NULL;
    g_autoptr(GVariant) propertyNames = NULL;
    g_autoptr(GVariant) layout = NULL;
    g_autoptr(GError) error = NULL;
    GVariantBuilder builder;
    guint revision = 0;
    GPtrArray* items = g_ptr_array_new_with_free_func ((GDestroyNotify) graceful_panel_tray_menu_item_free);

    if (item == NULL || item->busName == NULL || item->busName[0] == '\0') {
        return items;
    }

    if (item->menuPath == NULL || item->menuPath[0] != '/') {
        return items;
    }

    connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
    if (connection == NULL) {
        g_warning ("Failed to connect to session bus for tray menu: %s", error->message);
        return items;
    }

    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));
    propertyNames = g_variant_ref_sink (g_variant_builder_end (&builder));
    reply = g_dbus_connection_call_sync (
        connection,
        item->busName,
        item->menuPath,
        "com.canonical.dbusmenu",
        "GetLayout",
        g_variant_new ("(ii@as)", 0, 1, g_steal_pointer (&propertyNames)),
        G_VARIANT_TYPE ("(u(ia{sv}av))"),
        G_DBUS_CALL_FLAGS_NONE,
        800,
        NULL,
        &error
    );
    if (reply == NULL) {
        g_warning ("Failed to load tray menu layout: %s", error->message);
        return items;
    }

    g_variant_get (reply, "(u@(ia{sv}av))", &revision, &layout);
    g_ptr_array_unref (items);
    items = graceful_panel_status_notifier_menu_items_from_layout (layout);

    return items;
}

void graceful_panel_status_notifier_menu_item_click (const GracefulPanelTrayItem* item, int menuItemId)
{
    g_autoptr(GDBusConnection) connection = NULL;
    g_autoptr(GError) error = NULL;
    guint32 timestamp = (guint32) (g_get_monotonic_time () / 1000);

    if (item == NULL || item->busName == NULL || item->busName[0] == '\0') {
        return;
    }

    if (item->menuPath == NULL || item->menuPath[0] != '/') {
        return;
    }

    connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
    if (connection == NULL) {
        g_warning ("Failed to connect to session bus for tray menu event: %s", error->message);
        return;
    }

    g_dbus_connection_call (
        connection,
        item->busName,
        item->menuPath,
        "com.canonical.dbusmenu",
        "Event",
        g_variant_new ("(isvu)", menuItemId, "clicked", g_variant_new_int32 (0), timestamp),
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        800,
        NULL,
        on_item_action_done,
        NULL
    );
}

static void on_item_properties_loaded (GObject* sourceObject, GAsyncResult* result, gpointer userData)
{
    GDBusConnection* connection = G_DBUS_CONNECTION (sourceObject);
    g_autoptr(TrayPropertiesRequest) request = userData;
    g_autoptr(GVariant) reply = NULL;
    g_autoptr(GError) error = NULL;
    GVariant* properties = NULL;
    g_autofree char* title = NULL;
    g_autofree char* iconName = NULL;
    g_autofree char* menuPath = NULL;

    reply = g_dbus_connection_call_finish (connection, result, &error);
    if (reply == NULL) {
        return;
    }

    g_variant_get (reply, "(@a{sv})", &properties);
    title = lookup_string_property (properties, "Title");
    iconName = lookup_string_property (properties, "IconName");
    menuPath = lookup_object_path_property (properties, "Menu");
    graceful_panel_tray_model_upsert_item (
        request->id,
        request->busName,
        request->objectPath,
        title,
        iconName,
        menuPath
    );
    g_variant_unref (properties);
}

static void load_item_properties (GracefulPanelStatusNotifierWatcher* self, GracefulPanelStatusNotifierAddress* address)
{
    TrayPropertiesRequest* request = g_new0 (TrayPropertiesRequest, 1);

    request->id = g_strdup (address->id);
    request->busName = g_strdup (address->busName);
    request->objectPath = g_strdup (address->objectPath);
    g_dbus_connection_call (
        self->connection,
        address->busName,
        address->objectPath,
        "org.freedesktop.DBus.Properties",
        "GetAll",
        g_variant_new ("(s)", STATUS_NOTIFIER_ITEM_INTERFACE),
        G_VARIANT_TYPE ("(a{sv})"),
        G_DBUS_CALL_FLAGS_NONE,
        800,
        NULL,
        on_item_properties_loaded,
        request
    );
}

static void register_status_notifier_item (
    GracefulPanelStatusNotifierWatcher* self,
    const char* service,
    const char* sender
)
{
    g_autoptr(GracefulPanelStatusNotifierAddress) address =
        graceful_panel_status_notifier_address_new (service, sender);

    if (address == NULL || address->busName == NULL || address->busName[0] == '\0') {
        return;
    }

    graceful_panel_tray_model_upsert_item (
        address->id,
        address->busName,
        address->objectPath,
        address->busName,
        "application-x-executable-symbolic",
        NULL
    );
    watch_registered_item (self, address);
    emit_item_signal (self, "StatusNotifierItemRegistered", address->id);
    load_item_properties (self, address);
}

static void handle_method_call (
    GDBusConnection* connection,
    const char* sender,
    const char* objectPath,
    const char* interfaceName,
    const char* methodName,
    GVariant* parameters,
    GDBusMethodInvocation* invocation,
    gpointer userData
)
{
    GracefulPanelStatusNotifierWatcher* self = GRACEFUL_PANEL_STATUS_NOTIFIER_WATCHER (userData);
    const char* service = NULL;

    if (g_strcmp0 (methodName, "RegisterStatusNotifierItem") == 0) {
        g_variant_get (parameters, "(&s)", &service);
        register_status_notifier_item (self, service, sender);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (methodName, "RegisterStatusNotifierHost") == 0) {
        self->hostRegistered = TRUE;
        g_dbus_connection_emit_signal (
            self->connection,
            NULL,
            STATUS_NOTIFIER_WATCHER_PATH,
            STATUS_NOTIFIER_WATCHER_INTERFACE,
            "StatusNotifierHostRegistered",
            NULL,
            NULL
        );
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    g_dbus_method_invocation_return_error (
        invocation,
        G_IO_ERROR,
        G_IO_ERROR_NOT_SUPPORTED,
        "Unsupported method '%s'",
        methodName
    );
}

static GVariant* handle_get_property (
    GDBusConnection* connection,
    const char* sender,
    const char* objectPath,
    const char* interfaceName,
    const char* propertyName,
    GError** error,
    gpointer userData
)
{
    GracefulPanelStatusNotifierWatcher* self = GRACEFUL_PANEL_STATUS_NOTIFIER_WATCHER (userData);

    if (g_strcmp0 (propertyName, "RegisteredStatusNotifierItems") == 0) {
        return graceful_panel_status_notifier_watcher_registered_items_variant ();
    }

    if (g_strcmp0 (propertyName, "IsStatusNotifierHostRegistered") == 0) {
        return g_variant_new_boolean (self->hostRegistered);
    }

    if (g_strcmp0 (propertyName, "ProtocolVersion") == 0) {
        return g_variant_new_int32 (0);
    }

    return NULL;
}

static const GDBusInterfaceVTable gsWatcherVtable = {
    handle_method_call,
    handle_get_property,
    NULL,
    { 0 }
};

static void on_bus_acquired (GDBusConnection* connection, const char* name, gpointer userData)
{
    GracefulPanelStatusNotifierWatcher* self = GRACEFUL_PANEL_STATUS_NOTIFIER_WATCHER (userData);
    GDBusInterfaceInfo* interfaceInfo = self->introspection->interfaces[0];
    g_autoptr(GError) error = NULL;

    self->connection = g_object_ref (connection);
    self->objectId = g_dbus_connection_register_object (
        connection,
        STATUS_NOTIFIER_WATCHER_PATH,
        interfaceInfo,
        &gsWatcherVtable,
        self,
        NULL,
        &error
    );
    if (error != NULL) {
        g_warning ("Failed to register StatusNotifierWatcher object: %s", error->message);
    }
}

static void on_name_acquired (GDBusConnection* connection, const char* name, gpointer userData)
{
}

static void on_name_lost (GDBusConnection* connection, const char* name, gpointer userData)
{
    g_warning ("Failed to own %s", STATUS_NOTIFIER_WATCHER_NAME);
}

static void graceful_panel_status_notifier_watcher_dispose (GObject* object)
{
    GracefulPanelStatusNotifierWatcher* self = GRACEFUL_PANEL_STATUS_NOTIFIER_WATCHER (object);

    if (self->nameOwnerId != 0) {
        g_bus_unown_name (self->nameOwnerId);
        self->nameOwnerId = 0;
    }

    if (self->objectId != 0 && self->connection != NULL) {
        g_dbus_connection_unregister_object (self->connection, self->objectId);
        self->objectId = 0;
    }

    if (self->watchIds != NULL) {
        GHashTableIter iter;
        gpointer value = NULL;

        g_hash_table_iter_init (&iter, self->watchIds);
        while (g_hash_table_iter_next (&iter, NULL, &value)) {
            g_bus_unwatch_name (GPOINTER_TO_UINT (value));
        }
        g_hash_table_remove_all (self->watchIds);
    }

    g_clear_object (&self->connection);

    G_OBJECT_CLASS (graceful_panel_status_notifier_watcher_parent_class)->dispose (object);
}

static void graceful_panel_status_notifier_watcher_finalize (GObject* object)
{
    GracefulPanelStatusNotifierWatcher* self = GRACEFUL_PANEL_STATUS_NOTIFIER_WATCHER (object);

    g_clear_pointer (&self->watchIds, g_hash_table_unref);
    g_clear_pointer (&self->introspection, g_dbus_node_info_unref);

    G_OBJECT_CLASS (graceful_panel_status_notifier_watcher_parent_class)->finalize (object);
}

static void graceful_panel_status_notifier_watcher_class_init (GracefulPanelStatusNotifierWatcherClass* klass)
{
    GObjectClass* objectClass = G_OBJECT_CLASS (klass);

    objectClass->dispose = graceful_panel_status_notifier_watcher_dispose;
    objectClass->finalize = graceful_panel_status_notifier_watcher_finalize;
}

static void graceful_panel_status_notifier_watcher_init (GracefulPanelStatusNotifierWatcher* self)
{
    g_autoptr(GError) error = NULL;

    self->watchIds = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    self->introspection = g_dbus_node_info_new_for_xml (gsWatcherIntrospection, &error);
    if (error != NULL) {
        g_error ("Invalid StatusNotifierWatcher introspection XML: %s", error->message);
    }
}

GracefulPanelStatusNotifierWatcher* graceful_panel_status_notifier_watcher_new (void)
{
    return g_object_new (GRACEFUL_TYPE_PANEL_STATUS_NOTIFIER_WATCHER, NULL);
}

void graceful_panel_status_notifier_watcher_start (GracefulPanelStatusNotifierWatcher* self)
{
    g_return_if_fail (GRACEFUL_IS_PANEL_STATUS_NOTIFIER_WATCHER (self));

    if (self->nameOwnerId != 0) {
        return;
    }

    self->hostRegistered = TRUE;
    self->nameOwnerId = g_bus_own_name (
        G_BUS_TYPE_SESSION,
        STATUS_NOTIFIER_WATCHER_NAME,
        G_BUS_NAME_OWNER_FLAGS_NONE,
        on_bus_acquired,
        on_name_acquired,
        on_name_lost,
        g_object_ref (self),
        g_object_unref
    );
}
