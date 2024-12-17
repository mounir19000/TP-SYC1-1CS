#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define mutex 3
#define lr 1
#define lec 2
#define f 0

int sem_id, shm_id;

void p(int sem_id, int sem_num)
{
    struct sembuf operation = {sem_num, -1, 0};
    semop(sem_id, &operation, 1);
}

void v(int sem_id, int sem_num)
{
    struct sembuf operation = {sem_num, 1, 0};
    semop(sem_id, &operation, 1);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <sem_id>\n", argv[0]);
        exit(1);
    }

    sem_id = atoi(argv[1]);

    // Attacher la mémoire partagée pour le compteur de lecteurs
    key_t key = ftok(".", 'A');
    shm_id = shmget(key, sizeof(int) * 2, 0666);
    int *nl = (int *)shmat(shm_id, NULL, 0);

    // Demander l'accès pour lire
    p(sem_id, lec); // Assurer qu'un seul lecteur peut entrer
    p(sem_id, lr);  // Bloquer si un rédacteur est en cours

    p(sem_id, mutex); // Protection de nl
    (*nl)++;
    if (*nl == 1)
    {
        p(sem_id, f); // Premier lecteur bloque l'accès au fichier pour les rédacteurs
    }
    v(sem_id, mutex);

    v(sem_id, lr);  // Libérer le verrou pour un rédacteur en attente
    v(sem_id, lec); // Libérer le sémaphore de lecture pour les autres lecteurs

    // Lecture du fichier
    FILE *file = fopen("share_file.txt", "r");
    if (file == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier");
        exit(1);
    }
    printf("Lecteur PID %d est en train de lire:\n", getpid());
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        printf("Lecteur: %s", buffer);
    }
    fclose(file);

    // Fin de la lecture
    p(sem_id, mutex);
    (*nl)--;
    if (*nl == 0)
    {
        v(sem_id, f); // Dernier lecteur libère l'accès au fichier
    }
    v(sem_id, mutex);

    shmdt(nl);

    return 0;
}
