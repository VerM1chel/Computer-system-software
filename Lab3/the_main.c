#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <termios.h>
#include <fcntl.h>

#include <time.h>
#include <stdbool.h>

#define CFILENAME "le_file.txt"
#define PFILENAME "parent.txt"

bool check_keyboard()
{
  struct termios oldt, newt;
  int ch;
  int oldf;
  // установили терминал на неблокирующий ввод
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  //возвращаем значения флагов состояния
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  // и делаем их еще и неблокирующим
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
  ///
  ch = getchar();

  // возвращаем на место
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
  if (ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
  else
    return 0;
}

void out_time(int signal)
{
  time_t current_time = time(NULL);
  struct tm *tm = localtime(&current_time);
  printf("%s\n", asctime(tm));
}

void exit_child(int signal)
{
  printf("Ending the Child Process by SIGNAL : %d\n", signal);
  remove(CFILENAME);
  exit(0);
}

int main()
{
  pid_t mainPid = getpid();
  pid_t childPid = fork();
  FILE *file = fopen(PFILENAME, "ab+");

  char ch;

  switch (childPid)
  {
  default:
  {
    while (1)
    {
      char temp;
      signal(SIGUSR1, out_time); // переопр сигнал на свой обработчик
      ch = rand() % 31 + 'a';
      fwrite(&ch, 1, 1, file);
      if (check_keyboard())
      {
        kill(childPid, SIGQUIT);
        if ((temp = getchar()) == ' ')
        {
          fclose(file);
          exit(0);
        }
      }
    }
  }

  case 0:
  {
    //child
    fopen(CFILENAME, "ab+");
    signal(SIGQUIT, exit_child);
    while (1)
    {
      sleep(3);
      printf("Sending SIGUSR to PARENT (Need to be Handheld)\n");
      kill(mainPid, SIGUSR1);
    }
  }

  case -1:
  {
    printf("ERROR RUNNING FORK\n");
  }
  }
  return 0;
}