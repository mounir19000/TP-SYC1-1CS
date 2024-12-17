#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define f 0
#define sr 4

int sem_id;

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

    // Demander l'accès pour écrire
    p(sem_id, sr); // Protéger l'accès à nr
    // Incrémenter le nombre de rédacteurs
    // Effectuer la section critique de nr
    // Premier rédacteur bloque lr pour empêcher les nouveaux lecteurs
    v(sem_id, sr);

    // Demander le verrou du fichier
    p(sem_id, f); // Assurer que personne ne lit pendant l'écriture

    // Écriture dans le fichier
    FILE *file = fopen("share_file.txt", "a");
    if (file == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier");
        exit(1);
    }
    printf("Rédacteur PID %d a écrit ceci.\n", getpid());
    fprintf(file, "Rédacteur PID %d a écrit ceci.\n", getpid());

    // Ensure the data is written to the file and not just in the buffer
    fflush(file);
    fsync(fileno(file));

    fclose(file);

    // Fin de l'écriture
    v(sem_id, f);

    // Décrémenter le nombre de rédacteurs
    p(sem_id, sr);
    v(sem_id, sr);

    return 0;
}
