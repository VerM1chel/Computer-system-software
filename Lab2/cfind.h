#include <stdio.h>
#include <stdlib.h>
#include <getopt.h> // getopt
#include <libgen.h> // dirname()
#include <dirent.h> // directory functions
#include <inttypes.h>

#include <stdbool.h>
#include <string.h>
#include <sys/stat.h> // file stats
#include <ctype.h>
#include <fcntl.h>

typedef struct foundEntr
{
    int type;
    char *name;
};

int ok_to_process(char *fullpath, char *cname, char *need_name, char *need_user, char *need_group, char *need_perm);

static int type_n = 0;

void ff(char *path, char *name, char *type, char *user, char *group, char *perm)
{
    struct foundEntr found;
    DIR *the_dir;
    struct dirent *dent; // Запись каталога из потока каталога из dir
    // Сод. номер ноды, текущее полож курсора в каталоге, разм возвращаемой записи, поле имени
    the_dir = opendir(path);
    char fullpath[512];

    if (the_dir) // Открыли каталог
    {
        while ((dent = readdir(the_dir))) // Читаем след эл-т каталога, пока не конец
        {
            found.name = dent->d_name;
            found.type = dent->d_type;
            sprintf(fullpath, "%s/%s", path, found.name); // Сделали полный путь
            if ((found.name[0] != '.' && found.name[1] != '.') /* && found.name[3] */)
            // Если не текущая и не предыдущая дир
            {
                if (ok_to_process(fullpath, found.name, name, user, group, perm))
                {
                    bool t = found.type == DT_DIR;
                    if (t && type_n == 2)
                    {
                        printf("Папка %s \n", fullpath);
                        break;
                    }
                    if (found.type == DT_REG && type_n == 1)
                    {
                        printf("Файл %s \n", fullpath);
                        break;
                    }
                    else if (type_n == 0)
                    {
                        if (found.type == DT_REG)
                            printf("Файл %s\n", fullpath);
                        if (found.type == DT_DIR)
                            printf("Папка %s\n", fullpath);
                    }
                }
                if (found.type == DT_DIR) // Нашли папку- делаем рекурсивно
                {
                    ff(fullpath, name, type, user, group, perm);
                }
            }
        }
        closedir(the_dir);
    }
}

int ok_to_process(char *fullpath, char *cname, char *need_name, char *need_user, char *need_group, char *need_perm)
{
    // Ф-я аргументов как И
    struct stat stbuff;
    int result = 0;
    uid_t st_uid;
    gid_t st_gid;
    if (need_user)
        st_uid = atoi(need_user);
    if (need_group)
        st_gid = atoi(need_group);
    stat(fullpath, &stbuff); // Структура с инфой об элементе

    if (need_name != NULL)
    {
        if (!strcmp(cname, need_name)) // Если совпало имя
        {
            result = 1;
            if (need_perm != NULL)
            {
                if (!((unsigned int)atoi(need_perm) == stbuff.st_mode))
                    result = 0; // Если были переданы и не совпали
            }
            if (need_user != NULL)
            {
                if (!(stbuff.st_uid == st_uid))
                    result = 0; // Если были переданы и  не совпали
            }
            if (need_group != NULL)
            {
                if (!(stbuff.st_gid == st_gid))
                    result = 0; // Если были переданы и не совпали
            }
        }
        else
            result = 0;
    }

    /* if (need_name != NULL)
    {
        if (!strcmp(cname, need_name)) // Если совпало имя
        {   
            result =1;
            if (need_perm != NULL)
            {
                if (atoi(need_perm) == stbuff.st_mode)
                    result = 1; // Если были переданы и совпали
                else
                    result = 0;
            }
            if (need_user != NULL)
            {
                if (stbuff.st_uid == st_uid)
                    result = 1; // Если были переданы и совпали
                else
                    result = 0;
            }
            if (need_group != NULL)
            {
                if (stbuff.st_gid == st_gid)
                    result = 1; // Если были переданы и совпали
                else
                    result = 0;
            }
        }
        else
            result = 0;
    } */
    return result;
}