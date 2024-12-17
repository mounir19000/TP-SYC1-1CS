#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define CABINES 0
#define PANIERS 1

void P(int sem_id, int sem_num)
{
    struct sembuf op = {sem_num, -1, 0};
    semop(sem_id, &op, 1);
}

void V(int sem_id, int sem_num)
{
    struct sembuf op = {sem_num, 1, 0};
    semop(sem_id, &op, 1);
}

void nageur(int id, int sem_id)
{
    struct sembuf P_cabine = {CABINES, -1, 0};
    struct sembuf V_cabine = {CABINES, 1, 0};
    struct sembuf P_panier = {PANIERS, -1, 0};
    struct sembuf V_panier = {PANIERS, 1, 0};

    printf("Nageur %d: Je cherche une cabine libre...\n", id);
    P(sem_id, CABINES);
    printf("Nageur %d: J'ai trouve une cabine.\n", id);

    printf("Nageur %d: Je cherche un panier...\n", id);
    P(sem_id, PANIERS);
    printf("Nageur %d: J'ai trouve un panier. Je me change et vais nager.\n", id);

    sleep(2); // Simulation du temps de changement

    V(sem_id, CABINES);
    printf("Nageur %d: Cabine liberee. Je vais nager.\n", id);

    sleep(5); // Simulation du temps de baignade

    printf("Nageur %d: J'ai fini de nager. Je vais chercher mon panier.\n", id);
    V(sem_id, PANIERS);
    printf("Nageur %d: J'ai rendu mon panier. Je cherche une cabine pour me changer.\n", id);

    P(sem_id, CABINES);
    printf("Nageur %d: J'ai trouve une cabine pour me changer.\n", id);

    sleep(2); // Simulation du temps de changement

    V(sem_id, CABINES);
    printf("Nageur %d: J'ai libere ma cabine et quitte la piscine.\n", id);

    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <nombre_cabines> <nombre_paniers>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int nc = atoi(argv[1]); // Nombre de cabines
    int np = atoi(argv[2]); // Nombre de paniers
    int nb_nageurs = 7;     // Nombre de nageurs

    key_t key = ftok("./piscine", 1);
    if (key == -1)
    {
        perror("Erreur lors de la generation de la cle IPC");
        exit(EXIT_FAILURE);
    }

    int sem_id = semget(key, 2, IPC_CREAT | 0666);
    if (sem_id == -1)
    {
        perror("Erreur lors de la creation des semaphores");
        exit(EXIT_FAILURE);
    }

    // Initialisation des semaphores
    if (semctl(sem_id, CABINES, SETVAL, nc) == -1)
    {
        perror("Erreur lors de l'initialisation des semaphores CABINES");
        exit(EXIT_FAILURE);
    }
    if (semctl(sem_id, PANIERS, SETVAL, np) == -1)
    {
        perror("Erreur lors de l'initialisation des semaphores PANIERS");
        exit(EXIT_FAILURE);
    }

    // Creation des processus nageurs
    for (int i = 0; i < nb_nageurs; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            nageur(i + 1, sem_id);
        }
        else if (pid < 0)
        {
            perror("Erreur lors de la creation du processus");
            exit(EXIT_FAILURE);
        }
    }

    // Attendre la fin de tous les processus
    while (wait(NULL) > 0);

    // Suppression des semaphores
    if (semctl(sem_id, 0, IPC_RMID) == -1)
    {
        perror("Erreur lors de la suppression des semaphores");
        exit(EXIT_FAILURE);
    }

    printf("Tous les nageurs ont termine.\n");
    return 0;
}
