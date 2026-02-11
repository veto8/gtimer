#ifndef GTIMER_TIMER_SERVICE_H
#define GTIMER_TIMER_SERVICE_H

#include <glib-object.h>
#include "task-object.h"
#include "db-manager.h"
#include "idle-monitor.h"

#define GTIMER_TYPE_TIMER_SERVICE (gtimer_timer_service_get_type ())
G_DECLARE_FINAL_TYPE (GTimerTimerService, gtimer_timer_service, GTIMER, TIMER_SERVICE, GObject)

GTimerTimerService *gtimer_timer_service_new (GTimerDBManager *db_manager);

void gtimer_timer_service_start (GTimerTimerService *self, GTimerTask *task);
void gtimer_timer_service_stop  (GTimerTimerService *self);
void gtimer_timer_service_pause (GTimerTimerService *self);
void gtimer_timer_service_resume (GTimerTimerService *self);

GTimerTask *gtimer_timer_service_get_active_task (GTimerTimerService *self);
GTimerDBManager *gtimer_timer_service_get_db_manager (GTimerTimerService *self);
gint64             gtimer_timer_service_get_elapsed     (GTimerTimerService *self);
gboolean           gtimer_timer_service_is_paused      (GTimerTimerService *self);
gint64             gtimer_timer_service_get_idle_duration (GTimerTimerService *self);
void               gtimer_timer_service_remove_time     (GTimerTimerService *self, GTimerTask *task, gint64 seconds);

void               gtimer_timer_service_set_idle_monitor (GTimerTimerService *self, GTimerIdleMonitor *monitor);


#endif
