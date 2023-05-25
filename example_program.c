#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>

#define EVER ;;

int main(void){
    int shmid;
    int *pid;
    shmid = shmget(9999,sizeof(int),IPC_CREAT | 0666);
    pid = (int*)shmat(shmid,0,0);
    *pid = getpid();

    for(EVER){
        printf("P1 executando...%d\n", getpid());
        sleep(1);
    }
}