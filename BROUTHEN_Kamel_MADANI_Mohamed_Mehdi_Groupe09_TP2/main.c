#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/wait.h>

// constants
#define nc 2
#define np 5

// semaphore indices
#define mutex 0
#define scab 1
#define span 2

int semaphores;

int main(int argc, char *argv[])
{
    // semaphore set
    union semun { int val;  struct semid_ds *buf;  unsigned short *array;
    } mutexinit, scabinit, spaninit;

    int err;
    key_t cle;

    char buffer[2];

    // memory
    int mem_npo, mem_ndp;
    int* npo;
    int* ndp;

    // semaphore
    cle = ftok("main.c", 1);
    if(cle == -1){
        printf("Key Error!");
        exit(1);
    }
    semaphores = semget(cle, 3, IPC_CREAT|0666);
    if(semaphores == -1){
        printf("Semaphore Error!");
        exit(1);
    }

    // semaphore initialization
    mutexinit.val = 1;
    semctl(semaphores, mutex, SETVAL, mutexinit);
    scabinit.val = nc;
    semctl(semaphores, scab, SETVAL, scabinit);
    spaninit.val = 0;
    semctl(semaphores, span, SETVAL, spaninit);

    /* Shared memory */
    // npo
    cle = ftok("main.c", 2);
    if(cle == -1){
        printf("Key Error!");
        exit(1);
    }
    mem_npo = shmget(cle, 1*sizeof(int), IPC_CREAT|0666);
    if(mem_npo == -1){
        printf("Memory Error!");
        exit(1);
    }
    npo = (int*)shmat(mem_npo, 0, 0);
    if(npo == NULL){
        printf("Attachement Error!");
        exit(1);
    }
    *npo = 0;
    shmdt(npo);
    if(npo == NULL){
        printf("Dettachement Error!");
        exit(1);
    }

    //ndp
    cle = ftok("main.c", 3);
    if(cle == -1){
        printf("Key Error!");
        exit(1);
    }
    mem_ndp = shmget(cle, 1*sizeof(int), IPC_CREAT|0666);
    if(mem_ndp == -1){
        printf("Memory Error!");
        exit(1);
    }
    ndp = (int*)shmat(mem_ndp, 0, 0);
    if(ndp == NULL){
        printf("Attachement Error!");
        exit(1);
    }
    *ndp = 0;
    shmdt(ndp);
    if(ndp == NULL){
        printf("Dettachement Error!");
        exit(1);
    }

    /*Processes*/
    int p;
    for(int i=1; i<=10; i++){
        p = fork();
        if(p==0){
            sprintf(buffer, "%d", i);
            err = execlp("./nageur", "nageur", buffer, (char *)NULL);
            printf("Execution Error = %d \n", err>>8);
            exit(1);
        }
    }

    // wait children
    int status;
    while(wait(&status) != -1){}


    /* Destruction */
    shmctl(mem_ndp, IPC_RMID ,0);
    shmctl(mem_npo, IPC_RMID ,0);
    semctl(semaphores, 3, IPC_RMID, 0);

    return 0;
}