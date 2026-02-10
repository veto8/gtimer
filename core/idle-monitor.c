#include "idle-monitor.h"
#include <gio/gio.h>

struct _GTimerIdleMonitor
{
  GObject parent_instance;

  GDBusProxy *proxy;
  guint idle_watch_id;
  guint active_watch_id;
  gboolean is_idle;
};

G_DEFINE_TYPE (GTimerIdleMonitor, gtimer_idle_monitor, G_TYPE_OBJECT)

enum {
  SIGNAL_IDLE,
  SIGNAL_RESUME,
  N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0 };

static void
gtimer_idle_monitor_finalize (GObject *object)
{
  GTimerIdleMonitor *self = GTIMER_IDLE_MONITOR (object);
  gtimer_idle_monitor_stop (self);
  g_clear_object (&self->proxy);
  G_OBJECT_CLASS (gtimer_idle_monitor_parent_class)->finalize (object);
}

static void
gtimer_idle_monitor_class_init (GTimerIdleMonitorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = gtimer_idle_monitor_finalize;

  signals[SIGNAL_IDLE] =
    g_signal_new ("idle", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[SIGNAL_RESUME] =
    g_signal_new ("resume", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void
gtimer_idle_monitor_init (GTimerIdleMonitor *self)
{
  GError *error = NULL;
  // Use GNOME's Mutter IdleMonitor
  self->proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                            G_DBUS_PROXY_FLAGS_NONE,
                                            NULL,
                                            "org.gnome.Mutter.IdleMonitor",
                                            "/org/gnome/Mutter/IdleMonitor/Core",
                                            "org.gnome.Mutter.IdleMonitor",
                                            NULL,
                                            &error);
  if (error) {
    g_warning ("Failed to connect to Mutter IdleMonitor: %s", error->message);
    g_error_free (error);
  }
}

GTimerIdleMonitor *
gtimer_idle_monitor_new (void)
{
  return g_object_new (GTIMER_TYPE_IDLE_MONITOR, NULL);
}

static void
on_dbus_signal (GDBusProxy *proxy,
                const gchar *sender_name,
                const gchar *signal_name,
                GVariant *parameters,
                gpointer user_data)
{
  GTimerIdleMonitor *self = GTIMER_IDLE_MONITOR (user_data);
  (void)proxy;
  (void)sender_name;

  if (g_strcmp0 (signal_name, "WatchFired") == 0) {
    guint32 id;
    g_variant_get (parameters, "(u)", &id);
    if (id == self->idle_watch_id) {
       self->is_idle = TRUE;
       g_signal_emit (self, signals[SIGNAL_IDLE], 0);
    } else if (id == self->active_watch_id) {
       self->is_idle = FALSE;
       g_signal_emit (self, signals[SIGNAL_RESUME], 0);
    }
  }
}

void
gtimer_idle_monitor_start (GTimerIdleMonitor *self, guint timeout_seconds)
{
  g_return_if_fail (GTIMER_IS_IDLE_MONITOR (self));
  if (!self->proxy) return;

  gtimer_idle_monitor_stop (self);
  
  GError *error = NULL;
  
  // Add idle watch - fires after timeout of inactivity
  GVariant *result = g_dbus_proxy_call_sync (self->proxy,
                                            "AddIdleWatch",
                                            g_variant_new ("(t)", (guint64)timeout_seconds * 1000),
                                            G_DBUS_CALL_FLAGS_NONE,
                                            -1,
                                            NULL,
                                            &error);
  if (result) {
    g_variant_get (result, "(u)", &self->idle_watch_id);
    g_variant_unref (result);
  } else {
    g_warning ("Failed to add idle watch: %s", error->message);
    g_error_free (error);
    return;
  }
  
  // Add active watch - fires when user becomes active (idletime = 0)
  error = NULL;
  result = g_dbus_proxy_call_sync (self->proxy,
                                   "AddUserActiveWatch",
                                   NULL,
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1,
                                   NULL,
                                   &error);
  if (result) {
    g_variant_get (result, "(u)", &self->active_watch_id);
    g_variant_unref (result);
  } else {
    g_warning ("Failed to add active watch: %s", error->message);
    g_error_free (error);
  }
  
  // Connect to signals if not already connected
  static gulong handler_id = 0;
  if (handler_id == 0) {
    handler_id = g_signal_connect (self->proxy, "g-signal", G_CALLBACK (on_dbus_signal), self);
  }
}

void
gtimer_idle_monitor_stop (GTimerIdleMonitor *self)
{
  g_return_if_fail (GTIMER_IS_IDLE_MONITOR (self));
  
  if (self->idle_watch_id != 0 && self->proxy) {
    g_dbus_proxy_call_sync (self->proxy, "RemoveWatch", g_variant_new ("(u)", self->idle_watch_id), G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);
    self->idle_watch_id = 0;
  }
  
  if (self->active_watch_id != 0 && self->proxy) {
    g_dbus_proxy_call_sync (self->proxy, "RemoveWatch", g_variant_new ("(u)", self->active_watch_id), G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);
    self->active_watch_id = 0;
  }
  
  self->is_idle = FALSE;
}

gboolean
gtimer_idle_monitor_is_available (GTimerIdleMonitor *self)
{
  return self->proxy != NULL;
}
