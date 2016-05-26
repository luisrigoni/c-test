#ifndef __TASKSEM__
#define __TASKSEM__

#include "task.h"

typedef enum SemValidation
{
    Valido,
    Destruido
} SemValidation;

typedef struct semaphore_t
{
    int counter;
    struct task_t *queue;
    SemValidation validation;
} semaphore_t;

int task_semcreate(semaphore_t *s, int value);

int task_semdown(semaphore_t *s);

int task_semup(semaphore_t *s);

int task_semdestroy(semaphore_t *s);

#endif
