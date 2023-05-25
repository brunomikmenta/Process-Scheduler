#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>

#define NAMESIZE 32
#define MAXPROCESSES 10

typedef struct stdProcess stdP;
struct stdProcess {
  char name[NAMESIZE];
  int i, d, pid;
  stdP *next;
};

stdP *createLst() { return NULL; }

// insere ordenado pelo tempo que começa a execução
void insertNewLst(stdP **rtLst, char *name, int i, int d, int pid) {
  stdP *newElement = (stdP *)malloc(sizeof(stdP));
  strcpy(newElement->name, name);
  newElement->i = i;
  newElement->d = d;
  newElement->pid = pid;

  // se a lista estiver vazia ou se o processo novo tem o menor tempo de inicio
  // da exec
  if (*rtLst == NULL || newElement->i < (*rtLst)->i) {
    newElement->next = *rtLst;
    *rtLst = newElement;
    return;
  }

  stdP *prev = *rtLst, *after = (*rtLst)->next;
  while (after != NULL) {
    if (newElement->i < after->i) {
      break;
    }
    prev = after;
    after = after->next;
  }
  prev->next = newElement;
  newElement->next = after;
  return;
}

void removeFromLst(stdP **rtLst) {
  if (*rtLst == NULL) {
    return;
  }
  if ((*rtLst)->next == NULL) {
    free(*rtLst);
    *rtLst = NULL;
    return;
  }
  stdP *temp;
  temp = *rtLst;
  *rtLst = (*rtLst)->next;
  free(temp);
  return;
}

void printLst(stdP *rtLst) {
  printf("\nProcessos na lista de prontos real time:\n");
  while (rtLst != NULL) {
    printf("%s, ", rtLst->name);
    rtLst = rtLst->next;
  }
  printf("FIM DA LISTA RT\n");
  printf("-----------------------------------------\n");
}

stdP *searchProcessToRun(stdP *rtLst, int seconds) {
  while (rtLst != NULL) {
    if ((rtLst)->i == seconds) {
      return rtLst;
    }
    rtLst = rtLst->next;
  }
  return NULL;
}

int checkIfValidProcess(stdP *rtLst, stdP *Process) {
  int finishTimeProcess = Process->i + Process->d;
  if(finishTimeProcess>60){
    return 0;
  }
  int finishTimeCurrent;
  while (rtLst != NULL) {
    finishTimeCurrent = rtLst->i + rtLst->d;
    if (Process->i > rtLst->i && Process->i < finishTimeCurrent) {
      return 0;
    }
    if (finishTimeProcess > rtLst->i &&
        finishTimeProcess < finishTimeCurrent) {
      return 0;
    }
    rtLst = rtLst->next;
  }

  return 1;
}

void enqueue(stdP **rrq, char *name, int pid) {

  stdP *newElement = (stdP *)malloc(sizeof(stdP));
  if (newElement == NULL) {
    printf("Failed to allocate memory\n");
    exit(1);
  }
  // newElement->name = (char *)malloc(NAMESIZE * sizeof(char));
  strcpy(newElement->name, name);
  newElement->pid = pid;
  newElement->i = -1;
  newElement->d = -1;
  newElement->next = NULL;
  // fila vazia
  if (*rrq == NULL) {
    *rrq = newElement;
    return;
  }
  stdP *prev = *rrq;
  while (prev->next != NULL) {
    prev = prev->next;
  }
  prev->next = newElement;
  return;
}

void dequeue(stdP **rrq) {
  if (*rrq == NULL) {
    return;
  }
  if ((*rrq)->next == NULL) {
    free(*rrq);
    *rrq = NULL;
    return;
  }
  stdP *temp;
  temp = *rrq;
  *rrq = (*rrq)->next;
  free(temp);
  return;
}

// tira do início da fila e coloca no final
void requeue(stdP **rrq) {
  enqueue(rrq, (*rrq)->name, (*rrq)->pid);
  dequeue(rrq);
}

void printQueue(stdP *rrq) {
  printf("\nProcessos na lista de prontos round robin:\n");
  while (rrq != NULL) {
    printf("%s -> ", rrq->name);
    rrq = rrq->next;
  }
  printf("FIM DA FILA RR\n");
  printf("-----------------------------------------\n");
  return;
}
