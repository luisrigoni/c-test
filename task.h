#ifndef __TASK__
#define __TASK__

#include <ucontext.h>
#include "tasksem.h"

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define STACKSIZE   32768

#define QUANTUM		10

typedef enum Status
{
    Nova,
    Pronta,
    Rodando,
    Suspensa,
    Terminada
} Status;

typedef struct task_t
{
   unsigned int id;
   struct task_t *prev;
   struct task_t *next;
   ucontext_t context;
   Status status;
   unsigned int ticks_left;
   unsigned long tick_start;
   unsigned long tick_finish;
   unsigned long ticks_run;
   unsigned long activations;
   struct task_t *wait_queue;
   int exit_code;
   unsigned long wake_time;
} task_t;

void task_sleep (int t);

int task_join (task_t *task);

unsigned long systime();

void task_init ();

int task_create (task_t * task, void (*start_routine)(void *),  void * arg);

void task_exit (int exit_code);

int task_yield (task_t *task);

int task_id ();

#endif
