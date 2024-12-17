#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void operation_sur_A(int id)
{
    printf("Processus P%d: effectue son operation sur A.\n", id);
    sleep(1); // Simulation du travail sur A
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <nombre_processus>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[1]);
    if (N <= 0)
    {
        fprintf(stderr, "Le nombre de processus doit être un entier positif.\n");
        exit(EXIT_FAILURE);
    }

    key_t key = ftok("./precedence", 1);
    if (key == -1)
    {
        perror("Erreur lors de la generation de la cle");
        exit(EXIT_FAILURE);
    }

    int sem_id = semget(key, N, IPC_CREAT | 0666);
    if (sem_id == -1)
    {
        perror("Erreur lors de la creation des semaphores");
        exit(EXIT_FAILURE);
    }

    // Initialisation des sémaphores
    for (int i = 0; i < N; i++)
    {
        if (semctl(sem_id, i, SETVAL, i == 0 ? 1 : 0) == -1)
        {
            perror("Erreur lors de l'initialisation des semaphores");
            exit(EXIT_FAILURE);
        }
    }

    // Création des processus
    for (int i = 0; i < N; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        { // Processus enfant
            struct sembuf P = {i, -1, 0};
            struct sembuf V = {(i == N - 1) ? 0 : i + 1, 1, 0};

            semop(sem_id, &P, 1); // Attendre que P(i-1) termine
            operation_sur_A(i);

            semop(sem_id, &V, 1);


            // Finir l'execution du processus
            semop(sem_id, &P, 1);
            printf("Processus P%d: est libere et vas mourir apres liberer le suivants.\n", i);
            semop(sem_id, &V, 1);

            exit(0);
        }
        else if (pid < 0)
        {
            perror("Erreur lors de la creation d'un processus");
            exit(EXIT_FAILURE);
        }
    }

    // Attendre la fin de tous les processus
    while (wait(NULL) > 0)
        ;

    // Suppression des sémaphores
    if (semctl(sem_id, 0, IPC_RMID) == -1)
    {
        perror("Erreur lors de la suppression des semaphores");
        exit(EXIT_FAILURE);
    }

    printf("Tous les processus ont termine.\n");
    return 0;
}
