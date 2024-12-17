#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

#define SEM_NUM 6 // Total semaphores: f, lr, lec, mutex, sr
#define f 0       // Semaphore for file access
#define lr 1      // Semaphore for blocking readers when a writer is writing
#define lec 2     // Semaphore for controlling access of readers
#define mutex 3   // Semaphore to access nl (number of readers)
#define sr 4      // Semaphore to access nr (number of writers)
// Nombre de lecteurs et rédacteurs pour les testes
#define Nb_Lecteurs 3
#define Nb_Redacteurs 2

int shm_id, sem_id;

int main()
{
    key_t key;

    // Création de la clé IPC
    key = ftok(".", 'A');
    if (key == -1)
    {
        perror("Erreur de création de clé");
        exit(1);
    }

    // Création du set de sémaphores
    sem_id = semget(key, SEM_NUM, IPC_CREAT | 0666);
    if (sem_id == -1)
    {
        perror("Erreur de création de sémaphores");
        exit(1);
    }

    // Création de la mémoire partagée pour le nombre de lecteurs et rédacteurs
    shm_id = shmget(key, sizeof(int) * 2, IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        perror("Erreur de création de la mémoire partagée");
        exit(1);
    }

    // Attacher la mémoire partagée
    int *nl = (int *)shmat(shm_id, NULL, 0);
    int *nr = nl + 1; // `nr` is right after `nl`

    *nl = 0; // Nombre de lecteurs initialisé à 0
    *nr = 0; // Nombre de rédacteurs initialisé à 0

    // Initialiser les sémaphores
    semctl(sem_id, f, SETVAL, 1);     // Accès au fichier
    semctl(sem_id, lr, SETVAL, 1);    // Bloquer les lecteurs si un rédacteur est présent
    semctl(sem_id, lec, SETVAL, 1);   // Permettre à un seul lecteur à la fois
    semctl(sem_id, mutex, SETVAL, 1); // Protéger l'accès à nl
    semctl(sem_id, sr, SETVAL, 1);    // Protéger l'accès à nr

    // Créer plusieurs processus lecteurs et rédacteurs
    for (int i = 0; i < Nb_Lecteurs; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Exécuter le programme du lecteur
            char sem_id_str[10];
            snprintf(sem_id_str, sizeof(sem_id_str), "%d", sem_id);
            execlp("./Pgme_Lecteur", "./Pgme_Lecteur", sem_id_str, NULL);
            perror("Erreur lors de l'exécution du Lecteur");
            exit(1);
        }
    }

    for (int i = 0; i < Nb_Redacteurs; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Exécuter le programme du rédacteur
            char sem_id_str[10];
            snprintf(sem_id_str, sizeof(sem_id_str), "%d", sem_id);
            execlp("./Pgme_Redacteur", "./Pgme_Redacteur", sem_id_str, NULL);
            perror("Erreur lors de l'exécution du Rédacteur");
            exit(1);
        }
    }

    // Attendre la fin des processus enfants
    for (int i = 0; i < 5; i++)
    {
        wait(NULL);
    }

    // Détacher et supprimer la mémoire partagée
    shmdt(nl);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
