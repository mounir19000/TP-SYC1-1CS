#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

#define SEM_NUM 2
#define sf 1
#define mutex 0

int shm_id, sem_id;

int main()
{
    key_t key;

    // Create an IPC key
    key = ftok(".", 'A');
    if (key == -1)
    {
        perror("Erreur de création de clé");
        exit(1);
    }

    // Create semaphore set
    sem_id = semget(key, SEM_NUM, IPC_CREAT | 0666);
    if (sem_id == -1)
    {
        perror("Erreur de création de sémaphores");
        exit(1);
    }

    // Create shared memory segment
    shm_id = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        perror("Error creating shared memory");
        exit(1);
    }

    // Attach shared memory
    int *nl = (int *)shmat(shm_id, NULL, 0);
    *nl = 0; // Initialize read count to 0

    // Initialize semaphores without union
    semctl(sem_id, mutex, SETVAL, 1); // mutex semaphore value = 1
    semctl(sem_id, sf, SETVAL, 1);    // semaphore for writers value = 1

    // Number of readers and writers
    int num_readers = 5;
    int num_writers = 1;

    pid_t pid = fork();
    if (pid == 0)
    {
        // Execute the writer program, passing the semaphore ID as an argument
        char sem_id_str[10];
        snprintf(sem_id_str, sizeof(sem_id_str), "%d", sem_id);
        execlp("./Pgme_Redacteur", "./Pgme_Redacteur", sem_id_str, NULL);
        perror("Erreur lors de l'exécution du Redacteur");
        exit(1);
    }
    wait(NULL);

    // Create multiple reader processes
    for (int i = 0; i < num_readers; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Execute the reader program, passing the semaphore ID as an argument
            char sem_id_str[10];
            snprintf(sem_id_str, sizeof(sem_id_str), "%d", sem_id);
            execlp("./Pgme_Lecteur", "./Pgme_Lecteur", sem_id_str, NULL);
            perror("Erreur lors de l'exécution du Lecteur");
            exit(1);
        }
    }

    // Create multiple writer processes
    for (int i = 0; i < num_writers; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Execute the writer program, passing the semaphore ID as an argument
            char sem_id_str[10];
            snprintf(sem_id_str, sizeof(sem_id_str), "%d", sem_id);
            execlp("./Pgme_Redacteur", "./Pgme_Redacteur", sem_id_str, NULL);
            perror("Erreur lors de l'exécution du Redacteur");
            exit(1);
        }
    }

    // Wait for the child processes to finish
    for (int i = 0; i < num_readers + num_writers; i++)
    {
        wait(NULL);
    }

    // Detach and remove shared memory
    shmdt(nl);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
