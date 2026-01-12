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

// SHM
int SHM_INFO_SERVEUR;
int SHM_SCORE;

// MUTEX
pthread_mutex_t MUT_UPDATE_SERVER = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t MUT_LISTE_JOUEURS = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t MUT_ETAT_JOUEURS = PTHREAD_MUTEX_INITIALIZER; 

// CONDITION 
pthread_cond_t COND_TRY_CONNEXION = PTHREAD_COND_INITIALIZER;

// BAL
int BAL_ID; 

// JOUEURS
joueurs_t joueurs_enregistre; 
bool_t etat_joueurs[CONST_NOMBRE_JOUEURS]; 

// PROTOTYPE
void * thread_connexion(void * arg);
void * thread_ecoute(void * arg); 
void affichage_serveur();  
void affichage_liste_joueurs();
void changement_etat_serveur(etat_serveur_t etat_serveur); 
int find_index_player(pid_t pid_joueur); 
void affichage_liste_joueur_etat(); 
bool ready_player_start(); 
void affichage_attente(); 
int nb_ready_player(); 


int main(){

    #pragma region INIT
    // --------------------------------------- INIT --------------------------------------- //
    pthread_t TH_CONNEXION; 
    pthread_t TH_ECOUTE;  

    // Initialisation des sémaphores
    SEM_INFO_SERVEUR = sem_open(CONST_SEM_NOM_INFO_SERVEUR,  O_CREAT, 0666, 0); 
    SEM_SCORE = sem_open(CONST_SEM_NOM_SCORE, O_CREAT, 0666, 1); 

    // CREATION DES FICHIERS POUR LES SHM SI NECESSAIRE
    creation_fichiers_necessaires(TRUE);

    // Initialisation des SHM
    char chemin[500];  // pour concaténer les éléments du chemin
    // SHM INFO SERVEUR
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_INFO_SERVEUR);
    key_t tok_INFO_SERVEUR = ftok(chemin, CONST_PROJECT_ID); 
    SHM_INFO_SERVEUR = shmget(tok_INFO_SERVEUR, sizeof(info_serveur_t), 0666 | IPC_CREAT); 
    CHECK(SHM_INFO_SERVEUR, "DEBUG ] SERVEUR ] Erreur shmget SHM_INFO_SERVEUR"); 

    // SHM SCORE
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_SCORE);
    key_t tok_SCORE = ftok(chemin, CONST_PROJECT_ID); 
    SHM_SCORE = shmget(tok_SCORE, sizeof(score_t), 0666 | IPC_CREAT); 
    CHECK(SHM_SCORE, "DEBUG ] SERVEUR ] Erreur shmget SHM_SCORE");
    
    // BOITE AUX LETTRES
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_BAL);
    key_t tok_BAL = ftok(chemin, CONST_PROJECT_ID);
    CHECK(tok_BAL, "DEBUG ] CLIENT ] Erreur ftok BAL");
    BAL_ID = msgget(tok_BAL, 0666 | IPC_CREAT);
    CHECK(BAL_ID, "DEBUG ] CLIENT ] Erreur msgget BAL");

    // Initialisation liste des joueurs
    joueurs_enregistre.nb_joueurs = 0; 
    joueurs_enregistre.nb_joueurs_en_partie = 0; 

    // Initialisation bool d'état des joueurs
    for(int i = 0; i < CONST_NOMBRE_JOUEURS; i++){
        etat_joueurs[i] = FALSE; 
    }

    affichage_serveur();  // AFFICHAGE
    // ------------------------------------------------------------------------------------ //
    #pragma endregion
    

    #pragma region WRITE_INFO_SERVEUR
    // --------------------------------------- WRITE_INFO_SERVEUR --------------------------------------- //
    // Ecriture dans SHM_INFO_SERVEUR
    // Pas d'utilisation de la fonction changement_etat_serveur car la sémaphore est initialisé à 0 au début du programme
    pthread_mutex_lock(&MUT_UPDATE_SERVER); 
    info_serveur_t * info_serveur = shmat(SHM_INFO_SERVEUR, NULL, 0); 
    info_serveur->pid_serveur = getpid(); 
    info_serveur->etat_serveur = ATTENTE; 
    shmdt(info_serveur); 
    sem_post(SEM_INFO_SERVEUR);  
    pthread_mutex_unlock(&MUT_UPDATE_SERVER); 

    // -------------------------------------------------------------------------------------------------- //
    #pragma endregion


    #pragma region CREATE_THREAD
    // --------------------------------------- CREATE_THREAD --------------------------------------- //
    // Création des threads
    // THREAD DE CONNEXION
    pthread_create(&TH_CONNEXION, NULL, thread_connexion, NULL); 
    pthread_create(&TH_ECOUTE, NULL, thread_ecoute, NULL); 
    #pragma endregion

    
    #pragma region JEU
    // --------------------------------------- JEU --------------------------------------- //
        init_ncurses(); 
        #pragma region ATTENTE
            // --------------------------------------- ATTENTE --------------------------------------- //

            // MODIFICATION ETAT DU SERVEUR
            pthread_mutex_lock(&MUT_UPDATE_SERVER); 
            changement_etat_serveur(ATTENTE);  
            pthread_mutex_unlock(&MUT_UPDATE_SERVER); 

            // REINITIALISATION DU NOMBRE DE JOUEURS EN PARTIE
            pthread_mutex_lock(&MUT_LISTE_JOUEURS); 
            joueurs_enregistre.nb_joueurs_en_partie = 0; 
            pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 

            // AFFICHAGE ATTENTE NCURSES
            affichage_attente(); 

            while(!ready_player_start()){
                // ncurses ne fonctionne pas avec les threads 
                affichage_attente(); 
                usleep(100000); // 100 ms
            }
            // -------------------------------------------------------------------------------------------------- //
        #pragma endregion


        #pragma region PARTIE 
            // --------------------------------------- PARTIE --------------------------------------- //

            // -------------------------------------------------------------------------------------------------- //
        #pragma endregion

        
        #pragma region PODIUM 
            // --------------------------------------- PODIUM --------------------------------------- //

            // -------------------------------------------------------------------------------------------------- //
        #pragma endregion

    // -------------------------------------------------------------------------------------------------- //
    #pragma endregion


    #pragma region CLOSE 
    // --------------------------------------- CLOSE --------------------------------------- //
    pthread_join(TH_CONNEXION, NULL);
    pthread_join(TH_ECOUTE, NULL);   

    printf("SERVEUR ] FIN DE JEU - FERMETURE DU PROGRAMME");
    // Fermeture des SHM
    shmctl(SHM_INFO_SERVEUR, IPC_RMID, NULL);  // suppr
    shmctl(SHM_SCORE, IPC_RMID, NULL);  // suppr

    // Fermeture des sémaphores 
    sem_close(SEM_INFO_SERVEUR); 
    sem_unlink(CONST_SEM_NOM_INFO_SERVEUR); 


    sem_close(SEM_SCORE); 
    // ------------------------------------------------------------------------------------ //
    #pragma endregion

    return EXIT_SUCCESS;
}


