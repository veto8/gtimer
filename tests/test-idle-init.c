#include <glib.h>
#include "../core/idle-monitor.h"

static void
test_idle_init (void)
{
  GTimerIdleMonitor *monitor = gtimer_idle_monitor_new ();
  g_assert_nonnull (monitor);
  
  // availability depends on if Mutter is running, but it shouldn't crash
  gboolean available = gtimer_idle_monitor_is_available (monitor);
  g_message ("Idle monitor available: %s", available ? "yes" : "no");
  
  g_object_unref (monitor);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/idle/init", test_idle_init);

  return g_test_run ();
}
