#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

struct sembuf P_empty = {1, -1, 0};
struct sembuf V_full = {0, 1, 0};

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <sem_id> <shm_id>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sem_id = atoi(argv[1]);
    int shm_id = atoi(argv[2]);
    int *shared_mem = (int *)shmat(shm_id, NULL, 0);

    if (shared_mem == (void *)-1)
    {
        perror("Error attaching to shared memory");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL)); // Seed the random number generator

    for (int i = 1; i <= 10; i++)
    {
        int random_value = rand() % 1000; // Generate a random value between 0 and 99
        semop(sem_id, &P_empty, 1);
        *shared_mem = random_value;
        printf("Producer: Produced item %d\n", random_value);
        semop(sem_id, &V_full, 1);
    }

    return 0;
}