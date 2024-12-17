#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

// Define semaphore operations
#define P(mutex, sem_num) { \
    struct sembuf p_op = {sem_num, -1, 0}; \
    if (semop(mutex, &p_op, 1) == -1) { \
        perror("P operation failed"); \
        exit(1); \
    } \
}

#define V(mutex, sem_num) { \
    struct sembuf v_op = {sem_num, 1, 0}; \
    if (semop(mutex, &v_op, 1) == -1) { \
        perror("V operation failed"); \
        exit(1); \
    } \
}

int main(int argc, char *argv[]) {

    if (argc != 4) { //arguments ida male79ouloush
        fprintf(stderr, "Usage: %s <shm_id> <sem_id>\n", argv[0]);
        exit(1);
    }

    int cpt = 1;
    int Mc;

    int shm_id = atoi(argv[1]); //sion, on récupère shared buffer memory and semaphore declared
    int svide = atoi(argv[2]);
    int splein = atoi(argv[3]);
    int *shared_buffer;

    // Attach to the shared memory segment
    if ((shared_buffer = (int *)shmat(shm_id, NULL, 0)) == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    do{ //on va produire 10 foix nkherjou, te9der dirha à l'infini mais jamais 7a yekhlass lprogramme
    
        // Enter critical section te3 consommateur, tsemma P(splein)
        P(splein, 0);
        printf("CONSOMMATION TIME\n");

        sleep(1); //make consommator be slow to test any problem in semaphore

        Mc = *shared_buffer;
        printf("Le buffer consommé est: %d\n\n", Mc);
        cpt++;

        // Exit critical section, tsemme V(svide)
        V(svide, 0);
        

    } while (cpt < 10);

    
    printf("CONSOMATION FINISHED\n");
    exit(0);
}
