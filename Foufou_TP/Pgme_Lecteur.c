#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define mutex 0
#define sf 1


 int sem_id, shm_id;

void p(int sem_id, int sem_num) {
    struct sembuf operation = { sem_num, -1, 0 };
    semop(sem_id, &operation, 1);
}

void v(int sem_id, int sem_num) {
    struct sembuf operation = { sem_num, 1, 0 };
    semop(sem_id, &operation, 1);
}

int main() {
    key_t key = ftok(".", 'A');

    sem_id = semget(key, 2, 0666);
    shm_id = shmget(key, sizeof(int), 0666);

    int *nl = (int *)shmat(shm_id, NULL, 0);
//Commencer à lire
    p(sem_id,mutex);
    (*nl)++;
    if(*nl==1){
    p(sem_id, sf) ; //first reader locks the file for writers
}
   v(sem_id,mutex);


//section critique ; lire from the file 

FILE* file = fopen("share_file.txt","r");
   if(file==NULL){
   printf("Erreur lors de l'ouverture du fichier");
   exit(1);
}

  char buffer[256];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("Lecteur: %s", buffer);
    }
  fclose(file);

//end reading 
p(sem_id,mutex);
(*nl)--;
if(*nl ==0 ){
v(sem_id,sf);
}
v(sem_id, mutex);  //dernier lecteur releases the file 

shmdt(nl);

return 0;


}

