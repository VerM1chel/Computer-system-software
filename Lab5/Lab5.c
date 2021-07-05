#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>

#define LIB  "/home/vlad/Proj/Forgit/Lab5/my_aio.so"
#define FILENAME "res.txt"
#define SEMAPHORE_SET_ID 1000
#define SHARED_MEM_ID 2000

#define PERMS	0666
#define MAX_BUFF 250

typedef struct{
    char message [MAX_BUFF];
} what_to;

void* fReader(void* param);
void* fWriter(void*param);
int isTxt(char* str);

// 0й семафор - доступ к памяти (0 - свободно 1 - занято)
// 1й семафор - готова (1 )или не готова (0) строка из читателя
//2й семафор - управление потом писателя

int main (void){
    // создать набор семафоров и получ идентификатор
    int sem_id = semget (SEMAPHORE_SET_ID, 3, PERMS | IPC_CREAT);
    for (size_t i=0; i<3; i++)
        semctl(sem_id, i, SETVAL, 0); // освободили

    // создать кусок раздел памяти и присвоить идентификатор
    int shm_id = shmget(SHARED_MEM_ID, sizeof (what_to), PERMS | IPC_CREAT);


    pthread_t rTh;
    pthread_attr_t attr1;
    pthread_attr_init (&attr1);
    pthread_create (&rTh, &attr1, fReader, NULL);

    pthread_t wTh;
    pthread_attr_t attr2;
    pthread_attr_init (&attr2);
    pthread_create (&wTh, &attr2, fWriter, NULL);

    pthread_join(wTh, NULL);

    pthread_cancel(rTh);
    pthread_cancel(wTh);


    semctl(sem_id, 0,IPC_RMID, (struct semid_ds *) 0 );
    shmctl(shm_id, IPC_RMID, (struct shmid_ds *) 0);
    system("cat res.txt");
    return 0;
}
void* fReader(void* param){
    // получим наборы семафоров и память
    int sem_id = semget (SEMAPHORE_SET_ID, 3, 0);
    int shm_id = shmget(SHARED_MEM_ID, sizeof (what_to), 0);
    //RTLD_LAZY - разрешение всех неопределенных ссылок в коде будет
    //произведено при непосредственном обращении к загружаемым функциям
    void* so = dlopen(LIB, RTLD_LAZY);

    char* (*so_function) (char*);
    so_function = dlsym(so, "aio_Read");

    what_to* shMem;
    DIR* dir;
    char filename [30];
    struct dirent* dint = NULL;

    //добавляем сегмент раздел памяти к адресному пространству потока
    //возвращаем адрес и преобразуем его к указателю
    shMem = (what_to*)shmat(shm_id, 0, 0);
    dir = opendir("."); //Текущая
    while ((dint = readdir(dir))){
        if ((isTxt(dint->d_name) == 0) && (strcmp(dint->d_name, FILENAME) != 0)){
            //Ожидание памяти
            while (semctl(sem_id, 0, GETVAL, 0) || semctl(sem_id, 1, GETVAL, 0) );
            // пока 01 или 1й семафоры заблокированы, ждем пока не обработаем

            // Занимаем общую память
            semctl(sem_id, 0, SETVAL, 1);

            strncpy(filename, dint->d_name, 30);
            // асинхронно читаем
            char* msg = so_function (filename);

            // пишем в общ память нашу строку
            strncpy (shMem->message, msg, MAX_BUFF);
            // Устанавливаем, что нужно обработать сообщение
            semctl(sem_id, 1, SETVAL, 1);
            // Освободаем общую память
            semctl(sem_id, 0, SETVAL, 0);
            free (msg);
        }
    }
    //Закрыть writer (по выходу)
    semctl(sem_id, 2, SETVAL, 1); // как только установим, writer завершится


    //отделяем от адр пространства
    shmdt (shMem);
    closedir(dir);
    return NULL;
}

void* fWriter(void* atr){

    int sem_id = semget (SEMAPHORE_SET_ID, 3, 0);
    int shm_id = shmget(SHARED_MEM_ID, sizeof (what_to), 0);
    //RTLD_LAZY - разрешение всех неопределенных ссылок в коде будет
    //произведено при непосредственном обращении к загружаемым функциям
    void* so = dlopen(LIB, RTLD_LAZY);

    int (*so_function) (char*, char*);
    so_function = dlsym(so, "aio_Write");

    what_to* shMem;
    DIR* dir;
    char filename [30];
    struct dirent* dint = NULL;

    //добавляем сегмент раздел памяти к адресному пространству потока
    //возвращаем адрес и преобразуем его к указателю
    shMem = (what_to*)shmat(shm_id, 0, 0);
    dir = opendir("."); //Текущая

    //Удаление сушествующего файла FILENAME
    while ( dint = readdir(dir) ){
        if (strcmp(dint->d_name, FILENAME)) remove(FILENAME);
    }

    // 1 2го - запрещено
    while(!semctl(sem_id, 2, GETVAL, 0)) {
        // Если память свободна и в памяти есть сообщение
        if (!semctl(sem_id, 0, GETVAL, 0) && semctl(sem_id, 1, GETVAL, 0)) {
            // Занимаем общкю память
            semctl(sem_id, 0, SETVAL, 1);

            so_function(FILENAME, shMem->message);

            // Устанавливаем, что сообщение обработанно
            semctl(sem_id, 1, SETVAL, 0);
            // Освободаем общую память
            semctl(sem_id, 0, SETVAL, 0);
        }
    }
    //отделяем от адр пространства
    shmdt (shMem);
    return NULL;
}




int isTxt(char* str){
    int length = 0;
    while (str[length++] != '\0');
    if (str[length- 4] == 't' &&
        str[length- 3] == 'x' &&
        str[length- 2] == 't') // и на точкку проверить
        return 0;
    return -1;
}



