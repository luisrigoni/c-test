#ifndef __TASKLIST__
#define __TASKLIST__

#include "task.h"

void list_append (task_t **list, task_t *task);

task_t *list_remove (task_t **list, task_t *elem);

void list_print (char *name, task_t *list);

int list_size (task_t *list);

#endif
