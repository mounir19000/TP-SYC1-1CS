
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/shm.h>

#define SEM_NUM 2
#define sf 1
#define mutex 0

int shm_id, sem_id;

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main()
{

    key_t key;

    // créer une clé IPC

    key = ftok(".", 'A');
    if (key == -1)
    {
        perror("Erreur de création de clé");
        exit(1);
    }

    sem_id = semget(key, SEM_NUM, IPC_CREAT | 0666);
    if (sem_id == -1)
    {
        perror("Erreur de création de sémaphores");
        exit(1);
    }
    // Créér un segment de mémoire partagée
    int shm_id = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        perror("Error creating shared memory");
        exit(1);
    }
    // attacher le segment de la meoire partagée
    int *nl = (int *)shmat(shm_id, NULL, 0); // shmat returns a pointer to the beginner or l'adressage mem
    // À is for default read and write permissions
    *nl = 0; // Initialize nl to 0 : nombre de lecteurs

    // Initialiser les sémaphores
    union semun sem_init;
    sem_init.val = 1;
    semctl(sem_id, mutex, SETVAL, sem_init);

    sem_init.val = 1;
    semctl(sem_id, sf, SETVAL, sem_init);
    // créér les processus lecteurs redacteurs
    pid_t pid = fork();
    if (pid == 0)
    {
        execlp("./Pgme_Lecteur", "./Pgme_Lecteur", NULL);
        perror("Erreur lors de l'exécution du Lecteur");
        exit(1);
    }

    pid = fork();
    if (pid == 0)
    {
        execlp("./Pgme_Redacteur", "./Pgme_Redacteur", NULL);
        perror("Erreur lors de l'exécution du Redacteur");
        exit(1);
    }

    wait(NULL);
    wait(NULL);

    shmdt(nl); // detach l'espace memoire
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
