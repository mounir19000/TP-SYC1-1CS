#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define sf 1

void p(int sem_id, int sem_num) {
    struct sembuf operation = { sem_num, -1, 0 };
    semop(sem_id, &operation, 1);
}

void v(int sem_id, int sem_num) {
    struct sembuf operation = { sem_num, 1, 0 };
    semop(sem_id, &operation, 1);
} 


int main(){
int sem_id;
    key_t key = ftok(".", 'A');

    sem_id = semget(key, 2, 0666);

    // Start writing
    p(sem_id, sf);

    // Section critiaue , ecrire dans le fichier 
    FILE *file = fopen("share_file.txt", "a");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier ");
        exit(1);
    }
    fprintf(file, "Writer wrote something!\n");
    fclose(file);

    //fin d'ecriture 
    v(sem_id, sf);

    return 0;

}
