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

void etudiant(int id)
{
    while (1)
    {
        p(SEM_ITEMS); // Attendre qu'un exercice soit disponible

        p(SEM_MUTEX); // Accéder de manière exclusive au tampon
        int exercice = buffer[out];
        printf("Etudiant %d a retiré l'exercice %d à la case %d\n", id, exercice, out);
        out = (out + 1) % M; // Mise à jour de l'indice de lecture
        v(SEM_MUTEX);        // Libérer l'accès au tampon

        // Attendre que tous les étudiants aient retiré cet exercice
        p(SEM_DONE); // Attente collective sur chaque exercice

        v(SEM_DONE); // Signaler que cet exercice a été traité par un étudiant
    }
}

int main()
{
    key_t key = ftok(".", 'A');
    sem_id = semget(key, 4, 0666);

    if (sem_id == -1)
    {
        perror("Erreur de récupération des sémaphores");
        exit(1);
    }

    // Créer des processus pour chaque étudiant
    for (int i = 0; i < N; i++)
    {
        if (fork() == 0)
        {
            etudiant(i); // Lancer le programme d'un étudiant
            exit(0);
        }
    }

    // Attente de la fin de tous les étudiants (les enfants)
    for (int i = 0; i < N; i++)
    {
        wait(NULL);
    }

    return 0;
}