#pragma region THREAD
// --------------------------------------- THREAD --------------------------------------- //
    void * thread_connexion(void * arg){
        //printf("SERVEUR ] Début thread de connexion \n"); 
        msg_login_t msg_login;
        msg_reponse_serveur_t reponse_serveur; 
        while(1){
            msgrcv(BAL_ID, &msg_login, MSG_SIZEOF(msg_login_t), MSG_TYPE_LOGIN, 0);

            pid_t pid_client = msg_login.msg.pid_client;
            char pseudo[CONST_LONGUEUR_PSEUDO]; 
            strcpy(pseudo, msg_login.msg.pseudo); 
            
            //printf("SERVEUR ] Un joueur essaye de se connecter : \n"); 
            //printf("SERVEUR ] Voici les données enregistrées : PSEUDO : %s || PID : %d \n", pseudo, pid_client); 

            reponse_serveur.type = pid_client;

            pthread_mutex_lock(&MUT_LISTE_JOUEURS);  

            if (joueurs_enregistre.nb_joueurs < CONST_NOMBRE_JOUEURS){
                joueurs_enregistre.liste_joueurs[joueurs_enregistre.nb_joueurs].pid_client = pid_client;
                strcpy(joueurs_enregistre.liste_joueurs[joueurs_enregistre.nb_joueurs].pseudo, pseudo); 
                joueurs_enregistre.nb_joueurs++; 
                
                strcpy(reponse_serveur.msg, CONST_LOGIN_OK); 

                //printf("\n\nDEBUG ] SERVEUR ] NOUVELLE LISTE DE JOUEURS :\n"); 
                //affichage_liste_joueurs(); 
            }else{
                strcpy(reponse_serveur.msg, CONST_LOGIN_ECHEC); 
                //printf("\n\nDEBUG ] SERVEUR ] TROP DE JOUEURS DEJA CONNECTE\n"); 
            }

            pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 

            msgsnd(BAL_ID, &reponse_serveur, MSG_SIZEOF(msg_reponse_serveur_t), 0);
            
        }
        pthread_exit(0); 
    }

    void * thread_ecoute(void * arg){
        //printf("SERVEUR ] Début du thread d'écoute \n"); 
        ready_player_t ready_player; 
        int index; 
        
        while(1){
            msgrcv(BAL_ID, &ready_player, MSG_SIZEOF(ready_player_t), MSG_TYPE_READY, 0); 
            pid_t pid_joueur = ready_player.pid_joueur; 
            bool_t ready = ready_player.ready; 

            index = find_index_player(pid_joueur);

            pthread_mutex_lock(&MUT_ETAT_JOUEURS); 
            if (index != -1){
                etat_joueurs[index] = ready; 
            }
            pthread_mutex_unlock(&MUT_ETAT_JOUEURS); 

        }    
        
        //printf("EN ATTENDE DES JOUEURS"); 
        //affichage_liste_joueur_etat();
        //affichage_attente(); 

        pthread_exit(0); 
    }
