#include <glib.h>
#include "../core/timer-utils.h"

static void
test_format_duration (void)
{
  struct {
    gint64 seconds;
    const char *expected;
  } tests[] = {
    { 0, "00:00:00" },
    { 59, "00:00:59" },
    { 60, "00:01:00" },
    { 3599, "00:59:59" },
    { 3600, "01:00:00" },
    { 3661, "01:01:01" },
    { 90061, "25:01:01" }
  };

  for (guint i = 0; i < G_N_ELEMENTS (tests); i++) {
    char *result = gtimer_utils_format_duration (tests[i].seconds);
    g_assert_cmpstr (result, ==, tests[i].expected);
    g_free (result);
  }
}

static void
test_calculate_duration (void)
{
  // 1000 start, 1100 now, 10 paused -> 90 duration
  g_assert_cmpint (gtimer_utils_calculate_current_duration (1000, 1100, 10), ==, 90);
  
  // Future start (error case)
  g_assert_cmpint (gtimer_utils_calculate_current_duration (2000, 1000, 0), ==, 0);

  // Paused more than elapsed (error case)
  g_assert_cmpint (gtimer_utils_calculate_current_duration (1000, 1100, 200), ==, 0);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/utils/format_duration", test_format_duration);
  g_test_add_func ("/utils/calculate_duration", test_calculate_duration);

  return g_test_run ();
}
