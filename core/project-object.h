#ifndef GTIMER_PROJECT_OBJECT_H
#define GTIMER_PROJECT_OBJECT_H

#include <glib-object.h>

#define GTIMER_TYPE_PROJECT (gtimer_project_get_type ())
G_DECLARE_FINAL_TYPE (GTimerProject, gtimer_project, GTIMER, PROJECT, GObject)

GTimerProject *gtimer_project_new (int id, const char *name);

int         gtimer_project_get_id   (GTimerProject *self);
const char *gtimer_project_get_name (GTimerProject *self);

#endif
