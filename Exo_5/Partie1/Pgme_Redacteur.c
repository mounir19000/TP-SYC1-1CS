#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define sf 1

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

    sem_id = atoi(argv[1]); // Get semaphore ID from arguments

    // Start writing
    p(sem_id, sf);

    // Section critique: Write to the file
    FILE *file = fopen("share_file.txt", "a");
    if (file == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier");
        exit(1);
    }
    printf("Redacteur PID %d is writing:\n", getpid());
    fprintf(file, "Writer PID %d wrote something!\n", getpid());
    fclose(file);

    // End writing
    v(sem_id, sf);

    return 0;
}
