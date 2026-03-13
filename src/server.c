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
pthread_mutex_t MUT_JOUEURS_VIVANTS = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t MUT_CLOSE_ECOUTE = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t MUT_CLOSE_ECOUTE_PARTI = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t MUT_SCORE = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t MUT_EVENT = PTHREAD_MUTEX_INITIALIZER; 

// CONDITION 
pthread_cond_t COND_TRY_CONNEXION = PTHREAD_COND_INITIALIZER;

// BAL
int BAL_ID; 

// JOUEURS
joueurs_t joueurs_enregistre; 
bool_t etat_joueurs[CONST_NOMBRE_JOUEURS]; 
bool_t joueurs_vivants[CONST_NOMBRE_JOUEURS]; 
event_t last_event; 

// VARIABLES GLOBALE
bool close_ecoute = FALSE; 
bool close_ecoute_parti = FALSE; 

// PROTOTYPE
    // ----- THREAD -----
void * thread_connexion(void * arg); //  THREAD ECOUTE CONNEXION DES JOUEURS
void * thread_ecoute(void * arg); //  THREAD ECOUTE ATTENTE DE LA PARTIE
void * thread_ecoute_parti(void * arg); //  THREAD ECOUTE DURANT LA PARTIE

    // ----- AFFICHAGE -----
void affichage_serveur(); //  TEXTUEL : AFFICHAGE DES DONNEES DES SHM DU SERVEUR
void affichage_liste_joueurs(); //  DEBUG : AFFICHAGE DE LA LISTE DES JOUEURS
void affichage_liste_joueur_etat(); //  DEBUG : AFFICHAGE ETAT DES JOUEURS
void affichage_attente(); //  NCURSES : AFFICHAGE D'ATTENTE

    // ----- LECTURE/ECRITURE DONNEES -----
void changement_etat_serveur(etat_serveur_t etat_serveur); //  MODIFIE L'ETAT DU SERVEUR (ATTENTE, PARTIE, PODIUM)
int find_index_player(pid_t pid_joueur); //  TROUVE L'INDEX DU JOUEUR SELON SON PID
bool ready_player_start(); //  TEST : VALIDE LE LANCEMENT DE LA PARTIE OU NON
int nb_ready_player(); //  CALCUL NOMBRE DE JOUEURS PRÊTS

    // ----- GESTION DES ELEMENTS DE DONNEES -----
void deroute(); 
void close_shm_sem();
int check_end_game(); 


