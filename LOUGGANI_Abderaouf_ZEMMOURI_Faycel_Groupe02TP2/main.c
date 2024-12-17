#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define SEM_SYNCH 0
#define SEM_MUTEX 1
#define N 5
#define U 10

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main(int argc, char const *argv[])
{
    key_t cle = ftok(".", 'a');
    int seg_id;
    char *seg_adr;
    int sem_id;
    union semun sem_synch_init, sem_mutex_init;

    if (cle == -1)
    {
        printf("Erreur dans la création de la clé\n");
        exit(1);
    }

    // Creation et attachement du segment de mémoire
    seg_id = shmget(cle, N * sizeof(bool), IPC_CREAT | 0666);
    seg_adr = (char *) shmat(seg_id, NULL, 0);

    // initilisation du segment (variable paratgée m_dispo)
    for (int i = 0; i < N; i++)
    {
        seg_adr[i] = false;
    }

    // Creation de l'ensemble des sémaphores
    sem_id = semget(cle, 2, IPC_CREAT | 0666);

    if (sem_id == -1)
    {
        printf("Erreur dans la creation de l'ensemble des sémaphores\n");
        exit(2);
    }

    // Initilisation du sémaphore de synchronisation à N (Nombre de machines)
    sem_synch_init.val = N;
    semctl(sem_id, SEM_SYNCH, SETVAL, sem_synch_init);

    // Initilisation du sémaphore d'exlusion mutuelle
    sem_mutex_init.val = 1;
    semctl(sem_id, SEM_MUTEX, SETVAL, sem_mutex_init);

    for (int i = 0; i < U; i++) {
        int p = fork();
        if (p == 0){
            execl("client", "client", NULL);
        }
    }

    while(wait(NULL) != -1);

    // Destruction des sémaphores
    semctl(sem_id, 2, IPC_RMID, 0);

    // Détachement du segment de mémoire partagée
    shmdt(seg_adr);

    // Supression du segment de mémoire partagée
    shmctl(seg_id, IPC_RMID, 0);

    return 0;
}
