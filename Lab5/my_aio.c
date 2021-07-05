#include <stdio.h>
#include <stdlib.h>
#include <aio.h>
#include <dlfcn.h>

struct aiocb createIoRequest(int fd, off_t offset, volatile void * content, size_t length){
    struct aiocb controll_block = {0};

    // Дескриптор файла
    controll_block.aio_fildes = fd;
    // Смещение файла
    controll_block.aio_offset = offset;
    // Расположение буфера
    controll_block.aio_buf = content;
    // Длина передачи
    controll_block.aio_nbytes = length;
    return controll_block;
}

char* aio_Read(char* filename)
{
    FILE* file = fopen (filename, "r");
    int fd = fileno(file);
    int controll_block;
    char* buf;

    buf = (char*)calloc(200, sizeof(char));

    struct aiocb op  = createIoRequest(fd, 0, buf, 200);

    controll_block = aio_read(&op);
    if(controll_block!=0) printf("Read erro with controll_block %d\n", controll_block);

    const struct aiocb * aiolist[1];
    aiolist[0] = &op;

    //Приостанавливает поток, пока не будут завершены операции в листе
    //дождаться асинхронной операции ввода-вывода или тайм-аута
    controll_block = aio_suspend(aiolist, 1, NULL);
    if (controll_block!=0) printf("Suspend ERROR: %d\n", controll_block);

    // Возввращает статус ошибки
    controll_block = aio_error(&op);
    if (controll_block) printf("erro_aio: %d\n",controll_block);

    fclose(file);
    return buf;
}

int aio_Write(char* filename, char* msg)
{
    FILE* file = fopen(filename,"a");
    int fd = fileno(file);
    int controll_block = 0;
    int length = 0;
    while (msg[length++] != '\0');
    //Задаем статус
    struct aiocb op  = createIoRequest(fd, 0, msg, length - 1);
    // ставим в очередь
    controll_block = aio_write(&op);

    if (controll_block) printf("aio_write: %d\n", controll_block);
    const struct aiocb * aiolist[1];
    aiolist[0] = &op;

    //Приостанавлиает поток, пока не будут завершены операции указанные в листе

    controll_block = aio_suspend(aiolist,1,NULL);
    if (controll_block) printf("aio_suspend: %d\n",controll_block);

    controll_block = aio_error(&op);
    if (controll_block) printf("erro_aio: %d\n",controll_block);

    fclose(file);
    return 0;
}