int main(){

    #pragma region INIT
    // --------------------------------------- INIT --------------------------------------- //
    int nbJoueurs; 
    int rows, cols; 
    int timer; 
    
    pthread_t TH_CONNEXION; 
    pthread_t TH_ECOUTE;  
    //pthread_t TH_ECOUTE_PARTI[CONST_NOMBRE_JOUEURS]; 
    pthread_t TH_ECOUTE_PARTI; 
    struct sigaction newact; 

    // Initialisation des sémaphores
    SEM_INFO_SERVEUR = sem_open(CONST_SEM_NOM_INFO_SERVEUR,  O_CREAT, 0666, 0);
    SEM_SCORE = sem_open(CONST_SEM_NOM_SCORE, O_CREAT, 0666, 1);  

    // CREATION DES FICHIERS POUR LES SHM SI NECESSAIRE
    creation_fichiers_necessaires(TRUE);

    // Initialisation des SHM
    char chemin[500];  // pour concaténer les éléments du chemin
    // SHM INFO SERVEUR
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_INFO_SERVEUR);
    key_t tok_INFO_SERVEUR = ftok(chemin, CONST_PROJECT_ID_INFO); 
    SHM_INFO_SERVEUR = shmget(tok_INFO_SERVEUR, sizeof(info_serveur_t), 0666 | IPC_CREAT); 
    CHECK(SHM_INFO_SERVEUR, "DEBUG ] SERVEUR ] Erreur shmget SHM_INFO_SERVEUR"); 

    // SHM SCORE
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_SCORE);
    key_t tok_SCORE = ftok(chemin, CONST_PROJECT_ID_SCORE); 
    SHM_SCORE = shmget(tok_SCORE, sizeof(score_t), 0666 | IPC_CREAT); 
    CHECK(SHM_SCORE, "DEBUG ] SERVEUR ] Erreur shmget SHM_SCORE");

    // BOITE AUX LETTRES
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_BAL);
    key_t tok_BAL = ftok(chemin, CONST_PROJECT_ID_BAL);
    CHECK(tok_BAL, "DEBUG ] CLIENT ] Erreur ftok BAL");
    BAL_ID = msgget(tok_BAL, 0666 | IPC_CREAT);
    CHECK(BAL_ID, "DEBUG ] CLIENT ] Erreur msgget BAL");

    // Initialisation liste des joueurs
    joueurs_enregistre.nb_joueurs = 0; 
    joueurs_enregistre.nb_joueurs_en_partie = 0; 

    // Initialisation bool d'état des joueurs
    pthread_mutex_lock(&MUT_ETAT_JOUEURS); 
    for(int i = 0; i < CONST_NOMBRE_JOUEURS; i++){
        etat_joueurs[i] = FALSE; 
    }
    pthread_mutex_unlock(&MUT_ETAT_JOUEURS); 

    //  Initialisation bool d'état des joueurs en partie (VIVANT = TRUE | MORT = FALSE)
    pthread_mutex_lock(&MUT_JOUEURS_VIVANTS); 
    for(int i = 0; i < CONST_NOMBRE_JOUEURS; i++){
        joueurs_vivants[i] = TRUE; 
    }
    pthread_mutex_unlock(&MUT_JOUEURS_VIVANTS);

    affichage_serveur();  // AFFICHAGE
    // ------------------------------------------------------------------------------------ //
    #pragma endregion

    #pragma region DEROUTE 
    // --------------------------------------- DEROUTE --------------------------------------- //
    newact.sa_flags = 0;
    sigemptyset(&newact.sa_mask);
    newact.sa_handler = deroute;
    CHECK(sigaction(SIGINT, &newact, NULL), "Problème sigaction");

    // --------------------------------------------------------------------------------------- //
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


    #pragma region CREATE_THREAD_CONNEXION
    // --------------------------------------- CREATE_THREAD_CONNEXION --------------------------------------- //
    // Création des threads
    // THREAD DE CONNEXION
    pthread_create(&TH_CONNEXION, NULL, thread_connexion, NULL); 
    #pragma endregion

    
    #pragma region JEU
    do
    {
    // --------------------------------------- JEU --------------------------------------- //
        #pragma region REINIT 
        // REINITIALISATION DES DONNEES APRES UNE PARTIE 
        timer = 0; 
        init_ncurses(); 
        nodelay(stdscr, TRUE);   // getch devient non bloquant

        pthread_mutex_lock(&MUT_CLOSE_ECOUTE);
        close_ecoute = FALSE;
        pthread_mutex_unlock(&MUT_CLOSE_ECOUTE);

        pthread_mutex_lock(&MUT_CLOSE_ECOUTE_PARTI); 
        close_ecoute_parti = FALSE; 
        pthread_mutex_unlock(&MUT_CLOSE_ECOUTE_PARTI);

        pthread_mutex_lock(&MUT_EVENT); 
        last_event.event = 0; 
        strcpy(last_event.pseudo, "");
        pthread_mutex_unlock(&MUT_EVENT); 


        #pragma endregion
    
    
        #pragma region ATTENTE
            // --------------------------------------- ATTENTE --------------------------------------- //

            // MODIFICATION ETAT DU SERVEUR
            changement_etat_serveur(ATTENTE);  


            msg_ready_player_t tmp;
            while (msgrcv(BAL_ID, &tmp, MSG_SIZEOF(msg_ready_player_t),MSG_TYPE_READY, IPC_NOWAIT) != -1);

            pthread_mutex_lock(&MUT_CLOSE_ECOUTE);
            close_ecoute = FALSE;
            pthread_mutex_unlock(&MUT_CLOSE_ECOUTE);

            pthread_create(&TH_ECOUTE, NULL, thread_ecoute, NULL); 

            // REINITIALISATION DU NOMBRE DE JOUEURS EN PARTIE
            pthread_mutex_lock(&MUT_LISTE_JOUEURS); 
            joueurs_enregistre.nb_joueurs_en_partie = 0; 
            pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 

            while(!ready_player_start()){
                // ncurses ne fonctionne pas avec les threads 
                affichage_attente(); 
                usleep(100000); // 100 ms
            }

            //  FIN DE L'ATTENTE : GESTION D'AVANT PARTIE 
            //  Fermeture du thread
            pthread_mutex_lock(&MUT_CLOSE_ECOUTE); 
            close_ecoute = TRUE; 
            pthread_mutex_unlock(&MUT_CLOSE_ECOUTE); 
            pthread_join(TH_ECOUTE, NULL);   

            // SIGNAL DE LANCEMENT DE LA PARTIE 
            pthread_mutex_lock(&MUT_LISTE_JOUEURS); 
            for(int i = 0; i < joueurs_enregistre.nb_joueurs; i++){
                kill(joueurs_enregistre.liste_joueurs[i].pid_client, SIG_START); 
            }
            pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 

            // SAUVEGARDE DU NOMBRE DE JOUEURS
            pthread_mutex_lock(&MUT_LISTE_JOUEURS); 
            joueurs_enregistre.nb_joueurs_en_partie = joueurs_enregistre.nb_joueurs; 
            pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 

            // -------------------------------------------------------------------------------------------------- //
        #pragma endregion

        #pragma region CHANGEMENT ETAT SERVEUR = PARTIE

        // MODIFICATION ETAT DU SERVEUR
        changement_etat_serveur(PARTIE); 

        #pragma endregion

        #pragma region COMPTEUR
        // AFFICHAGE DU LANCEMENT DE LA PARTIE 

        for (int s=3; s>0; s--) {
            for (int p=1; p<4; p++) {
                affichage_compteur(s, p); 
                usleep(333333); // 1/3 s  
            }
            usleep(1000000);
        }
        affichage_lancement(); 
        usleep(1000000); // 1 s
        
        #pragma endregion

        #pragma region PARTIE 
            // --------------------------------------- PARTIE --------------------------------------- //

            //  OUVERTURE DU THREAD D'ECOUTE DE JEU
            pthread_create(&TH_ECOUTE_PARTI, NULL, thread_ecoute_parti, NULL);

            
            do{   // ATTENTE DE FIN DE PARTIE
                //  Affichage
                getmaxyx(stdscr, rows, cols);
                clear(); 
                affichage_logo(2, ((cols-34)/2)); 
                mvprintw(7, ((cols-19)/2), "Game in progress..."); 

                pthread_mutex_lock(&MUT_LISTE_JOUEURS);
                nbJoueurs = joueurs_enregistre.nb_joueurs_en_partie; 
                pthread_mutex_unlock(&MUT_LISTE_JOUEURS);

                mvprintw(9, ((cols-25)/2), "Higher score : [Score] by [Pseudo]"); 
                mvprintw(11, ((cols-30)/2), "Number of players alive : %d/%d", check_end_game(), nbJoueurs); 
                mvprintw(13, ((cols-10)/2), "Time : %d s", timer); 
                timer++; 

                switch(last_event.event){
                    case 1 :
                        // 1 -> DERNIER EVENT : MORT d'UN JOUEUR

                        attron(A_BOLD);
                        attron(COLOR_PAIR(3));
                        pthread_mutex_lock(&MUT_EVENT); 
                            mvprintw(15, ((cols-22)/2), "%s", last_event.pseudo); 
                        pthread_mutex_unlock(&MUT_EVENT); 
                        attron(COLOR_PAIR(1));
                        attroff(A_BOLD);
                        printw(" is dead...");
                        refresh(); 

                        break; 

                    case 2 : 
                        // 2 -> DERNIER EVENT : BONUS/MALUS D'UN JOUEUR

                        // AFFICHAGE MALUS
                        // AFFICHAGE MORT SUR LE SERVEUR
                        pthread_mutex_lock(&MUT_LISTE_JOUEURS); 
                        attron(A_BOLD);
                        attron(COLOR_PAIR(2));
                        pthread_mutex_lock(&MUT_EVENT); 
                            mvprintw(14, ((cols-50)/2), "%s", last_event.pseudo); 
                        pthread_mutex_unlock(&MUT_EVENT); 
                        attron(COLOR_PAIR(1));
                        attroff(A_BOLD);
                        printw(" gave everyone a lovely gift!");
                        pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 
                        refresh(); 

                        break; 

                    default : 
                        break; 
                }


                mvprintw(rows-1, 0, "Tetriis was made by GREBERT Cloe and DUTHOIT Thomas"); 
                refresh(); 
 
                usleep(1000000); // 1s
            } while(check_end_game() != 1); 
 
            //Enregistrement du dernier joueur de la partie
            //Récup index dernier survivant.
            pthread_mutex_lock(&MUT_LISTE_JOUEURS); 
            int nbJoueurs = joueurs_enregistre.nb_joueurs_en_partie; 
            pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 
            int index; 
            pthread_mutex_lock(&MUT_JOUEURS_VIVANTS); 
                for(int i = 0; i < nbJoueurs; i++){
                    if(joueurs_vivants[i] == TRUE){
                        index = i; 
                        break;  
                    }
                }
            pthread_mutex_unlock(&MUT_JOUEURS_VIVANTS); 
            char pseudo_joueur[CONST_LONGUEUR_PSEUDO]; 
            pthread_mutex_lock(&MUT_LISTE_JOUEURS); 
                strcpy(pseudo_joueur, joueurs_enregistre.liste_joueurs[index].pseudo); 
            pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 

            pthread_mutex_lock(&MUT_SCORE); 
                        
                sem_wait(SEM_SCORE); 
            
                    score_t * info_score = shmat(SHM_SCORE, NULL, 0); 
                    strcpy(info_score->last_survivors[0], pseudo_joueur); 
                    shmdt(info_score); 
                    
                sem_post(SEM_SCORE); 

            pthread_mutex_unlock(&MUT_SCORE); 
            
            pthread_mutex_lock(&MUT_CLOSE_ECOUTE_PARTI); 
            close_ecoute_parti = TRUE;              // PARTIE TERMINEE / FERMETURE THREAD ECOUTE PARTIE
            pthread_mutex_unlock(&MUT_CLOSE_ECOUTE_PARTI); 

            pthread_join(TH_ECOUTE_PARTI, NULL); 

            //while(1); 


            // -------------------------------------------------------------------------------------------------- //
        #pragma endregion

        
        #pragma region PODIUM 
            // --------------------------------------- PODIUM --------------------------------------- //
            // MODIFICATION ETAT DU SERVEUR
            changement_etat_serveur(PODIUM);  

            // SIGNAL DE FIN DE PARTIE
            pthread_mutex_lock(&MUT_LISTE_JOUEURS); 

                for(int i = 0; i < joueurs_enregistre.nb_joueurs_en_partie; i++){

                    pid_t pid = joueurs_enregistre.liste_joueurs[i].pid_client;

                    if(kill(pid, SIG_END) == -1){
                        perror("kill SIG_END");
                    }
                }

            pthread_mutex_unlock(&MUT_LISTE_JOUEURS);



            do
            {
                pthread_mutex_lock(&MUT_SCORE); 
                            
                    sem_wait(SEM_SCORE); 
                
                        info_score = shmat(SHM_SCORE, NULL, 0); 
                        
                        affichage_podium(info_score); 


                        shmdt(info_score); 
                        
                    sem_post(SEM_SCORE); 

                pthread_mutex_unlock(&MUT_SCORE); 
                usleep(500000); // 50ms
            } while (1);


            // -------------------------------------------------------------------------------------------------- //
        #pragma endregion
    } while (1);

    // -------------------------------------------------------------------------------------------------- //
    #pragma endregion


    #pragma region CLOSE 
    // --------------------------------------- CLOSE --------------------------------------- //
    pthread_join(TH_CONNEXION, NULL);

    printf("SERVEUR ] FIN DE JEU - FERMETURE DU PROGRAMME");
    close_shm_sem(); 


    sem_close(SEM_SCORE); 
    endwin(); 
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
            bool_t ferme = msg_login.msg.fermer;
            
            //printf("SERVEUR ] Un joueur essaye de se connecter : \n"); 
            //printf("SERVEUR ] Voici les données enregistrées : PSEUDO : %s || PID : %d \n", pseudo, pid_client); 

            reponse_serveur.type = pid_client;
 
            switch(ferme){
                case 0 : 
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
                    break; 
                case 1 : 
                    pthread_mutex_lock(&MUT_LISTE_JOUEURS);
                        int index = find_index_player(pid_client); 
                        if(index != -1){

                            for(int i = index; i < joueurs_enregistre.nb_joueurs; i++){
                                joueurs_enregistre.liste_joueurs[i] = joueurs_enregistre.liste_joueurs[i+1]; 
                            }
                            joueurs_enregistre.nb_joueurs--; 

                        }
                    pthread_mutex_unlock(&MUT_LISTE_JOUEURS);
                    break; 
                default : 
                    break; 
            } 

            msgsnd(BAL_ID, &reponse_serveur, MSG_SIZEOF(msg_reponse_serveur_t), 0);
            
        }
        pthread_exit(0); 
    }

    void * thread_ecoute(void * arg){
        //printf("SERVEUR ] Début du thread d'écoute \n"); 
        msg_ready_player_t ready_player; 
        int index; 
        bool close = FALSE; 
        
        while(1){

            pthread_mutex_lock(&MUT_CLOSE_ECOUTE);
            close = close_ecoute;
            pthread_mutex_unlock(&MUT_CLOSE_ECOUTE);

            if(close) break;

            if(msgrcv(BAL_ID, &ready_player,MSG_SIZEOF(msg_ready_player_t),MSG_TYPE_READY,IPC_NOWAIT) != -1)
            {
                pid_t pid_joueur = ready_player.pid_joueur;
                bool_t ready = ready_player.ready;

                pthread_mutex_lock(&MUT_LISTE_JOUEURS);
                int index = find_index_player(pid_joueur);
                pthread_mutex_unlock(&MUT_LISTE_JOUEURS);

                pthread_mutex_lock(&MUT_ETAT_JOUEURS); 
                if(index != -1)
                    etat_joueurs[index] = ready;
                pthread_mutex_unlock(&MUT_ETAT_JOUEURS); 
            }

            usleep(50000); // 50ms
        }   
        

        pthread_mutex_lock(&MUT_CLOSE_ECOUTE); 
        close_ecoute = FALSE; 
        pthread_mutex_unlock(&MUT_CLOSE_ECOUTE);
        //printf("EN ATTENDE DES JOUEURS"); 
        //affichage_liste_joueur_etat();
        //affichage_attente(); 

        pthread_exit(0); 
    }

    void * thread_ecoute_parti(void * arg){
        // Données écouté durant la partie 
        // Mort d'un joueur dans la partie
        // MALUS/BONUS
        bool close = FALSE;  
        msg_game_player_t msg_game_player; 
        pid_t pid_joueur; 

        pid_t pid_joueur_premier; 
        char pseudo_joueur[CONST_LONGUEUR_PSEUDO]; 


        while(1){
            pthread_mutex_lock(&MUT_CLOSE_ECOUTE_PARTI); 
            close = close_ecoute_parti; 
            pthread_mutex_unlock(&MUT_CLOSE_ECOUTE_PARTI);

            if(close) break; 

            if(msgrcv(BAL_ID, &msg_game_player, MSG_SIZEOF(msg_game_player_t), MSG_TYPE_GAME, IPC_NOWAIT) != -1)
            {
                //Récupère le pid du joueur 
                pid_joueur = msg_game_player.pid_joueur; 
                pthread_mutex_lock(&MUT_LISTE_JOUEURS); 
                int index_joueur = find_index_player(pid_joueur); 
                pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 

                pthread_mutex_lock(&MUT_LISTE_JOUEURS); 
                    strcpy(pseudo_joueur, joueurs_enregistre.liste_joueurs[index_joueur].pseudo); 
                pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 

                switch (msg_game_player.type_msg)
                {
                case GAME_MSG_DEATH:  // MORT D'UN JOUEUR
                    pthread_mutex_lock(&MUT_JOUEURS_VIVANTS); 
                    joueurs_vivants[index_joueur] = FALSE; 
                    pthread_mutex_unlock(&MUT_JOUEURS_VIVANTS); 

                    // STOCKER POUR AFFICHAGE MORT SUR LE SERVEUR
                    pthread_mutex_lock(&MUT_EVENT);    
                        last_event.event = 1; 
                        strcpy(last_event.pseudo, pseudo_joueur); 
                    pthread_mutex_unlock(&MUT_EVENT);

                    if (check_end_game() < 3){
                        pthread_mutex_lock(&MUT_SCORE); 
                        
                            sem_wait(SEM_SCORE); 
                        
                                score_t * info_score = shmat(SHM_SCORE, NULL, 0); 
                                strcpy(info_score->last_survivors[check_end_game()], pseudo_joueur); 
                                shmdt(info_score); 
                                
                            sem_post(SEM_SCORE); 

                        pthread_mutex_unlock(&MUT_SCORE); 
                    }


                    break;
                
                case GAME_MSG_LINE:  // MALUS/BONUS SUITE A 2 LIGNES COMPLETES 
                    // Un joueur a rempli 2 lignes en un coup
                    // Malus : tous les autres joueurs ont une ligne en plus
                    // Bonus : le joueur avec le plus haut score n'a pas de ligne en plus 
                    
                    // Lecture premier joueur dans le score
                    pthread_mutex_lock(&MUT_SCORE);  
        
                    sem_wait(SEM_SCORE); 
                        
                        score_t * info_score = shmat(SHM_SCORE, NULL, 0); 
                        pid_joueur_premier = info_score->premier.login_joueur.pid_client; 
                        shmdt(info_score); 
                        
                    sem_post(SEM_SCORE); 
                    
                    pthread_mutex_unlock(&MUT_SCORE); 
                    
                    pthread_mutex_lock(&MUT_LISTE_JOUEURS); 

                    msg_game_server_t msg_server;
                    msg_server.type_msg = GAME_MSG_MALUS;

                    for(int i = 0; i < joueurs_enregistre.nb_joueurs; i++){
                        if(joueurs_enregistre.liste_joueurs[i].pid_client != pid_joueur_premier && joueurs_enregistre.liste_joueurs[i].pid_client != pid_joueur){
                            msg_server.type = joueurs_enregistre.liste_joueurs[i].pid_client;
                            msgsnd(BAL_ID, &msg_server, MSG_SIZEOF(msg_game_server_t), 0); 
                        }
                    }

                    pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 

                    // STOCKER POUR AFFICHAGE BONUS/MALUS SUR LE SERVEUR
                    pthread_mutex_lock(&MUT_EVENT);    
                        last_event.event = 2; 
                        strcpy(last_event.pseudo, pseudo_joueur); 
                    pthread_mutex_unlock(&MUT_EVENT);

                    break; 
                
                default:
                    break;
                }
                usleep(500000); // 5s
            }
            
            usleep(50000); // 50ms
        }


        pthread_exit(0); 
    }
