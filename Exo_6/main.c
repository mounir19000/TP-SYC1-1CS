#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

#define M 5 // Number of slots in the buffer
#define N 1  // Number of students

#define SEM_MUTEX 0  // Semaphore for mutual exclusion (mutex)
#define SEM_ITEMS 1  // Semaphore for the number of items in the buffer
#define SEM_SPACES 2 // Semaphore for available spaces in the buffer
#define SEM_DONE 3   // Semaphore for tracking the completion of an exercise by all students

int sem_id;          // Semaphore set identifier
int buffer[M];       // Circular buffer
int in = 0, out = 0; // Indices for inserting and reading exercises

// Function for P (decrementing semaphore)
void p(int sem_num)
{
    struct sembuf op = {sem_num, -1, 0};
    semop(sem_id, &op, 1);
}

// Function for V (incrementing semaphore)
void v(int sem_num)
{
    struct sembuf op = {sem_num, 1, 0};
    semop(sem_id, &op, 1);
}

// Function for the teacher to deposit exercises
void enseignant()
{
    for (int i = 0; i < 10; i++)
    {                  // 10 exercises to be deposited
        p(SEM_SPACES); // Wait for an available space in the buffer
        p(SEM_MUTEX);  // Lock the buffer for exclusive access

        buffer[in] = i; // Deposit the exercise
        printf("Enseignant a déposé l'exercice %d à la case %d\n", i, in);

        v(SEM_MUTEX); // Unlock the buffer
        v(SEM_ITEMS); // Signal that an exercise has been deposited

        // Wait for all students to process the exercise
        for (int j = 0; j < N; j++)
        {
            v(SEM_DONE); // Signal that an exercise is ready to be processed
        }

        in = (in + 1) % M; // Update the input index
    }
}

// Function for students to retrieve exercises
void etudiant(int id)
{
    while (1)
    {
        p(SEM_ITEMS); // Wait for an exercise to be available

        p(SEM_MUTEX); // Lock the buffer for exclusive access
        int exercice = buffer[out];
        printf("Etudiant %d a retiré l'exercice %d à la case %d\n", id, exercice, out);
        out = (out + 1) % M; // Update the output index
        v(SEM_MUTEX);        // Unlock the buffer

        // Wait for the teacher to deposit the exercise
        p(SEM_DONE); // Wait for this exercise to be processed

        // Signal that the student has processed this exercise
        v(SEM_DONE); // Decrement the done semaphore for this exercise
    }
}

int main()
{
    key_t key = ftok(".", 'A'); // Create IPC key

    // Create or obtain a semaphore set
    sem_id = semget(key, 4, IPC_CREAT | 0666); // 4 semaphores: mutex, items, spaces, done

    if (sem_id == -1)
    {
        perror("Erreur de création de sémaphores");
        exit(1);
    }

    // Initialize the semaphores
    semctl(sem_id, SEM_MUTEX, SETVAL, 1);  // Mutex initialized to 1
    semctl(sem_id, SEM_ITEMS, SETVAL, 0);  // No items in the buffer initially
    semctl(sem_id, SEM_SPACES, SETVAL, M); // M spaces available in the buffer
    semctl(sem_id, SEM_DONE, SETVAL, 0);   // No exercise has been processed yet

    // // Create student processes
    // for (int i = 0; i < N; i++)
    // {
    //     if (fork() == 0)
    //     {
    //         etudiant(i); // Start the student process
    //         exit(0);
    //     }
    // }

    // Teacher deposits exercises
    enseignant();

    // Wait for all student processes to finish
    for (int i = 0; i < N; i++)
    {
        wait(NULL);
    }

    // Cleanup: Remove the semaphore set
    semctl(sem_id, 0, IPC_RMID, 0); // Remove semaphores

    return 0;
}
