#ifndef GTIMER_IDLE_MONITOR_H
#define GTIMER_IDLE_MONITOR_H

#include <glib-object.h>

#define GTIMER_TYPE_IDLE_MONITOR (gtimer_idle_monitor_get_type ())
G_DECLARE_FINAL_TYPE (GTimerIdleMonitor, gtimer_idle_monitor, GTIMER, IDLE_MONITOR, GObject)

GTimerIdleMonitor *gtimer_idle_monitor_new (void);

void     gtimer_idle_monitor_start (GTimerIdleMonitor *self, guint timeout_seconds);
void     gtimer_idle_monitor_stop  (GTimerIdleMonitor *self);
gboolean gtimer_idle_monitor_is_available (GTimerIdleMonitor *self);

#endif
