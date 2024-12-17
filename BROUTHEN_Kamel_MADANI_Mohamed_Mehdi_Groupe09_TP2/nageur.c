#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>

// constants
#define nc 2
#define np 5

// semaphore indices
#define mutex 0
#define scab 1
#define span 2

int semaphores;

// memory
int mem_npo, mem_ndp;
int* npo;
int* ndp;

//semaphore operations
struct sembuf Pmutex = {mutex, -1, 0};
struct sembuf Vmutex = {mutex, 1, 0};
struct sembuf Pscab = {scab, -1, 0};
struct sembuf Vscab = {scab, 1, 0};
struct sembuf Pspan = {span, -1, 0};
struct sembuf Vspan = {span, 1, 0};

void demander_panier(int* nump){
    semop(semaphores, &Pmutex, 1);

    *ndp = *ndp +1;
    if(*npo == np){
        semop(semaphores, &Vmutex, 1);
        semop(semaphores, &Pspan, 1);
    }

    *ndp = *ndp - 1;
    *npo = *npo +1;

    *nump = *npo;
    semop(semaphores, &Vmutex, 1);
}

void liberer_panier(int i, int* nump){
    semop(semaphores, &Pmutex, 1);
    
    *npo = *npo - 1;
    *nump = *npo;

    if(*ndp>0){
        printf("Le nageur %d va liberer un panier et il y'a %d demandes en attente \n", i, *ndp);
        semop(semaphores, &Vspan, 1);
    }
    else{
        semop(semaphores, &Vmutex, 1);
    }
}

void demander_cabine(){
    semop(semaphores, &Pscab, 1);
}

void liberer_cabine(){
    semop(semaphores, &Vscab, 1);
}

int main(int argc, char* argv[]){
    key_t cle;
    int nump;

    // semaphore set
    union semun { int val;  struct semid_ds *buf;  unsigned short *array;
    } mutexinit, scabinit, spaninit;

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

    // Nageur
    demander_panier(&nump);

    demander_cabine();
    sleep(4);
    liberer_cabine();

    sleep(7);

    printf("Je suis le nageur num = %s, J'occupe le panier %d\n", argv[1], nump);

    demander_cabine();
    sleep(4);
    liberer_cabine();

    liberer_panier(atoi(argv[1]), &nump);
    printf("Je suis le nageur num = %s, J'ai liberer un panier, il reste %d paniers libres\n", argv[1], np-nump);

    // memory detachement
    shmdt(npo);
    if(npo == NULL){
        printf("Detachement Error!");
        exit(1);
    }

    shmdt(ndp);
    if(ndp == NULL){
        printf("Detachement Error!");
        exit(1);
    }
}