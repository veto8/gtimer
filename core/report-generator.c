#include "report-generator.h"
#include "timer-utils.h"
#include <stdio.h>

char *
gtimer_report_generate (GTimerDBManager *db_manager,
                        GTimerReportType type,
                        GTimerReportFormat format,
                        GDateTime *start_date,
                        GDateTime *end_date,
                        GList *task_ids,
                        int rounding_minutes)
{
  (void)type;
  (void)end_date;
  (void)task_ids;
  
  if (format == GTIMER_REPORT_TEXT) {
    GString *report = g_string_new ("GTimer Report\n");
    char *date_str = g_date_time_format (start_date, "%Y-%m-%d");
    g_string_append_printf (report, "Date: %s\n\n", date_str);
    g_free (date_str);

    int year = g_date_time_get_year (start_date);
    int month = g_date_time_get_month (start_date);
    int day = g_date_time_get_day_of_month (start_date);

    GList *rows = gtimer_db_manager_get_daily_report (db_manager, year, month, day);
    if (rows) {
      gint64 grand_total = 0;
      for (GList *l = rows; l != NULL; l = l->next) {
        GTimerReportRow *row = l->data;
        gint64 seconds = row->total_duration;
        
        if (rounding_minutes > 0) {
          gint64 round_secs = (gint64)rounding_minutes * 60;
          seconds = ((seconds + round_secs / 2) / round_secs) * round_secs;
        }

        char *duration = gtimer_utils_format_duration (seconds);
        g_string_append_printf (report, "%-10s  %s\n", duration, row->task_name);
        grand_total += seconds;
        g_free (duration);
      }
      char *total_str = gtimer_utils_format_duration (grand_total);
      g_string_append_printf (report, "\n%-10s  Total\n", total_str);
      g_free (total_str);
      g_list_free_full (rows, (GDestroyNotify)gtimer_report_row_free);
    } else {
      g_string_append (report, "No data for this period.\n");
    }

    return g_string_free (report, FALSE);
  }

  if (format == GTIMER_REPORT_HTML) {
    GString *report = g_string_new ("<!DOCTYPE html><html><head><style>");
    g_string_append (report, "body { font-family: sans-serif; margin: 2em; }");
    g_string_append (report, "table { border-collapse: collapse; width: 100%; }");
    g_string_append (report, "th, td { border-bottom: 1px solid #ddd; padding: 8px; text-align: left; }");
    g_string_append (report, "th { background-color: #f2f2f2; }");
    g_string_append (report, ".duration { font-family: monospace; }");
    g_string_append (report, "</style></head><body>");
    
    char *date_str = g_date_time_format (start_date, "%Y-%m-%d");
    g_string_append_printf (report, "<h1>GTimer Report</h1><h3>Date: %s</h3>", date_str);
    g_free (date_str);

    g_string_append (report, "<table><tr><th>Duration</th><th>Task</th></tr>");

    int year = g_date_time_get_year (start_date);
    int month = g_date_time_get_month (start_date);
    int day = g_date_time_get_day_of_month (start_date);

    GList *rows = gtimer_db_manager_get_daily_report (db_manager, year, month, day);
    if (rows) {
      gint64 grand_total = 0;
      for (GList *l = rows; l != NULL; l = l->next) {
        GTimerReportRow *row = l->data;
        gint64 seconds = row->total_duration;
        if (rounding_minutes > 0) {
          gint64 round_secs = (gint64)rounding_minutes * 60;
          seconds = ((seconds + round_secs / 2) / round_secs) * round_secs;
        }
        char *duration = gtimer_utils_format_duration (seconds);
        g_string_append_printf (report, "<tr><td class='duration'>%s</td><td>%s</td></tr>", duration, row->task_name);
        grand_total += seconds;
        g_free (duration);
      }
      char *total_str = gtimer_utils_format_duration (grand_total);
      g_string_append_printf (report, "<tr><th class='duration'>%s</th><th>Total</th></tr>", total_str);
      g_free (total_str);
      g_list_free_full (rows, (GDestroyNotify)gtimer_report_row_free);
    } else {
      g_string_append (report, "<tr><td colspan='2'>No data for this period.</td></tr>");
    }

    g_string_append (report, "</table></body></html>");
    return g_string_free (report, FALSE);
  }

  return g_strdup ("Unknown report format.");
}
