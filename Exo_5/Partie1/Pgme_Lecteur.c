#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define mutex 0
#define sf 1

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

    sem_id = atoi(argv[1]); // Get semaphore ID from arguments

    // Get shared memory for reader count
    key_t key = ftok(".", 'A');
    shm_id = shmget(key, sizeof(int), 0666);
    int *nl = (int *)shmat(shm_id, NULL, 0);

    // Start reading
    p(sem_id, mutex);
    (*nl)++;
    if (*nl == 1)
    {
        p(sem_id, sf); // First reader locks the file for writers
    }
    v(sem_id, mutex);

    // Section critique: Read from the file
    FILE *file = fopen("share_file.txt", "r");
    if (file == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier\n");
        exit(1);
    }

    char buffer[256];
    printf("Lecteur PID %d is reading:\n", getpid());
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        printf("Lecteur: %s", buffer);
    }
    fclose(file);

    // End reading
    p(sem_id, mutex);
    (*nl)--;
    if (*nl == 0)
    {
        v(sem_id, sf); // Last reader releases the file for writers
    }
    v(sem_id, mutex);

    shmdt(nl);

    return 0;
}
