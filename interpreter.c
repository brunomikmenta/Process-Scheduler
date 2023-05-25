#include "estruturas.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

void endHandler(int sinal){
  printf("Encerrando sessão...\n");
  exit(0);
}

int main(void) {
  FILE *exec;
  int line_counter = 1;
  key_t shmkey_pr = 1010;
  key_t shmkey_counter = 1234;
  key_t shmkey_pid_interpretador = 2000;
  stdP *processes;
  char *command = "./escalonador";
  char *arguments[] = {"./escalonador", NULL};
  char line[256];
  int *internalProcessCounter;
  int *pid_interpretador;
  int shmid_pr = shmget(shmkey_pr, sizeof(stdP) * MAXPROCESSES, IPC_CREAT | 0666);
  int shmid_counter = shmget(shmkey_counter, sizeof(int), IPC_CREAT | 0666);
  int shmid_pid_interpretador = shmget(shmkey_pid_interpretador, sizeof(int), IPC_CREAT | 0666);
  signal(SIGUSR1, endHandler);

  if (shmid_pr == -1) {
    perror("Erro ao alocar regiao de memoria compartilhada para processos\n");
    exit(1);
  }
  else if(shmid_counter == - 1){
    perror("Erro ao alocar regiao de memoria compartilhada para counter\n");
    exit(1);
  }
  else if(shmid_pid_interpretador == -1){
    perror("Erro ao alocar regiao de memoria compartilhada para pid interpretador\n");
    exit(1);
  }
   else {
    processes = (stdP *)shmat(shmid_pr, 0, 0);
    internalProcessCounter = (int*)shmat(shmid_counter, 0, 0);
    pid_interpretador = (int*)shmat(shmid_pid_interpretador,0,0);
  }

  *pid_interpretador = getpid();
  exec = fopen("exec.txt", "r");

  if (exec == NULL) {
    printf("Erro ao abrir o arquivo exec.txt!\n");
    exit(0);
  }

  if (fork() == 0) {
    execvp(command, arguments);
  }

  *internalProcessCounter = -1;
  while (fgets(line, 256, exec)) {
    (*internalProcessCounter)++;
    if (sscanf(line, " Run %s I=%d D=%d", (processes+(*internalProcessCounter))->name, &((processes+(*internalProcessCounter))->i),
               &((processes+(*internalProcessCounter))->d)) == 3) {
        (processes+(*internalProcessCounter))->pid = - 1; 

    } 
    else if (sscanf(line, " Run %s", (processes+(*internalProcessCounter))->name) == 1) {
      (processes+(*internalProcessCounter))->i = -1;
      (processes+(*internalProcessCounter))->d = -1;

    } else {
      printf("Erro de formatacao na linha %d\n", line_counter);
      (*internalProcessCounter)--;
    }
    line_counter++;
    sleep(1);
  }
  fclose(exec);
  while(1);

  return 0;
}
