#include <glib.h>

static void
test_basic (void)
{
  g_assert_cmpint (1, ==, 1);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/basic/test", test_basic);

  return g_test_run ();
}