// ------------------------------------------------------------------------------------ //
#pragma endregion 

#pragma region FONCTION GESTION 
// --------------------------------------- FONCTION GESTION --------------------------------------- //
    int find_index_player(pid_t pid_joueur){
        //pthread_mutex_lock(&MUT_LISTE_JOUEURS); 
        for(int i = 0; i < joueurs_enregistre.nb_joueurs; i++){
            if(joueurs_enregistre.liste_joueurs[i].pid_client == pid_joueur){
                //pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 
                return i; 
            }
        }
        //pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 
        return -1; 
    }

    bool ready_player_start(){
        pthread_mutex_lock(&MUT_LISTE_JOUEURS);
        int readyJoueur = 0; 
        int nbTotalJoueur = joueurs_enregistre.nb_joueurs; 

        if (nbTotalJoueur < 3) { // Cas particulier : aucun joueur dans la liste ou pas assez de joueurs dans la liste
            pthread_mutex_unlock(&MUT_LISTE_JOUEURS);
            return false; 
        }
        pthread_mutex_unlock(&MUT_LISTE_JOUEURS);

        readyJoueur = nb_ready_player(); 
        return readyJoueur >= (nbTotalJoueur * 3) / 4;
    } 

    int nb_ready_player(){
        int readyJoueur = 0; 

        pthread_mutex_lock(&MUT_LISTE_JOUEURS);
        pthread_mutex_lock(&MUT_ETAT_JOUEURS); 
        for(int i = 0; i < joueurs_enregistre.nb_joueurs; i++){
            if(etat_joueurs[i] == true) readyJoueur++;  
        }
        pthread_mutex_unlock(&MUT_ETAT_JOUEURS);
        pthread_mutex_unlock(&MUT_LISTE_JOUEURS);

        return readyJoueur; 
    }

    void changement_etat_serveur(etat_serveur_t etat_serveur){
        pthread_mutex_lock(&MUT_UPDATE_SERVER);  
        
            sem_wait(SEM_INFO_SERVEUR); 

                info_serveur_t * info_serveur = shmat(SHM_INFO_SERVEUR, NULL, 0); 
                info_serveur->pid_serveur = getpid(); 
                info_serveur->etat_serveur = etat_serveur; 
                shmdt(info_serveur); 
                

            sem_post(SEM_INFO_SERVEUR); 
            
        pthread_mutex_unlock(&MUT_UPDATE_SERVER); 
    }

    int check_end_game(){
        int compteur = 0; 
        pthread_mutex_lock(&MUT_LISTE_JOUEURS);

        for(int i = 0; i < joueurs_enregistre.nb_joueurs_en_partie; i++){
            pthread_mutex_lock(&MUT_JOUEURS_VIVANTS);
            if(joueurs_vivants[i] == TRUE) compteur++; 
            pthread_mutex_unlock(&MUT_JOUEURS_VIVANTS);
        }

        pthread_mutex_unlock(&MUT_LISTE_JOUEURS);
        return compteur; 
    }
