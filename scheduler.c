#include "estruturas.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#define EVER ;;
#define TIMELIMIT 120



int main(int argc, char *argv[]) {
  stdP *rrQ;
  stdP *rtLst;
  stdP *rrProcessToRun;
  stdP *rtProcessToRun;
  stdP* stagedProcess;
  stdP* receivedProcess;
  char executableName[NAMESIZE + 2] = "./";
  struct timeval elapsedTime;
  int initSeconds,secondsNow,elapsedSeconds;
  int *pid, *pid_interpretador;
  int *internalProcessCounter;
  char programName[NAMESIZE];
  key_t shmkey_pr = 1010;
  key_t shmkey_counter = 1234;
  key_t shmkey_pid_process = 9999;
  key_t shmkey_pid_interpretador = 2000;
  int processCounterComparer = -1;


  rrQ = createLst();
  rtLst = createLst();
  int shmid_pr = shmget(shmkey_pr, sizeof(stdP)*MAXPROCESSES, IPC_CREAT | 0666);
  int shmid_counter = shmget(shmkey_counter, sizeof(int), IPC_CREAT | 0666);
  int shmid_pid_process = shmget(shmkey_pid_process, sizeof(int), IPC_CREAT | 0666);
  int shmid_pid_interpretador = shmget(shmkey_pid_interpretador, sizeof(int), IPC_CREAT | 0666);

  if (shmid_pr == -1) {
    perror("Erro ao alocar regiao de memoria compartilhada para processos\n");
    exit(1);
  }
  else if(shmid_pid_process == - 1){
    perror("Erro ao alocar regiao de memoria compartilhada para pid\n");
    exit(1);
  }
  else if(shmid_counter == -1){
    perror("Erro ao alocar regiao de memoria compartilhada para counter\n");
    exit(1);
  }
  else if(shmid_pid_interpretador == - 1){
    perror("Erro ao alocar regiao de memoria compartilhada para pid interpretador\n");
    exit(1);
  }
  else {
    receivedProcess = (stdP*)shmat(shmid_pr, 0, 0);
    internalProcessCounter = (int*)shmat(shmid_counter, 0, 0);
    pid = (int*)shmat(shmid_pid_process, 0, 0);
    pid_interpretador = (int*)shmat(shmid_pid_interpretador,0,0);
  }

  gettimeofday(&elapsedTime, NULL);
  initSeconds = (int)elapsedTime.tv_sec;
  gettimeofday(&elapsedTime, NULL);
  secondsNow = (int)elapsedTime.tv_sec;
 
  while(secondsNow-initSeconds<TIMELIMIT){
    strcpy(executableName, "./");
    gettimeofday(&elapsedTime, NULL);
    secondsNow = (int)elapsedTime.tv_sec;
    elapsedSeconds = (secondsNow-initSeconds)%60;
    // verifica se um processo novo foi enviado pelo interpretador
    while(processCounterComparer < *internalProcessCounter){
      processCounterComparer++;
      stagedProcess = receivedProcess+processCounterComparer;
      // verifica se processo é round robin para adicioná-lo na fila correta
      if(stagedProcess->i == -1){
        enqueue(&rrQ, stagedProcess->name, -1);
      }
      else{
      // verifica se o processo pode ser inserido na lista de real time
        if(checkIfValidProcess(rtLst,stagedProcess)==1){
          insertNewLst(&rtLst, stagedProcess->name, stagedProcess->i, stagedProcess->d, -1);
          printLst(rtLst);
        }
      }
    }
     
    rtProcessToRun = searchProcessToRun(rtLst, elapsedSeconds);
    // se não há um processo real time para rodar nesse momento, rodar um processo round robin
    if(rtProcessToRun == NULL){
      // se houver um processo round robin na fila de prontos
      if(rrQ != NULL){
        rrProcessToRun = rrQ;
        // se o processo ainda não foi inicializado, faz um execvp para começar a executá-lo
        if(rrProcessToRun->pid == -1){
          strcpy(programName, rrProcessToRun->name);
          strcat(executableName,programName);
          printf("\nTempo: %d\n", elapsedSeconds);
          printQueue(rrQ);
          if(fork() == 0){
            char* arguments[] = {executableName, NULL};
            execvp(executableName, arguments);
          }
          else{
            // como é round robin, esperar um segundo e para o processo
            sleep(1);
            kill(*pid, SIGSTOP);
            // pega o pid do programa teste
            rrProcessToRun->pid = *pid;
            // joga o processo pro final da fila de prontos dos processos round robin
            requeue(&rrQ);
            // printa a fila
            printQueue(rrQ);
          }
        }
        else{
          // roda o processo round robin por um segundo
          printf("\nTempo: %d\n", elapsedSeconds);
          printQueue(rrQ);
          kill(rrProcessToRun->pid, SIGCONT);
          sleep(1);
          kill(rrProcessToRun->pid, SIGSTOP);
          // joga o processo pro final da fila de prontos dos processos round robin
          requeue(&rrQ);
          // printa a fila
          printQueue(rrQ);
          
        }
      }
    }
    // se há um processo real time para rodar nesse momento:
    else{
      // se processo ainda não foi inicializado, faz um execvp para começar a executá-lo
      if(rtProcessToRun->pid == -1){
        strcpy(programName, rtProcessToRun->name);
        strcat(executableName,programName);
        printf("\nTempo: %d\n", elapsedSeconds);
        //printf("nome executavel: %s\n", executableName);
        if(fork()==0){
          char* arguments[] = {executableName, NULL};
          execvp(executableName, arguments);
        }
        else{
          for(EVER){
            // verifica se terminou o tempo de execução
            gettimeofday(&elapsedTime, NULL);
            secondsNow = (int)elapsedTime.tv_sec;
            elapsedSeconds = (secondsNow-initSeconds)%60;
            if(elapsedSeconds == (rtProcessToRun->d+rtProcessToRun->i)){
              kill(*pid,SIGSTOP);
              // pega o pid do programa teste
              rtProcessToRun->pid = *pid;
              break;
            }
          }
        }
      }
      // se processo já foi inicializado, continua ele
      else{
        printf("\nTempo: %d\n", elapsedSeconds);
        kill(rtProcessToRun->pid, SIGCONT);
        for(EVER){
          // verifica se terminou o tempo de execução
          gettimeofday(&elapsedTime, NULL);
          secondsNow = (int)elapsedTime.tv_sec;
          elapsedSeconds = (secondsNow-initSeconds)%60;
          if(elapsedSeconds == (rtProcessToRun->d+rtProcessToRun->i)){
            kill(rtProcessToRun->pid,SIGSTOP);
            break;
          }
        }
      }
    }
  }
  kill(*pid_interpretador, SIGUSR1);
  return 0;
}


