#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define TRAM 100
pthread_t runningTh[TRAM];

pthread_mutex_t mut;

void *thread_handler1(void *arg)
{
    int tmp = *(int *)arg;
    while (1)
    {
        pthread_mutex_lock(&mut);
        printf("LOCK START THRR\n");
        printf("THREAD № %d\n ", tmp);
        printf("LOCK STOP THRR\n");
        pthread_mutex_unlock(&mut);
        sleep(5);
        
    }
    return NULL;
}

int main()
{
    char ch;
    int counter = 0;
    int erro = pthread_mutex_init(&mut, NULL);
    if (erro != 0)
    {
        perror("Error creating Mutex\n");
        return -1;
    }
    while (1)
    {
        pthread_mutex_lock(&mut);
        printf("LOCK START IN\n");
        printf(">");
        printf("LOCK STOP IN \n");
        pthread_mutex_unlock(&mut);
        scanf("%c", &ch);
        //printf("LOCK STOP IN \n");
        //pthread_mutex_unlock(&mut);
        switch (ch)
        {
        case '+':
        {
            pthread_t tid;
            if (counter < TRAM)
            {
                // указ на идент потока, аттрибуты NULL <=> не меняем, ф-я, данные для передачи
                erro = pthread_create(&tid, NULL, thread_handler1, ((void *)&counter));

                runningTh[counter++] = tid;

                if (erro != 0)
                {
                    perror("Error creating thread\n");
                    return -1;
                }//to Close is
                //erro = pthread_join(tid, NULL);
                //if (erro != 0)
                //{
                //    perror("Error th_join :\n");
                //    printf("!%d!\n", erro);
                //   return -1;
                //}
            }
            else
            {
                perror("MAx Amount of Threads\n");
            };
            break;
        };
        case '-':
        {
            if (counter > 0)
            {
                /* досрочное завершение потока */
                //pthread_cancel(runningTh[counter-1]);
                //выгрузка потока, т.к нам неинтересно статус и возвр значение
                printf ("H_NUM to Close is %d\n", counter);
                pthread_cancel(runningTh[--counter]);
                printf ("H_NUM After %d\n", counter);
            }
            else
            {
                pthread_mutex_lock(&mut);
                printf("LOCK START 0\n");
                printf("0 Threads left\n");
                printf("LOCK STOP 0\n");
                pthread_mutex_unlock(&mut);
            }
            break;
        }
        case 'q':
        {
            for (int i = 0; i < counter; i++)
            {
                //printf("THr %ld \n", runningTh[i]);
                pthread_detach(runningTh[i]);
            }
            pthread_mutex_destroy(&mut);
            exit(0);
            break;
        }
        default:
            break;
        };
    }

    return 0;
}