// ------------------------------------------------------------------------------------ //
#pragma endregion

#pragma region FONCTION NCURSES 
// --------------------------------------- FONCTION NCURSES --------------------------------------- //
void affichage_attente(){
    
    int nbJoueurs = 0; 
    int index_joueurs;
    int rows, cols; 
    
    getmaxyx(stdscr, rows, cols); 

    clear(); 
    // Message HEADER
    affichage_logo(2, ((cols-34)/2)); 
    mvprintw(7, ((cols-15)/2), "Waiting room..."); 
    mvprintw(8, ((cols-32)/2), "Welcome to the game : Tetriis !!"); 
    
    // Affichage du message ENTRER
    mvprintw(10, ((cols-34)/2), "Use "); 
    attron(A_BOLD);
    printw("[ENTER]");
    attroff(A_BOLD); 
    printw(" to be ready / not ready"); 
    
    // Affichage du message ESCAPE
    mvprintw(11, ((cols-34)/2), "Use "); 
    attron(A_BOLD); 
    printw("[ESCAPE]"); 
    attroff(A_BOLD); 
    printw(" to run away"); 


    // Affichage des informations dynamiques
    pthread_mutex_lock(&MUT_LISTE_JOUEURS);
        nbJoueurs = joueurs_enregistre.nb_joueurs; 
    pthread_mutex_unlock(&MUT_LISTE_JOUEURS);

    
    mvprintw(12, ((cols-10)/2), "Players : %d", nbJoueurs); 
    if (nbJoueurs < 3){
        attron(COLOR_PAIR(3)); 
        mvprintw(13, ((cols-40)/2), "Waiting for more players before starting");
        attron(COLOR_PAIR(1));  
    }else{
        mvprintw(13, ((cols-30)/2), "Players ready to start : %d / %d" , nb_ready_player(), (int)((float)nbJoueurs*0.75)); 
    }

    if(nbJoueurs > 0){
        pthread_mutex_lock(&MUT_LISTE_JOUEURS); 
            pthread_mutex_lock(&MUT_ETAT_JOUEURS); 
                for(int i = 0; i < joueurs_enregistre.nb_joueurs; i++){
                    index_joueurs = find_index_player(joueurs_enregistre.liste_joueurs[i].pid_client); 
                    
                    if(etat_joueurs[index_joueurs] == TRUE){
                        attron(COLOR_PAIR(2)); //  Joueur prêt VERT
                    }else{
                        attron(COLOR_PAIR(3)); //  Joueur pas prêt ROUGE
                    }

                    mvprintw(15+(i/3), (cols/6)+((i%3)*CONST_LONGUEUR_PSEUDO), "<  %s  >", joueurs_enregistre.liste_joueurs[i].pseudo); 

                    attron(COLOR_PAIR(1));
                }

            pthread_mutex_unlock(&MUT_ETAT_JOUEURS); 
        pthread_mutex_unlock(&MUT_LISTE_JOUEURS); 
    }

    // Message BOTTOM
    mvprintw(rows-1, 0, "Tetriis was made by GREBERT Cloe and DUTHOIT Thomas");
    
    char touche = getch();  // récupération de l'input

    if (touche == 27) {  // touche entrée activée, on toggle le fait d'être prêt
        kill(getpid(), SIGINT); 
    }

    refresh(); 
}


