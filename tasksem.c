#include "tasksem.h"
#include "task.h"
#include "tasklist.h"

extern task_t *curr_task;
extern task_t *ready_tasks;
extern int kernelMode;

int task_semcreate(semaphore_t *s, int value)
{
    if(!s)
        return -1;

    kernelMode = 1;

    s->counter = value;
    s->queue = NULL;
    s->validation = Valido;

    kernelMode = 0;
    return 0;
}

int task_semdown(semaphore_t *s)
{
    if(!s || s->validation == Destruido)
        return -1;

    kernelMode = 1;

    s->counter--;
    if(s->counter < 0)
    {
        curr_task->status = Suspensa;
        list_append(&s->queue, list_remove(&ready_tasks, curr_task));
        task_yield(NULL);
    }

    kernelMode = 0;

    if(s->validation == Destruido)
        return -1;

    return 0;
}

int task_semup(semaphore_t *s)
{
    if(!s)
        return -1;

    kernelMode = 1;

    s->counter++;
    if(s->counter <= 0)
    {
        s->queue->status = Pronta;
        list_append(&ready_tasks, list_remove(&s->queue, s->queue));
    }

    kernelMode = 0;

    if(s->validation == Destruido)
        return -1;

    return 0;
}

int task_semdestroy(semaphore_t *s)
{
    if(!s || s->validation == Destruido)
        return -1;

    kernelMode = 1;

    s->validation = Destruido;

    while(s->queue)
        task_semup(s);

    kernelMode = 0;
    return 0;
}
