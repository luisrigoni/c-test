#include<stdio.h>

#include "tasklist.h"

void list_append (task_t **list, task_t *task)
{
    if(!task)
    {
        printf("Erro. Tarefa inconsistente.\n");
        return;
    }

    if(task->next || task->prev)
    {
        printf("Erro. Tarefa já iniciada.\n");
        return;
    }

    task->next = task->prev = task;

    if(! *list)
        (*list) = task;

    task_t *last;
    last = (*list)->prev;

    task->prev = last;
    last->next = task;
    task->next = (*list);
    (*list)->prev = task;
}

task_t *list_remove (task_t **list, task_t *elem)
{
    if(!elem)
    {
        printf("Erro. Tarefa inconsistente.\n");
        return NULL;
    }

    if(! *list)
    {
        printf("Erro. Lista inconsistente.\n");
        return NULL;
    }

    task_t *aux;
    for(aux = *list; aux != elem; aux = aux->next)
    {
        if(aux == (*list)->prev)
        {
            printf("Erro. Tarefa não encontrada.\n");
            return NULL;
        }
    }

    if(aux->next == aux)
    {
        (*list) = NULL;
        aux->next = aux->prev = NULL;
        return aux;
    }
    if(aux == (*list))
        (*list) = aux->next;

    task_t *before, *after;
    before = aux->prev;
    after = aux->next;
    before->next = after;
    after->prev = before;

    aux->next = aux->prev = NULL;

    return aux;
}

void list_print (char *name, task_t *list)
{
    task_t *aux;
    aux = list;

    printf("%s: [ ", name);
    do
    {
        if(!aux)
            break;
        printf("%i<%i>%i ", aux->prev->id, aux->id, aux->next->id);
        aux = aux->next;
    } while(aux != list);
    printf("]\n");
}

int list_size (task_t *list)
{
    if(!list)
        return 0;

    int size;
    task_t *aux;

    for(size = 1, aux = list; aux != list->prev; aux = aux->next, size++);

    return size;
}