// ------------------------------------------------------------------------------------ //
#pragma endregion







#pragma region FONCTION DEROUTE
// --------------------------------------- FONCTION DEROUTE --------------------------------------- //

void deroute(int signal){


    switch(signal){

        case SIGINT : 
            // SIGINT -> Signal de fermeture du serveur
            printf("Fermeture des joueurs et du serveur \n"); 
            
            for(int i = 0; i < joueurs_enregistre.nb_joueurs; i++){
                kill(joueurs_enregistre.liste_joueurs[i].pid_client, SIGINT); 
            }
            close_shm_sem(); 
            endwin();
            exit(EXIT_SUCCESS);  
            break; 


        default :  
            printf("Commande inconnue"); 
            break; 
    }
}

void close_shm_sem(){
    // Fermeture des SHM
    shmctl(SHM_INFO_SERVEUR, IPC_RMID, NULL);  // suppr
    shmctl(SHM_SCORE, IPC_RMID, NULL);  // suppr

    // Fermeture des sémaphores 
    sem_close(SEM_INFO_SERVEUR); 
    sem_close(SEM_SCORE); 

    sem_unlink(CONST_SEM_NOM_INFO_SERVEUR); 
    sem_unlink(CONST_SEM_NOM_SCORE); 
}

// ------------------------------------------------------------------------------------ //
#pragma endregion 







#pragma region FONCTION AFFICHAGE
// --------------------------------------- FONCTION DEBUG AFFICHAGE --------------------------------------- //
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