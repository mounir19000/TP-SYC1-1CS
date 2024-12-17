#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct sembuf P_full = {0, -1, 0};
struct sembuf V_empty = {1, 1, 0};

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

    for (int i = 1; i <= 10; i++)
    {
        semop(sem_id, &P_full, 1);
        printf("Consumer: Consumed item %d\n", *shared_mem);
        semop(sem_id, &V_empty, 1);
    }

    return 0;
}