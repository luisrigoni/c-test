#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"

#define BUFFER_SIZE 5

task_t  prod[3],
        cons[2];

int     buffer[BUFFER_SIZE] = {0};
int     set, get;


semaphore_t     s_buffer,
                s_item,
                s_vaga;

void produtor (void * arg)
{
    while(1)
    {
        
        task_sleep (1);

        int item = random() % 100;

        task_semdown(&s_vaga);
        task_semdown(&s_buffer);

        buffer[set++%BUFFER_SIZE] = item;

        printf("%s produziu %02i\n", (char*)arg, item);

        task_semup(&s_buffer);
        task_semup(&s_item);
    }
}

void consumidor (void * arg)
{
    while(1)
    {
        task_semdown(&s_item);
        task_semdown(&s_buffer);

        int item = buffer[get++%BUFFER_SIZE];

        printf("\t\t%s consumiu %02i\n", (char*)arg, item);

        task_semup(&s_buffer);
        task_semup(&s_vaga);

        task_sleep (1);
    }
}

int main()
{
    printf ("Main INICIO\n") ;

    task_init();

    task_create (&prod[0], produtor, "p1") ;
    task_create (&prod[1], produtor, "p2") ;
    task_create (&prod[2], produtor, "p3") ;

    task_create (&cons[0], consumidor, "c1") ;
    task_create (&cons[1], consumidor, "c2") ;

    task_semcreate (&s_buffer, 1); /* 1 = Exclusão mútua. */
    task_semcreate (&s_item, 0) ; /* 0 = Itens no Buffer. */
    task_semcreate (&s_vaga, BUFFER_SIZE) ; /* BUFFER_SIZE = Vagas disponíveis. */

    printf ("Main FIM\n") ;
    task_exit (0) ;
    exit (0) ;
}
