#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#define SEM_SYNCH 0
#define SEM_MUTEX 1
#define N 5

struct sembuf p_synch = {SEM_SYNCH, -1, 0}; // la primitive P sur le sémaphore nlibre
struct sembuf v_synch = {SEM_SYNCH, 1, 0};  // la primitive V sur le sémpahore nlibre
struct sembuf p_mutex = {SEM_MUTEX, -1, 0}; // la primitive P sur le sémpahore mutex
struct sembuf v_mutex = {SEM_MUTEX, 1, 0};  // la primitive V sur le sémaphore mutex

int allouer()
{
    int num = 0;
    key_t cle = ftok(".", 'a');
    int sem_id = semget(cle, 2, IPC_CREAT | 0666);
    int seg_id = shmget(cle, N * sizeof(bool), IPC_CREAT | 0666);
    char *sem_adr = (char *)shmat(seg_id, NULL, 0);

    semop(sem_id, &p_synch, 1); // P(nlibre)
    semop(sem_id, &p_mutex, 1); // P(mutex)

    while (sem_adr[num])
    {
        num++;
    }

    sem_adr[num] = true;

    semop(sem_id, &v_mutex, 1); // V(mutex)

    return num;
}

void restituer(int num)
{
    key_t cle = ftok(".", 'a');
    int sem_id = semget(cle, 2, IPC_CREAT | 0666);
    int seg_id = shmget(cle, N * sizeof(bool), IPC_CREAT | 0666);
    char *sem_adr = (char *)shmat(seg_id, NULL, 0);

    semop(sem_id, &p_mutex, 1); // P(mutex)
    sem_adr[num] = false;
    semop(sem_id, &v_mutex, 1); // V(mutex)
    semop(sem_id, &v_synch, 1); // V(nlibre)
}

int main(int argc, char const *argv[])
{
    int machine, temps;
    struct timespec tm;

    clock_gettime(CLOCK_MONOTONIC, &tm);
    srand((unsigned)(tm.tv_sec ^ tm.tv_nsec ^ (tm.tv_nsec >> 31)));

    machine = allouer();
    printf("Je suis le client de pid='%d', j'occupe la machine N°='%d'.\n", getpid(), machine);
    temps = 1 + rand() % 20;
    sleep(temps); // utiliser la machine
    printf("Le client de pid='%d' libère la machine N°='%d' après %d seconds.\n", getpid(), machine, temps);
    restituer(machine);

    return 0;
}
