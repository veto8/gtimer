#ifndef GTIMER_REPORT_WINDOW_H
#define GTIMER_REPORT_WINDOW_H

#include <adwaita.h>

#define GTIMER_TYPE_REPORT_WINDOW (gtimer_report_window_get_type ())
G_DECLARE_FINAL_TYPE (GTimerReportWindow, gtimer_report_window, GTIMER, REPORT_WINDOW, AdwWindow)

GTimerReportWindow *gtimer_report_window_new (GtkWindow *parent, const char *title, const char *report_text);

#endif
