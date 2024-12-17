#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 10 //N étudiants
#define M 5 //M cases de Tampons exercices

// Define semaphore operations
// P(mutex)
#define P(sem_id, sem_num) { \
    struct sembuf p_op = {sem_num, -1, 0}; \
    if (semop(sem_id, &p_op, 1) == -1) { \
        perror("P operation failed "); \
        exit(1); \
    } \
}

//V(mutex)
#define V(sem_id, sem_num) { \
    struct sembuf v_op = {sem_num, 1, 0}; \
    if (semop(sem_id, &v_op, 1) == -1) { \
        perror("V operation failed"); \
        exit(1); \
    } \
}

int main() {

    key_t sm_key, shm_key, shmCpt_key; //keys are essentiel for semaphores and shared memories
    int shm_id, shmCpt_id, sm_id; // ids te3 mémoire partagée et ensemble de sémaphores, also essentials

    int *shared_buffer, *shared_cpt; // Pointer to shared memory Tampon de taille M, let's suppose type de case is int, cpt[] tableau partagée te3 exo besh n3erfou ch7el étudiant dare hadak exo

    // Create a unique key for the semaphores 
    if ((sm_key = ftok("/main2.c", 'A')) == -1 ) {
        perror("ftok for semaphore failed");
        exit(1);
    }

    // Create a unique key for the shared memory
    if ((shm_key = ftok("/main2.c", 'C')) == -1) {
        perror("ftok for shared memory failed");
        exit(1);
    }

    // Create a semaphore svide et splein[], khatesh fel exercices , splein est un tableau de sémaphores de taille N , 3endna tanik mutex
    // tsemma sm_id 7a yshed N+1 sémaphores , indice 0 yshed svide , 1..N yshedou splein[]
    /*EXEMPLE: 

        P(sm_id, 0) < = > P(svide)
        P(sm_id, 5) < = > P(splein[4])

        P(sm_id, N+1) < = > P(mutex)

        n7ettou mutext indice lekher kamel fel ensemble
        so id yerfed des ensembles de sémaphores , le nombre jaye fe deuxième paramètre te3 semget

    */

    if ((sm_id = semget(sm_key, N+2 , IPC_CREAT | 0666)) == -1 ) {
        perror("semget failed");
        exit(1);
    }

    // Initialize  svide=M , mutex=1
    if (semctl(sm_id, 0, SETVAL, M) == -1 || semctl(sm_id, N+1, SETVAL, 1) == -1) {
        perror("semctl failed");
        exit(1);
    }

    //Initialize splein[] kamel 0
    for (int i = 1; i <= N; i++){ // chaque itération ninitialisiw sm_id fe indice i à 0
    
        if (semctl(sm_id, i, SETVAL, 0) == -1 ) {
            perror("semctl failed");
            exit(1);
        }
    }
    //Tsemma sm_id hya ensemble des sémaphore wine indice 0 hya svide = 1 / indices 1..N splein[] init à 0 / indice N+1 mutex=1
    

    // Create a shared memory tampon et CPT de taille M
    if ((shm_id = shmget(shm_key, sizeof(int) * M, IPC_CREAT | 0666)) == -1 || (shmCpt_id = shmget(shmCpt_key, sizeof(int) * M, IPC_CREAT | 0666)) == -1 ) {
        perror("shmget failed");
        exit(1);
    }

    // Attach the shared memory segment and CPT to the process's address space
    if ((shared_buffer = (int *)shmat(shm_id, NULL, 0)) == (void *)-1 || (shared_cpt = (int *)shmat(shmCpt_id, NULL, 0)) == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    int pid ; 
    for (int i = 1; i <= N; i++){ //necreyyiw des étudiants tsemma N fils

        pid = fork();

        if (pid == -1) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) { // Child process , hada étudiant nkhelliweh yseyyi ydire des exercices (en parallèle , le père ysouti yzide yekhdem d'autres étudiants)

            //Tsemma hadi partie des processus etudiant i, ida bghitouhe ykoune fichier.c we7doukher, I showed the technique in prod/cons mail
            int j = 0; // j local pour l'étudiant
            int cpt=0; // cpt local besh n7ebssou fe 10ieme exo
            int Mc;
            
            while (cpt < 10){ //Hade étudiant i va effectuer 10 differents exos
                
                P(sm_id, i); //hadi équivalente lel P(splein[i-1]) fel algo

                P(sm_id, N+1); // ---> P(mutex)

                sleep(0.5);
                Mc = shared_buffer[j];
                printf("etudiant %d essaye exo de case %d de contenue %d\n", i, j, Mc); 
                shared_cpt[j]--; //étudiant i dare hade l'exo, tsemma cnombre restant d'étudiants li madaroush exo j houwa cpt[j] - 1

                if(shared_cpt[j] == 0){ // ida ja dernier étudiant i kemmel hade l'exo, V(svide) , onlibère cette case pour le prof père
                    V(sm_id,0);
                    printf("\nDING! case %d libéré\n\n",j);
                } 

                j = (j+1)%M ; //aller au prochain Tampon
                cpt++; //hadi cpt local berk,  no wesswass with the global one

                V(sm_id, N+1); // ---> V(mutex)


            }

        } 

        if (pid == 0) exit(0); //ida fils kemmel nkherjouhe c'est bon n7ebssouhe besh mayesrash khalouta fel for, ghire père ye93ed ydoure
        
    }
    
    
    if(pid != 0){ // Parent process, suppose this is the prof 7a yebda yekhdem les exercices

        int j = 0; // j local pour l'étudiant
        int cpt=0; // cpt local besh n7ebssou fe 10ieme exo

        while (cpt < 10){
            
            P(sm_id, 0); //----> P(svide)

            sleep(0.25);
            shared_buffer[j] = cpt;
            shared_cpt[j] = N; //cpt[j] = N
            printf("\nle prof ecrit exo sur case %d de contenue %d\n\n", j, shared_buffer[j]);

            j = (j+1)%M ; //aller au prochain Tampon
            cpt++; //hadi cpt local berk,  no wesswass with the global one

            for (int k = 1; k <= N; k++) V(sm_id, k); //signaler tous les étudiants , V(splein[k-1]) 
            


        }        


        while (wait(NULL) != -1); //Yesstenna etuds i ykemlou
        printf("Processes completed\n");

        // Cleanup: Detach and remove the shared memory segment
        if (shmdt(shared_buffer) == -1 || shmdt(shared_cpt) == -1) {
            perror("shmdt failed");
            exit(1);
        }

        if (shmctl(shm_id, IPC_RMID, NULL) == -1 || shmctl(shmCpt_id, IPC_RMID, NULL) == -1) {
            perror("shmctl IPC_RMID failed");
            exit(1);
        }

        // Cleanup: Remove the semaphore set
        if (semctl(sm_id, 0, IPC_RMID) == -1) {
            perror("semctl IPC_RMID failed");
            exit(1);
        }

    }

    return 0;
}