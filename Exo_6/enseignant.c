#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

#define M 5 // Nombre de cases dans le tampon
#define N 3 // Nombre d'étudiants

#define SEM_MUTEX 0  // Sémaphore pour protéger l'accès au tampon
#define SEM_ITEMS 1  // Nombre d'éléments dans le tampon
#define SEM_SPACES 2 // Nombre de cases disponibles dans le tampon
#define SEM_DONE 3   // Nombre d'étudiants ayant retiré un exercice

int sem_id;          // Identifiant du jeu de sémaphores
int buffer[M];       // Tampon circulaire
int in = 0, out = 0; // Indices pour l'insertion et la lecture des exercices

// Fonction pour réaliser une opération P (decrémenter un sémaphore)
void p(int sem_num)
{
    struct sembuf op = {sem_num, -1, 0};
    semop(sem_id, &op, 1);
}

// Fonction pour réaliser une opération V (incrémenter un sémaphore)
void v(int sem_num)
{
    struct sembuf op = {sem_num, 1, 0};
    semop(sem_id, &op, 1);
}

void enseignant()
{
    for (int i = 0; i < 10; i++)
    {                  // 10 séries d'exercices
        p(SEM_SPACES); // Attendre un espace dans le tampon
        p(SEM_MUTEX);  // Accéder de manière exclusive au tampon

        buffer[in] = i; // Déposer un exercice
        printf("Enseignant a déposé l'exercice %d à la case %d\n", i, in);

        v(SEM_MUTEX); // Libérer l'accès au tampon
        v(SEM_ITEMS); // Indiquer qu'un exercice a été ajouté

        // L'enseignant attend que tous les étudiants retirent cet exercice
        v(SEM_DONE); // Signaler que l'exercice est prêt à être retiré

        in = (in + 1) % M; // Mise à jour de l'indice de lecture
    }
}

int main()
{
    key_t key = ftok(".", 'A'); // Créer une clé IPC

    // Créer ou obtenir un ensemble de sémaphores
    sem_id = semget(key, 4, IPC_CREAT | 0666); // 4 sémaphores pour `mutex`, `items`, `spaces`, `done`

    if (sem_id == -1)
    {
        perror("Erreur de création de sémaphores");
        exit(1);
    }

    // Initialisation des sémaphores
    semctl(sem_id, SEM_MUTEX, SETVAL, 1);  // Mutex initialisé à 1
    semctl(sem_id, SEM_ITEMS, SETVAL, 0);  // Pas d'éléments dans le tampon au début
    semctl(sem_id, SEM_SPACES, SETVAL, M); // M cases disponibles
    semctl(sem_id, SEM_DONE, SETVAL, 0);   // Aucun étudiant n'a retiré un exercice

    enseignant(); // L'enseignant commence à déposer les exercices

    return 0;
}
