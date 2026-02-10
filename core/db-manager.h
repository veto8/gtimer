#ifndef GTIMER_DB_MANAGER_H
#define GTIMER_DB_MANAGER_H

#include <sqlite3.h>
#include <glib.h>

typedef struct _GTimerDBManager GTimerDBManager;

GTimerDBManager *gtimer_db_manager_new (const char *path, GError **error);
void gtimer_db_manager_free (GTimerDBManager *self);

sqlite3 *gtimer_db_manager_get_db (GTimerDBManager *self);

void gtimer_db_manager_create_task (GTimerDBManager *self, const char *name, int project_id);
void gtimer_db_manager_update_task (GTimerDBManager *self, int task_id, const char *name, int project_id);
void gtimer_db_manager_delete_task (GTimerDBManager *self, int task_id);
void gtimer_db_manager_hide_task (GTimerDBManager *self, int task_id, gboolean hidden);
GList *gtimer_db_manager_get_hidden_tasks (GTimerDBManager *self);

void gtimer_db_manager_create_project (GTimerDBManager *self, const char *name);
void gtimer_db_manager_update_project (GTimerDBManager *self, int project_id, const char *name);
GList *gtimer_db_manager_get_projects (GTimerDBManager *self);

// Task time management
gint64 gtimer_db_manager_get_task_total_time (GTimerDBManager *self, int task_id);
gint64 gtimer_db_manager_get_task_today_time (GTimerDBManager *self, int task_id);
void gtimer_db_manager_add_task_time (GTimerDBManager *self, int task_id, gint64 seconds);
void gtimer_db_manager_set_task_today_time (GTimerDBManager *self, int task_id, gint64 seconds);
void gtimer_db_manager_start_task_timing (GTimerDBManager *self, int task_id);
void gtimer_db_manager_stop_task_timing (GTimerDBManager *self, int task_id);
gboolean gtimer_db_manager_is_task_timing (GTimerDBManager *self, int task_id);

typedef struct {
  char *task_name;
  gint64 total_duration;
} GTimerReportRow;

GList *gtimer_db_manager_get_daily_report (GTimerDBManager *self, int year, int month, int day);
void gtimer_report_row_free (GTimerReportRow *row);

typedef struct {
  gint64 id;
  gint64 task_id;
  gint64 created_at;
  char *text;
} GTimerAnnotation;

GList *gtimer_db_manager_get_annotations (GTimerDBManager *self, int task_id);
void gtimer_db_manager_add_annotation (GTimerDBManager *self, int task_id, const char *text);
void gtimer_annotation_free (GTimerAnnotation *annotation);

#endif
