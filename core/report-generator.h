#ifndef GTIMER_REPORT_GENERATOR_H
#define GTIMER_REPORT_GENERATOR_H

#include "db-manager.h"

typedef enum {
  GTIMER_REPORT_DAILY,
  GTIMER_REPORT_WEEKLY,
  GTIMER_REPORT_MONTHLY,
  GTIMER_REPORT_YEARLY
} GTimerReportType;

typedef enum {
  GTIMER_REPORT_TEXT,
  GTIMER_REPORT_HTML
} GTimerReportFormat;

char *gtimer_report_generate (GTimerDBManager *db_manager,
                              GTimerReportType type,
                              GTimerReportFormat format,
                              GDateTime *start_date,
                              GDateTime *end_date,
                              GList *task_ids,
                              int rounding_minutes);

#endif
