#ifndef GTIMER_TIMER_UTILS_H
#define GTIMER_TIMER_UTILS_H

#include <glib.h>

/**
 * gtimer_utils_format_duration:
 * @seconds: Total seconds to format.
 *
 * Returns a string in HH:MM:SS format. Stateless and pure.
 * Caller must free the returned string.
 */
char *gtimer_utils_format_duration (gint64 seconds);

/**
 * gtimer_utils_calculate_current_duration:
 * @start_time: Unix timestamp of start.
 * @current_time: Unix timestamp of "now".
 * @paused_duration: Total seconds the timer was paused/idle.
 *
 * Calculates duration. Pure function.
 */
gint64 gtimer_utils_calculate_current_duration (gint64 start_time, 
                                               gint64 current_time, 
                                               gint64 paused_duration);

#endif
