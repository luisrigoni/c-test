#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#include "task.h"
#include "tasklist.h"

task_t  *curr_task,
        main_task,
        dispatcher_task;

task_t  *ready_tasks = NULL,
        *sleeping_tasks = NULL;

unsigned int    id_cont = 0,
                task_count = 0;

int    kernelMode = 0;

struct itimerval timer;
struct sigaction action;

unsigned long   clkTickCounter = 0;

void task_awake ()
{
    if(!sleeping_tasks)
        return;

    task_t *aux, *auxN;

    unsigned int lenght;
    for(aux = sleeping_tasks, lenght = list_size(sleeping_tasks); lenght > 0; lenght--)
    {
        auxN = aux->next;

        if(aux->wake_time <= systime())
        {
            list_append(&ready_tasks, list_remove(&sleeping_tasks, aux));
            aux->status = Pronta;
        }

        if(lenght > 1)
            aux = auxN;
    }
}

void task_sleep (int t)
{
    if(t <= 0)
        return;

    if(curr_task == &dispatcher_task)
        return;

    kernelMode = 1;

    curr_task->wake_time = systime() + t*1000;
    list_append(&sleeping_tasks, list_remove(&ready_tasks, curr_task));
    curr_task->status = Suspensa;

    kernelMode = 0;
    task_yield(NULL);
}

int task_join (task_t *task)
{
    if(!task)
        return -1;

    kernelMode = 1;

    if(task == curr_task || task == &dispatcher_task)
    {
        kernelMode = 0;
        return -1;
    }

    if(task->status == Terminada)
    {
        kernelMode = 0;
        return -1;
    }

    list_append(&task->wait_queue, list_remove(&ready_tasks, curr_task));
    curr_task->status = Suspensa;

    task_yield(NULL);
    kernelMode = 0;
    return task->exit_code;
}

unsigned long systime()
{
    return clkTickCounter;
}

void ticks_handler ()
{
    clkTickCounter++;
    curr_task->ticks_run++;

    if(kernelMode == 1)
        return;

    if(curr_task == &dispatcher_task)
        return;

    curr_task->ticks_left == 0 ? task_yield(NULL) : curr_task->ticks_left--;
}

task_t* scheduler_fifo ()
{
    task_t *aux;
    aux = ready_tasks;

    if(aux)
        ready_tasks = ready_tasks->next;

    return aux;
}

void dispatcher ()
{
    kernelMode = 1;

    task_t *next;

    task_count--;
    while(task_count > 0)
    {
        task_awake();

        next = scheduler_fifo();
        if(next)
        {
            next->ticks_left = QUANTUM;
            task_yield(next);
            switch(next->status)
            {
                case Terminada:
                    list_remove(&ready_tasks, next);
                    if(next->context.uc_stack.ss_sp)
                            free(next->context.uc_stack.ss_sp);
                    task_count--;
                    break;
                default:
                    break;
            }
        }
        else
            sleep(60);
    }

    #ifdef DEBUG
    printf("Dispatcher sendo encerrado.\n");
    #endif

    task_exit(0);
}

void setgearclock (int usec, void (*handler)(int))
{
    action.sa_handler = handler;
    sigemptyset (&action.sa_mask);
    action.sa_flags = 0;
    sigaction (SIGALRM, &action, 0);

    timer.it_value.tv_usec = usec;
    timer.it_value.tv_sec  = 0;
    timer.it_interval.tv_usec = usec;
    timer.it_interval.tv_sec  = 0;

    if (setitimer (ITIMER_REAL, &timer, 0) < 0)
    perror ("Erro no setitimer: ");
}

void task_init ()
{
    kernelMode = 1;

    setvbuf (stdout, 0, _IONBF, 0);

    if(getcontext(&main_task.context) != 0)
        return;

    main_task.context.uc_stack.ss_sp = NULL;
    main_task.id = 0;
    main_task.status = Rodando;
    main_task.tick_start = systime();
    main_task.activations = 0;
    main_task.ticks_run = 0;
    main_task.wait_queue = NULL;

    curr_task = &main_task;
    task_count++;

    #ifdef DEBUG
    printf("task_init: tarefa main iniciada com sucesso.\n");
    #endif

    task_create(&dispatcher_task, dispatcher, NULL);
    dispatcher_task = *list_remove(&ready_tasks, ready_tasks);

    setgearclock(1000, &ticks_handler);

    list_append(&ready_tasks, &main_task);
    task_yield(NULL);

    kernelMode = 0;
}

int task_create (task_t * task, void (*start_routine)(void *),  void * arg)
{
    if(!task)
        return -1;

    if(getcontext (&task->context) != 0)
        return -1;

    kernelMode = 1;

    task->status = Nova;

    char *stack;
    stack = malloc (STACKSIZE);
    if (stack)
    {
        task->context.uc_stack.ss_sp = stack;
        task->context.uc_stack.ss_size = STACKSIZE;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link =  0;
        task->id = ++id_cont;
        task->tick_start = systime();
        task->activations = 0;
        task->ticks_run = 0;
        task->wait_queue = NULL;
    }
    else
    {
        perror ("Erro na criação da pilha: ");
        exit (1);
    }

    makecontext (&task->context, (void*)(*start_routine), 1, arg);

    #ifdef DEBUG
    printf("task_creat: criou tarefa %i.\n", task->id);
    #endif

    list_append(&ready_tasks, task);
    task->status = Pronta;

    task_count++;

    kernelMode = 0;
    return task->id;
}

void task_exit (int exit_code)
{
    kernelMode = 1;

    #ifdef DEBUG
    printf("task_exit: tarefa %i sendo encerrada.\n", task_id());
    #endif

    while(curr_task->wait_queue)
    {
        curr_task->wait_queue->status = Pronta;
        list_append(&ready_tasks, list_remove(&curr_task->wait_queue, curr_task->wait_queue));
    }

    curr_task->exit_code = exit_code;

    curr_task->tick_finish = systime();
    curr_task->status = Terminada;

    printf("Task %02i exit: %04lims run time, %04lims CPU time, %03li activations.\n",
        task_id(), (curr_task->tick_finish - curr_task->tick_start), curr_task->ticks_run, curr_task->activations);

    task_t *next;
    if (curr_task == &dispatcher_task)
        next = &main_task;
    else
        next = &dispatcher_task;

    task_t *old;
    old = curr_task;
    curr_task = next;

    next->status = Rodando;

    swapcontext(&old->context, &next->context);

    if(curr_task == &dispatcher_task)
        kernelMode = 1;
    else
        kernelMode = 0;
}

int task_yield (task_t *task)
{
    kernelMode = 1;

    if(!task)
        task = &dispatcher_task;

    #ifdef DEBUG
    printf("task_yield: trocando contexto %i -> %i.\n", task_id(), task->id);
    #endif

    task_t *old;
    old = curr_task;
    curr_task = task;

    old->status = Pronta;
    curr_task->status = Rodando;

    curr_task->activations++;

    if(task != &dispatcher_task)
        kernelMode = 0;
    return swapcontext(&old->context, &task->context);
}

int task_id ()
{
    return curr_task->id;
}
