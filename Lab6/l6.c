#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define FILENAME "f"

// Кол-во пото
#define N_THREADS 2 //Сколько обрабатывает потоков
#define CHUNKS 4 // всего маленьких кусков
#define ROUNDS CHUNKS/N_THREADS // кол-во подходов для обработки потоком ему принадлежащего куска

//Не меньше 1

//8*8 = 64!

#define SL 1 //сколько спим перед началом работы потока

typedef struct {
    char* chunks_start;
    char* chunks_end;
    long int id;
} parse_chunk_thread_args;

static uint64_t processed[N_THREADS];
uint64_t filesize_bytes=0;
uint64_t chunk_size = 0;
static pthread_barrier_t barrier;

char* parse_chunk(char* at, char* end, int whthread){
    uint64_t val=0;
    while ((at) < (end)) {
        //if (*at =='\n') printf("~\n");
        (at)++;
       val++;
    }
    processed[whthread]=val;
    return at;
}


void* parse_chunk_thread(void *args) {

    int s;
    parse_chunk_thread_args* a = (parse_chunk_thread_args* ) args;   // original code
    for(int i = 0; i < ROUNDS; i++) {
        sleep(SL);
        printf("----------------Started thread--------------\n");

        // правильнее не чанк, а th_block!
        //идет по одному и тому же куску постоянно
        //a->chunks_start = parse_chunk((a->chunks_start), a->chunks_end,i);
        //
        a->chunks_start = parse_chunk((a->chunks_start), a->chunks_start + chunk_size, i);
        a->chunks_end = a->chunks_start + chunk_size;


        printf("!Thread %ld reached barrier %d after working, processed = %ld bs\n", a->id, i, processed[i]);
        s = pthread_barrier_wait(&barrier);

        if (s == 0) {
            printf("+Thread %ld passed barrier %d: return value was 0\n", a->id, i);
        } else if (s == PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("Thread %ld passed barrier %d with returning "
                   "PTHREAD_BARRIER_SERIAL_THREAD\n", a->id, i);
            printf("\n");

        }
    }
    sleep(2); // ждем, чтобы не перемешались сообщения
    //можно конечно было добавить семафор, ноооо
    printf("------------------Thread %ld terminating--------\n", a->id);
    return NULL;

}







int main (){

    int fd = open(FILENAME, O_RDONLY);

    if (-1==fd ){
        perror("Check file;\n");
        exit(-1);
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        return false;
    }
    uint64_t filesize_bytes = sb.st_size;
    chunk_size = filesize_bytes / CHUNKS;
    // PROT_READ не противоречит арг, с какими открыт файл, только для чтения, приватно
    char* buf_start = mmap(NULL, filesize_bytes, PROT_READ, MAP_PRIVATE, fd, 0);
    char* buf_end = buf_start + filesize_bytes;
    char* chunks [CHUNKS]; //массив указателей на начала каждлго блока для каждого потока
    for (int i=0; i<CHUNKS;i++){
        chunks[i] = buf_start + i * (chunk_size);
    }
    ///
    pthread_t threads[N_THREADS];
    pthread_barrier_init (&barrier, NULL, N_THREADS);


    parse_chunk_thread_args args[N_THREADS];
    for (int i = 0; i < N_THREADS; i++) {

        args[i].chunks_start = chunks[(i * ROUNDS)];
        if (i < N_THREADS - 1) {
            args[i].chunks_end = chunks[(i * ROUNDS) + ROUNDS];
        }
        else {
            args[i].chunks_end = buf_end;
        }
        args[i].id = i;     // new code
        pthread_create(&threads[i], NULL, parse_chunk_thread, &args[i]);
    }







    ///
    for (int i = 0; i < N_THREADS; i++) {
        //Ждем завершения всех
        pthread_join(threads[i], NULL);
    }
    // уберем отображение в память
    if (munmap(buf_start, filesize_bytes) == -1) {
        perror("Failed while: munmap\n");
        exit(-1);
    }
    





    return 0;
}