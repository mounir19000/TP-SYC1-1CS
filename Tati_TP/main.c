#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

// Define semaphore operations
// P(mutex)
#define P(mutex, sem_num) { \
    struct sembuf p_op = {sem_num, -1, 0}; \
    if (semop(mutex, &p_op, 1) == -1) { \
        perror("P operation failed "); \
        exit(1); \
    } \
}

//V(mutex)
#define V(mutex, sem_num) { \
    struct sembuf v_op = {sem_num, 1, 0}; \
    if (semop(mutex, &v_op, 1) == -1) { \
        perror("V operation failed"); \
        exit(1); \
    } \
}

int main() {

    key_t svide_key, splein_key, shm_key; //keys are essentiel for semaphores and shared memories
    int svide, splein, shm_id; //on a deux sémaphores: svide et splein , Shm pour allouer the shared memory

    int *shared_buffer; // Pointer to shared memory buffer

    // Create a unique key for the semaphores 
    if ((svide_key = ftok("/tmp", 'A')) == -1 || (splein_key = ftok("/tmp", 'B')) == -1) {
        perror("ftok for semaphore failed");
        exit(1);
    }

    // Create a unique key for the shared memory
    if ((shm_key = ftok("/tmp", 'C')) == -1) {
        perror("ftok for shared memory failed");
        exit(1);
    }

    // Create a semaphore svide et splein
    if ((svide = semget(svide_key, 1, IPC_CREAT | 0666)) == -1 || (splein = semget(splein_key, 1, IPC_CREAT | 0666)) == -1) {
        perror("semget failed");
        exit(1);
    }

    // Initialize  svide=1, splein=0
    if (semctl(svide, 0, SETVAL, 1) == -1 || semctl(splein, 0, SETVAL, 0) == -1) {
        perror("semctl failed");
        exit(1);
    }

    // Create a shared memory buffer, size of one single int, ida bghina tableau te9der tbeddel
    if ((shm_id = shmget(shm_key, sizeof(int) * 1, IPC_CREAT | 0666)) == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Attach the shared memory segment to the process's address space
    if ((shared_buffer = (int *)shmat(shm_id, NULL, 0)) == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // Initialize the shared memory buffer with zero
    *shared_buffer = 0;

    pid_t pid = fork(); //necreyyiw un fils producteur
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) { // Child process , hada producteur y3eyyet lel programme Pgme_Prod.c
        // Enter critical section
        
        char shm_id_str[16], svide_str[16], splein_str[16]; //3 strings 7a ndekhloulhoume semaphores et shared memory li creyyenahoume fel main
        snprintf(shm_id_str, sizeof(shm_id_str), "%d", shm_id); //we'll use snprintf for conversion lel string
        snprintf(svide_str, sizeof(svide_str), "%d", svide);
        snprintf(splein_str, sizeof(splein_str), "%d", splein);

        execl("./Pgme_Prod", "Pgme_Prod", shm_id_str, svide_str, splein_str, NULL); //We'll execute this program te3 ^prod en passant ces 3 shared variables
        perror("execl failed"); //ida rana hna tsemma srate erreur
        exit(1);


    } else { // Parent process

        pid = fork(); //necreyyiw un fils consomateur
        if (pid == -1) {
            perror("fork failed");
            exit(1);
        }

        if(pid == 0){ //hada fils, consomateur

            char shm_id_str[16], svide_str[16], splein_str[16]; //3 strings 7a ndekhloulhoume semaphores et shared memory li creyyenahoume fel main
            snprintf(shm_id_str, sizeof(shm_id_str), "%d", shm_id); //we'll use snprintf for conversion lel string
            snprintf(svide_str, sizeof(svide_str), "%d", svide);
            snprintf(splein_str, sizeof(splein_str), "%d", splein);


            execl("./Pgme_Consom", "Pgme_Consom", shm_id_str, svide_str, splein_str, NULL); //We'll execute this program te3 consomatteur en passant ces deux shared variables
            perror("execl failed"); //ida rana hna tsemma srate erreur
            exit(1);

        }else{ //parent

            while (wait(NULL) != -1); //Yesstenna producteurs et consommateurs ykemlou
            printf("Processes completed\n");

            // Cleanup: Detach and remove the shared memory segment
            if (shmdt(shared_buffer) == -1) {
                perror("shmdt failed");
                exit(1);
            }

            if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
                perror("shmctl IPC_RMID failed");
                exit(1);
            }

            // Cleanup: Remove the semaphore set
            if (semctl(svide, 0, IPC_RMID) == -1) {
                perror("semctl IPC_RMID failed");
                exit(1);
            }
            

        }

    }

    return 0;
}
