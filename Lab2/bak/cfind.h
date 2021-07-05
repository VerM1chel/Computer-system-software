#include <stdio.h>
#include <stdlib.h>
#include <getopt.h> // getopt
#include <libgen.h> // header files for dirname()
#include <dirent.h> // directory functions
#include <inttypes.h>

#include <stdbool.h>
#include <string.h>   // strcat, strcmp
#include <sys/stat.h> // file stats
#include <ctype.h>
#include <fcntl.h>

typedef struct foundEntr
{
    int type;
    char *name;
    //void node;
};


int ok_to_process(char *fullpath, char *cname, char *need_name, char *need_type, uid_t *need_user, gid_t *need_group, char *need_perm, int found_type);

static int type_n = 0;
uid_t st_uid = 100;
gid_t st_gid = NULL;

void ff(char *path, char *name, char *type, uid_t *user, uid_t *group, char *perm)
{

    //puts("Func start\n");

    struct foundEntr found;
    DIR *the_dir;
    struct dirent *dent; // Запись каталога из потока каталога из dir
    // Сод. номер ноды, текущее полож курсора в каталоге, разм возвращаемой записи, поле имени
    the_dir = opendir(path);

    char p_d[2] = {'.', '.'};
    char c_d[1] = {'.'};
    char fullpath[512];

   


    

    if (the_dir) // Открыли каталог
    {
        while ((dent = readdir(the_dir))) // Читаем след эл-т каталога, пока не конец
        {
            found.name = dent->d_name;
            found.type = dent->d_type;
            sprintf(fullpath, "%s/%s", path, found.name); // Сделали полный путь
            bool a = strcmp(found.name, p_d);
            bool b = strcmp(found.name, c_d);
            if ((found.name[0] != '.' && found.name[1] != '.') && found.name[3])
            // Если не текущая и не предыдущая дир
            {
                bool aa = ok_to_process(fullpath, found.name, name, type, user, group, perm, found.type);
                if (aa)
                {
                    if (found.type == DT_DIR && type_n==2)
                    {
                        printf("Папка %s \n", found.name);
                    }
                    else if (found.type == DT_REG && type_n==1)
                    printf("Файл %s \n", found.name); 
                    //else printf("Не папка и не файл\, но подходит\n");
                }
                if (found.type == DT_DIR) // Нашли папку- делаем рекурсивно
                {
                    ff(fullpath, name, type, user, group, perm);
                }
            }
        }
        closedir(the_dir);
    }

    //puts ("F end\n");
}

int ok_to_process(char *fullpath, char *cname, char *need_name, char *need_type, uid_t* need_user, gid_t* need_group, char *need_perm, int found_type)
{ //cname - имя найденного
    // need_name - имя искомого
    // Ф-я аргументов как И
    struct stat stbuff;
    int result = 0;

    uid_t cuser;
    gid_t cgroup;

    char *cperm = NULL;

    stat(fullpath, &stbuff); // Структура с инфой об элементе

    if (need_name != NULL)
    {
        if (!strcmp(cname, need_name)) // Если совпало имя
        {
            if (need_perm != NULL)
            {
                if (atoi(need_perm) == stbuff.st_mode)
                    result = 1; // Если были переданы и совпали
                else
                    result = 0;
            }
            if (need_user != NULL)
            {
                if (stbuff.st_uid == *need_user)
                    result = 1; // Если были переданы и совпали
                else
                    result = 0;
            }
            if (need_group != NULL)
            {
                if (stbuff.st_gid == *need_group)
                    result = 1; // Если были переданы и совпали
                else
                    result = 0;
            }
        }
        else
            result = 0;
    }
    return result;
}