// ------------------------------------------------------------------------------------ //
#pragma endregion 

#pragma region FONCTION GESTION 
// --------------------------------------- FONCTION GESTION --------------------------------------- //
    int find_index_player(pid_t pid_joueur){
        pthread_mutex_lock(&MUT_LISTE_JOUEURS); 
        for(int i = 0; i < joueurs_enregistre.nb_joueurs; i++){
            if(joueurs_enregistre.liste_joueurs[i].pid_client == pid_joueur){
                pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 
                return i; 
            }
        }
        pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 
        return -1; 
    }

    bool ready_player_start(){

        pthread_mutex_lock(&MUT_LISTE_JOUEURS);

        int readyJoueur = 0; 
        int nbTotalJoueur = joueurs_enregistre.nb_joueurs; 

        if (nbTotalJoueur < 3) {
            pthread_mutex_unlock(&MUT_LISTE_JOUEURS);
            return false; // Cas particulier : aucun joueur dans la liste
        }
        pthread_mutex_unlock(&MUT_LISTE_JOUEURS);

        readyJoueur = nb_ready_player(); 

        return (((float)readyJoueur/nbTotalJoueur) > 0.75); 
    } 

    int nb_ready_player(){
        int readyJoueur = 0; 

        pthread_mutex_lock(&MUT_LISTE_JOUEURS);
        
        for(int i = 0; i < joueurs_enregistre.nb_joueurs; i++){
            pthread_mutex_lock(&MUT_ETAT_JOUEURS); 
            if(etat_joueurs[i] == true) readyJoueur++;  
            pthread_mutex_unlock(&MUT_ETAT_JOUEURS);
        }

        pthread_mutex_unlock(&MUT_LISTE_JOUEURS);

        return readyJoueur; 
    }

    void changement_etat_serveur(etat_serveur_t etat_serveur){
        sem_wait(SEM_INFO_SERVEUR); 

        info_serveur_t * info_serveur = shmat(SHM_INFO_SERVEUR, NULL, 0); 
        info_serveur->pid_serveur = getpid(); 
        info_serveur->etat_serveur = etat_serveur; 
        shmdt(info_serveur); 
        

        sem_post(SEM_INFO_SERVEUR); 
    }
// ------------------------------------------------------------------------------------ //
#pragma endregion

#pragma region FONCTION NCURSES 
// --------------------------------------- FONCTION NCURSES --------------------------------------- //
void affichage_attente(){
    int nbJoueurs = 0; 
    affichage_logo(2, 23); 
    mvprintw(7, 23, "Waiting room..."); 
    mvprintw(8, 23, "Welcome to the game : Tetriis !!"); 
    mvprintw(10, 23, "Use [ENTER] to be ready / not ready"); 

    pthread_mutex_lock(&MUT_LISTE_JOUEURS);
    nbJoueurs = joueurs_enregistre.nb_joueurs; 
    pthread_mutex_unlock(&MUT_LISTE_JOUEURS);

    
    mvprintw(12, 23, "Players : %d", nbJoueurs); 
    mvprintw(13, 23, "Players ready : %d / %d" , nb_ready_player(), nbJoueurs); 

    mvprintw(CONST_NB_LIGNES-1, 0, "Tetriis was made by GREBERT Cloe and DUTHOIT Thomas"); 

    refresh(); 
}

// ------------------------------------------------------------------------------------ //
#pragma endregion

#pragma region FONCTION AFFICHAGE
// --------------------------------------- FONCTION AFFICHAGE --------------------------------------- //
void affichage_liste_joueur_etat(){
    pthread_mutex_lock(&MUT_LISTE_JOUEURS);
    for(int i = 0; i < joueurs_enregistre.nb_joueurs; i++){
        printf("SERVEUR ] %s : %d", joueurs_enregistre.liste_joueurs[i].pseudo, etat_joueurs[i]); 
    }
    pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 
}


void affichage_liste_joueurs(){
    for(int i = 0 ; i < joueurs_enregistre.nb_joueurs; i++){
        printf("DEBUG ] SERVEUR ] %d ] %d => %s\n", i, joueurs_enregistre.liste_joueurs[i].pid_client, joueurs_enregistre.liste_joueurs[i].pseudo); 
    }
    printf("\n"); 
}

void affichage_serveur() {
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
// ------------------------------------------------------------------------------------ //
#pragma endregion
