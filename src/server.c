/**
 *      _____ ___ _____ ___ ___ ___ ___
 *     |_   _| __|_   _| _ \_ _|_ _/ __|
 *       | | | _|  | | |   /| | | |\__ \
 *       |_| |___| |_| |_|_\___|___|___/
 *
 * Project : Tetriis
 * File name : server.c
 * Description :  
 *          Main file for the server.
 *          Allows clients to connect and handle the game logic
 * Authors : DUTHOIT Thomas / GREBERT Cloé
 */

#include "server.h" 

// SEMAPHORE
sem_t * SEM_INFO_SERVEUR;
sem_t * SEM_SCORE; 

//SHM
int SHM_INFO_SERVEUR;
int SHM_SCORE;

//MUTEX
pthread_mutex_t MUT_TRY_CONNEXION = PTHREAD_MUTEX_INITIALIZER; 

//CONDITION 
pthread_cond_t COND_TRY_CONNEXION = PTHREAD_COND_INITIALIZER;

//BAL
int id_bal; 

void * thread_connexion(void * arg); 


int main(){

    #pragma region INIT
    // --------------------------------------- INIT --------------------------------------- //
    pthread_t TH_CONNEXION; 
    pthread_t TH_JEU;  

    //Initialisation des sémaphores
    SEM_INFO_SERVEUR = sem_open(CONST_SEM_NOM_INFO_SERVEUR,  O_CREAT, 0666, 0); 
    SEM_SCORE = sem_open(CONST_SEM_NOM_SCORE, O_CREAT, 0666, 1); 

    // CREATION DES FICHIERS POUR LES SHM SI NECESSAIRE
    creation_fichiers_necessaires();

    //Initialisation des SHM
    char chemin[500];  // pour concaténer les éléments du chemin
    //SHM INFO SERVEUR
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_INFO_SERVEUR);
    key_t tok_INFO_SERVEUR = ftok(chemin, CONST_PROJECT_ID); 
    SHM_INFO_SERVEUR = shmget(tok_INFO_SERVEUR, sizeof(info_serveur_t), 0666 | IPC_CREAT); 
    CHECK(SHM_INFO_SERVEUR, "DEBUG ] SERVEUR ] Erreur shmget SHM_INFO_SERVEUR"); 

    //SHM SCORE
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_SCORE);
    key_t tok_SCORE = ftok(chemin, CONST_PROJECT_ID); 
    SHM_SCORE = shmget(tok_SCORE, sizeof(score_t), 0666 | IPC_CREAT); 
    CHECK(SHM_SCORE, "DEBUG ] SERVEUR ] Erreur shmget SHM_SCORE");
    
    //Boite aux lettres
    int key = ftok(CONST_FIC_BAL, CONST_PROJECT_ID);
    CHECK(key, "erreur ftok BAL");

    id_bal = msgget(key, IPC_CREAT | 0666);
    CHECK(id_bal, "erreur msgget BAL");

    //AFFICHAGE
    affichage_serveur(); 
    // ------------------------------------------------------------------------------------ //
    #pragma endregion
    

    #pragma region WRITE_INFO_SERVEUR
    // --------------------------------------- WRITE_INFO_SERVEUR --------------------------------------- //
    //Ecriture dans SHM_INFO_SERVEUR
    info_serveur_t * info_serveur = shmat(SHM_INFO_SERVEUR, NULL, 0); 
    info_serveur->pid_serveur = getpid(); 
    info_serveur->etat_serveur = ATTENTE; 
    shmdt(info_serveur); 
    sem_post(SEM_INFO_SERVEUR);  

    // -------------------------------------------------------------------------------------------------- //
    #pragma endregion

    //Création des threads
    pthread_create(&TH_CONNEXION, NULL, thread_connexion, NULL); 





    #pragma region CLOSE 
    // --------------------------------------- CLOSE --------------------------------------- //
    pthread_join(TH_CONNEXION, NULL); 

    printf("SERVEUR ] FIN DE JEU - FERMETURE DU PROGRAMME");
    //Fermeture des SHM
    shmctl(SHM_INFO_SERVEUR, IPC_RMID, NULL);  // suppr
    shmctl(SHM_SCORE, IPC_RMID, NULL);  // suppr

    //Fermeture des sémaphores 
    sem_close(SEM_INFO_SERVEUR); 
    sem_unlink(CONST_SEM_NOM_INFO_SERVEUR); 


    sem_close(SEM_SCORE); 
    // ------------------------------------------------------------------------------------ //
    #pragma endregion

    return 0;
}

void affichage_serveur(){
    struct shmid_ds stat;
    
    shmctl(SHM_INFO_SERVEUR, IPC_STAT, &stat);
    
    printf("MAIN ] ======== INFO SERVEUR ========\n");

    printf("MAIN ] taille : %ld\n", stat.shm_segsz);
    printf("MAIN ] atime : %ld\n", stat.shm_atime);
    printf("MAIN ] ctime : %ld\n", stat.shm_ctime);
    printf("MAIN ] dtime : %ld\n", stat.shm_dtime);
    printf("MAIN ] cpid : %d\n", stat.shm_cpid);
    printf("MAIN ] lpid : %d\n", stat.shm_lpid);
    printf("MAIN ] nattch : %ld\n", stat.shm_nattch);
    printf("MAIN ] perm.cgid : %d\n", stat.shm_perm.cgid);
    printf("MAIN ] perm.cuid : %d\n", stat.shm_perm.cuid);
    printf("MAIN ] perm.gid : %d\n", stat.shm_perm.gid);
    printf("MAIN ] perm.mode : %d\n", stat.shm_perm.mode);
    printf("MAIN ] perm.uid : %d\n", stat.shm_perm.uid);

    shmctl(SHM_SCORE, IPC_STAT, &stat);
    
    printf("\nMAIN ] ======== SCORE ========\n");

    printf("MAIN ] taille : %ld\n", stat.shm_segsz);
    printf("MAIN ] atime : %ld\n", stat.shm_atime);
    printf("MAIN ] ctime : %ld\n", stat.shm_ctime);
    printf("MAIN ] dtime : %ld\n", stat.shm_dtime);
    printf("MAIN ] cpid : %d\n", stat.shm_cpid);
    printf("MAIN ] lpid : %d\n", stat.shm_lpid);
    printf("MAIN ] nattch : %ld\n", stat.shm_nattch);
    printf("MAIN ] perm.cgid : %d\n", stat.shm_perm.cgid);
    printf("MAIN ] perm.cuid : %d\n", stat.shm_perm.cuid);
    printf("MAIN ] perm.gid : %d\n", stat.shm_perm.gid);
    printf("MAIN ] perm.mode : %d\n", stat.shm_perm.mode);
    printf("MAIN ] perm.uid : %d\n", stat.shm_perm.uid);

    printf("MAIN ] ===========================\n");

    printf("\n\nMAIN ] =========== PID ==========\n");

    printf("MAIN ] %d\n", getpid());

    printf("MAIN ] ==========================\n");
}

void * thread_connexion(void * arg){
    printf("Début thread de connexion \n"); 
    

    pthread_exit(0); 
}

