#ifndef EDBUS_CONNECTION_H
#define EDBUS_CONNECTION_H 1

/**
 * @defgroup EDBus_Conneciton Connection
 *
 * @{
 */
typedef enum
{
   EDBUS_CONNECTION_TYPE_UNKNOWN = 0,       /**< sentinel, not a real type */
   EDBUS_CONNECTION_TYPE_SESSION,
   EDBUS_CONNECTION_TYPE_SYSTEM,
   EDBUS_CONNECTION_TYPE_STARTER,
   EDBUS_CONNECTION_TYPE_LAST       /**< sentinel, not a real type */
} EDBus_Connection_Type;

#define EDBUS_TIMEOUT_INFINITE ((int) 0x7fffffff)

/**
 * Establish a connection to bus and integrate it with the ecore main
 * loop. If a connection of given type was already created before, its
 * reference counter is incremented and the connection is returned.
 *
 * @param type type of connection e.g EDBUS_CONNECTION_TYPE_SESSION,
 * EDBUS_CONNECTION_TYPE_SYSTEM or EDBUS_CONNECTION_TYPE_STARTER
 *
 * @return connection with bus
 */
EAPI EDBus_Connection *edbus_connection_get(EDBus_Connection_Type type);

/**
 * Always create and establish a new connection to bus and integrate it with
 * the ecore main loop. Differently from edbus_connection_get(), this function
 * guarantees to create a new connection to the D-Bus daemon and the connection
 * is not shared by any means.
 *
 * @param type type of connection e.g EDBUS_CONNECTION_TYPE_SESSION,
 * EDBUS_CONNECTION_TYPE_SYSTEM or EDBUS_CONNECTION_TYPE_STARTER
 *
 * @return connection with bus
 */
EAPI EDBus_Connection *edbus_private_connection_get(EDBus_Connection_Type type);

/**
 * @brief Increment connection reference count.
 *
 * @param conn The given EDBus_Connection object to reference
 */
EAPI EDBus_Connection *edbus_connection_ref(EDBus_Connection *conn) EINA_ARG_NONNULL(1);

/**
 * @brief Decrement connection reference count.
 *
 * If reference count reaches 0, the connection to bus will be dropped and all
 * its children will be invalidated.
 */
EAPI void              edbus_connection_unref(EDBus_Connection *conn) EINA_ARG_NONNULL(1);

/**
 * @brief Add a callback function to be called when connection is freed
 *
 * @param conn The connection object to add the callback to.
 * @param cb callback to be called
 * @param data data passed to callback
 */
EAPI void              edbus_connection_free_cb_add(EDBus_Connection *conn, EDBus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Remove callback registered in edbus_connection_free_cb_add().
 */
EAPI void              edbus_connection_free_cb_del(EDBus_Connection *conn, EDBus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Set an attached data pointer to an object with a given string key.
 *
 * @param conn The connection object to store data to
 * @param key to identify data
 * @param data data that will be stored
 */
EAPI void              edbus_connection_data_set(EDBus_Connection *conn, const char *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Get data stored in connection.
 *
 * @param conn connection where data is stored
 * @param key key that identifies data
 *
 * @return pointer to data if found otherwise NULL
 */
EAPI void             *edbus_connection_data_get(const EDBus_Connection *conn, const char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Del data stored in connection.
 *
 * @param conn connection where data is stored
 * @param key that identifies data
 *
 * @return pointer to data if found otherwise NULL
 */
EAPI void             *edbus_connection_data_del(EDBus_Connection *conn, const char *key) EINA_ARG_NONNULL(1, 2);

typedef enum
{
   EDBUS_CONNECTION_EVENT_DEL,
   EDBUS_CONNECTION_EVENT_DISCONNECTED,
   EDBUS_CONNECTION_EVENT_LAST    /**< sentinel, not a real event type */
} EDBus_Connection_Event_Type;

typedef void (*EDBus_Connection_Event_Cb)(void *data, EDBus_Connection *conn, void *event_info);

/**
 * @brief Add a callback function to be called when an event occurs of the
 * type passed.
 */
EAPI void                  edbus_connection_event_callback_add(EDBus_Connection *conn, EDBus_Connection_Event_Type type, EDBus_Connection_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Remove callback registered in edbus_connection_event_callback_add().
 */
EAPI void                  edbus_connection_event_callback_del(EDBus_Connection *conn, EDBus_Connection_Event_Type type, EDBus_Connection_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Send a message.
 *
 * @param conn the connection where the message will be sent
 * @param msg message that will be sent
 * @param cb if msg is a method call a callback should be passed
 * to be executed when a response arrives
 * @param cb_data data passed to callback
 * @param timeout timeout in milliseconds, -1 to use default internal value or
 * EDBUS_TIMEOUT_INFINITE for no timeout
 */
EAPI EDBus_Pending *edbus_connection_send(EDBus_Connection *conn, EDBus_Message *msg, EDBus_Message_Cb cb, const void *cb_data, double timeout) EINA_ARG_NONNULL(1, 2);
/**
 * @}
 */
#endif
