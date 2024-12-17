#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <wait.h>

union semun
{
    int val;
};

struct sembuf PMutex = {0, -1, 0};
struct sembuf VMutex = {0, 1, 0};

int main(int argc, char *argv[])
{

    printf("zebi\n");
    fflush(stdout);

    key_t key;
    int semid;
    int v1 = 0, v2 = 0;
    int N = 4;

    // Shared memory segment for cpt
    int shm_id;
    int *cpt; // Pointer to the shared memory for cpt

    // Create shared memory for cpt
    shm_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        perror("shmget failed");
        exit(1);
    }

    // Attach shared memory to the process
    cpt = (int *)shmat(shm_id, NULL, 0);
    if (cpt == (int *)-1)
    {
        perror("shmat failed");
        exit(1);
    }

    *cpt = 1; // Initialize cpt

    printf("bedya\n");
    fflush(stdout);
    key = ftok("./main.c", 'A');
    if (key == -1)
    {
        perror("ftok failed");
        exit(1);
    }

    semid = semget(key, N + 1, IPC_CREAT | 0666);
    if (semid == -1)
    {
        perror("semget failed");
        exit(1);
    }

    union semun semaph;

    printf("rand\n");
    fflush(stdout);
    semaph.val = 0;
    for (int i = 1; i < N; i++)
    {
        if (semctl(semid, i, SETVAL, semaph) == -1)
        {
            perror("semctl semaph failed");
            exit(1);
        }
    }

    semaph.val = 1;
    if (semctl(semid, N, SETVAL, semaph) == -1)
    {
        perror("semctl semaph failed");
        exit(1);
    }

    if (semctl(semid, 0, SETVAL, semaph) == -1)
    {
        perror("semctl semaph failed");
        exit(1);
    }

    printf("9bel la boucle\n");
    fflush(stdout);

    __pid_t p;
    for (int i = 0; i < N; i++)
    {
        p = fork();
        printf("forkit\n");
        fflush(stdout);
        if (p == 0)
        {
            int j;
            printf("rani el dakhel\n");
            fflush(stdout);

            // Lock the mutex before modifying cpt
            if (semop(semid, &PMutex, 1) == -1)
            {
                perror("semop failed");
                exit(1);
            }

            j = *cpt; // Get the value of cpt from shared memory
            (*cpt)++; // Increment cpt in shared memory
            v1++;
            printf("v1 value is %d, incremented by %d\n", v1, j);
            fflush(stdout);

            // Unlock the mutex after modifying cpt
            if (semop(semid, &VMutex, 1) == -1)
            {
                perror("semop failed");
                exit(1);
            }

            struct sembuf PTJ = {j, -1, 0};
            struct sembuf VTJ = {j - 1, 1, 0};

            if (semop(semid, &PTJ, 1) == -1)
            {
                perror("semop failed");
                exit(1);
            }

            v2++;
            v2++;
            printf("v2 value is %d, incremented by %d\n", v2, j);
            fflush(stdout);

            if (j > 1)
            {
                if (semop(semid, &VTJ, 1) == -1)
                {
                    perror("semop failed");
                    exit(1);
                }
            }

            exit(0);
        }
    }

    while (wait(NULL) != -1)
    {
    }

    // Detach shared memory and clean up
    if (shmdt(cpt) == -1)
    {
        perror("shmdt failed");
        exit(1);
    }

    // Remove the shared memory
    if (shmctl(shm_id, IPC_RMID, NULL) == -1)
    {
        perror("shmctl failed");
        exit(1);
    }

    for (int i = 0; i < 4; i++)
    {
        semctl(semid, i, IPC_RMID);
    }

    return 0;
}
