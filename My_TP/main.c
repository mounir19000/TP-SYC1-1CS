#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define splein 0
#define svide 1

int main()
{
    key_t key;
    int sem_id, shm_id;
    int *shared_mem;

    /*----------- Semaphore Creation -----------*/
    // Generate a unique key for semaphores
    key = ftok("./main.c", 1);
    if (key == -1)
    {
        perror("Error generating semaphore key");
        exit(EXIT_FAILURE);
    }

    // Create semaphore set
    sem_id = semget(key, 2, IPC_CREAT | 0666);
    if (sem_id == -1)
    {
        perror("Error creating semaphore set");
        exit(EXIT_FAILURE);
    }

    // Initialize semaphores
    // union semun
    // {
    //     int val;
    //     struct semid_ds *buf;
    //     unsigned short *array;
    // } splein_init, svide_init;

    // splein_init.val = 0; // No items initially
    // svide_init.val = 1;  // Buffer initially empty

    int splein_init = 0;
    int svide_init = 1;

    if (semctl(sem_id, splein, SETVAL, splein_init) == -1)
    {
        perror("Error initializing splein semaphore");
        exit(EXIT_FAILURE);
    }

    if (semctl(sem_id, svide, SETVAL, svide_init) == -1)
    {
        perror("Error initializing svide semaphore");
        exit(EXIT_FAILURE);
    }

    /*----------- Shared Memory Creation -----------*/
    // Generate a unique key for shared memory
    key = ftok("./main.c", 2);
    if (key == -1)
    {
        perror("Error generating shared memory key");
        exit(EXIT_FAILURE);
    }

    // Create shared memory segment
    shm_id = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        perror("Error creating shared memory segment");
        exit(EXIT_FAILURE);
    }

    // Attach to shared memory
    shared_mem = (int *)shmat(shm_id, NULL, 0);
    if (shared_mem == (void *)-1)
    {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    /*----------- Process Creation -----------*/
    pid_t producer_pid, consumer_pid;

    // Create producer process
    producer_pid = fork();
    if (producer_pid == 0)
    {
        printf("\nStarting producer process...");
        char sem_arg[10], shm_arg[10];
        sprintf(sem_arg, "%d", sem_id);
        sprintf(shm_arg, "%d", shm_id);
        execl("./producteur", "producteur", sem_arg, shm_arg, NULL);
        perror("Error executing producer process");
        exit(EXIT_FAILURE);
    }
    else if (producer_pid < 0)
    {
        perror("Error creating producer process");
        exit(EXIT_FAILURE);
    }

    // Create consumer process
    consumer_pid = fork();
    if (consumer_pid == 0)
    {
        printf("\nStarting consumer process...");
        char sem_arg[10], shm_arg[10];
        sprintf(sem_arg, "%d", sem_id);
        sprintf(shm_arg, "%d", shm_id);
        execl("./consommateur", "consommateur", sem_arg, shm_arg, NULL);
        perror("Error executing consumer process");
        exit(EXIT_FAILURE);
    }
    else if (consumer_pid < 0)
    {
        perror("Error creating consumer process");
        exit(EXIT_FAILURE);
    }

    // Wait for child processes to finish
    while (wait(NULL) != -1)
        ;

    /*----------- Cleanup -----------*/
    if (semctl(sem_id, 0, IPC_RMID) == -1)
    {
        perror("Error removing semaphore set");
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1)
    {
        perror("Error removing shared memory segment");
    }

    if (shmdt(shared_mem) == -1)
    {
        perror("Error detaching shared memory");
    }

    printf("\nProgram finished.\n");
    return 0;
}