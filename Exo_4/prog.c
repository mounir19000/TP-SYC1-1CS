#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>

#define N 5 // Capacité maximale des bacs B1 et B2

// Structure pour les opérations sémaphores
struct sembuf P = {0, -1, 0}; // P operation (wait)
struct sembuf V = {0, 1, 0};  // V operation (signal)

int sem_b1, sem_b2;             // Sémaphores pour les pièces disponibles
int sem_space_b1, sem_space_b2; // Sémaphores pour l'espace disponible
int mutex_b1, mutex_b2;         // Sémaphores mutex pour protéger les bacs

void P1()
{
    while (1)
    {
        // Vérifie s'il y a de l'espace dans le bac B1
        semop(sem_space_b1, &P, 1);

        // Ajoute une pièce A dans le bac B1
        semop(mutex_b1, &P, 1); // Verrouiller l'accès au bac B1
        printf("P1 : Produit une pièce A et la dépose dans B1.\n");
        sleep(1);               // Simulation du temps de production
        semop(mutex_b1, &V, 1); // Déverrouiller l'accès au bac B1

        // Signale qu'une pièce A est disponible dans B1
        semop(sem_b1, &V, 1);
    }
}

void P2()
{
    while (1)
    {
        // Vérifie s'il y a de l'espace dans le bac B2
        semop(sem_space_b2, &P, 1);

        // Ajoute une pièce B dans le bac B2
        semop(mutex_b2, &P, 1); // Verrouiller l'accès au bac B2
        printf("P2 : Produit une pièce B et la dépose dans B2.\n");
        sleep(1);               // Simulation du temps de production
        semop(mutex_b2, &V, 1); // Déverrouiller l'accès au bac B2

        // Signale qu'une pièce B est disponible dans B2
        semop(sem_b2, &V, 1);
    }
}

void assembler(int id)
{
    while (1)
    {
        // Retire une pièce A de B1
        semop(sem_b1, &P, 1);   // Attendre une pièce A disponible
        semop(mutex_b1, &P, 1); // Verrouiller l'accès au bac B1
        printf("P%d : Prend une pièce A de B1.\n", id);
        sleep(1);               // Simulation du temps de prise
        semop(mutex_b1, &V, 1); // Déverrouiller l'accès au bac B1

        // Libère un espace dans B1
        semop(sem_space_b1, &V, 1);

        // Retire une pièce B de B2
        semop(sem_b2, &P, 1);   // Attendre une pièce B disponible
        semop(mutex_b2, &P, 1); // Verrouiller l'accès au bac B2
        printf("P%d : Prend une pièce B de B2.\n", id);
        sleep(1);               // Simulation du temps de prise
        semop(mutex_b2, &V, 1); // Déverrouiller l'accès au bac B2

        // Libère un espace dans B2
        semop(sem_space_b2, &V, 1);

        // Assemble les pièces A et B
        printf("P%d : Assemble une pièce A et une pièce B.\n", id);
        sleep(2); // Simulation du temps d'assemblage
    }
}

int main()
{
    pid_t pid1, pid2, pid3, pid4;

    // Création des sémaphores
    sem_b1 = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    sem_b2 = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    sem_space_b1 = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    sem_space_b2 = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    mutex_b1 = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    mutex_b2 = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);

    if (sem_b1 == -1 || sem_b2 == -1 || sem_space_b1 == -1 || sem_space_b2 == -1 || mutex_b1 == -1 || mutex_b2 == -1)
    {
        perror("Erreur lors de la création des sémaphores");
        exit(1);
    }

    // Initialisation des sémaphores
    semctl(sem_b1, 0, SETVAL, 0);       // Aucun élément dans B1 au départ
    semctl(sem_b2, 0, SETVAL, 0);       // Aucun élément dans B2 au départ
    semctl(sem_space_b1, 0, SETVAL, N); // Bac B1 peut contenir N éléments
    semctl(sem_space_b2, 0, SETVAL, N); // Bac B2 peut contenir N éléments
    semctl(mutex_b1, 0, SETVAL, 1);     // Mutex initialisé à 1
    semctl(mutex_b2, 0, SETVAL, 1);     // Mutex initialisé à 1

    // Création des processus
    if ((pid1 = fork()) == 0)
    {
        P1();
        exit(0);
    }
    if ((pid2 = fork()) == 0)
    {
        P2();
        exit(0);
    }
    if ((pid3 = fork()) == 0)
    {
        assembler(3); // P3
        exit(0);
    }
    if ((pid4 = fork()) == 0)
    {
        assembler(4); // P4
        exit(0);
    }

    // Attente des processus enfants
    for (int i = 0; i < 4; i++)
    {
        wait(NULL);
    }

    // Suppression des sémaphores
    semctl(sem_b1, 0, IPC_RMID);
    semctl(sem_b2, 0, IPC_RMID);
    semctl(sem_space_b1, 0, IPC_RMID);
    semctl(sem_space_b2, 0, IPC_RMID);
    semctl(mutex_b1, 0, IPC_RMID);
    semctl(mutex_b2, 0, IPC_RMID);

    return 0;
}
