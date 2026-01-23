/**
 *      _____ ___ _____ ___ ___ ___ ___
 *     |_   _| __|_   _| _ \_ _|_ _/ __|
 *       | | | _|  | | |   /| | | |\__ \
 *       |_| |___| |_| |_|_\___|___|___/
 *
 * Project : Tetriis
 * File name : client.c
 * Description :  
 *          Main file for the client.
 *          Allows connexion to the server to play in multiplayer mode
 * Authors : DUTHOIT Thomas / GREBERT Cloé
 */

#include "client.h" 

// SEMAPHORE
sem_t * SEM_INFO_SERVEUR;
sem_t * SEM_SCORE;

// SHM
int SHM_INFO_SERVEUR;
int SHM_SCORE;

// BAL
int BAL_ID;

// MUTEXS
pthread_mutex_t MUT_INFO_SERVEUR = PTHREAD_MUTEX_INITIALIZER;

// AUTRES
info_serveur_t serveur; // informations du serveur tetriis
login_t joueur;  // contient les informations sur le client actuel


// PROTOTYPES
void maj_info_serveur();
void deroute(int signal);
void render();



int main(){

    #pragma region INIT
    // --------------------------------------- INIT --------------------------------------- //

    // OUVERTURE DES SEMAPHORES
    SEM_INFO_SERVEUR = sem_open(CONST_SEM_NOM_INFO_SERVEUR, 0); 
    SEM_SCORE = sem_open(CONST_SEM_NOM_SCORE, 0); 
    
    // TEST ERREUR DE CREATION
    if (SEM_INFO_SERVEUR == SEM_FAILED || SEM_SCORE == SEM_FAILED  ) {
        printf("Il faut lancer le serveur avant de lancer un client !\n");
        exit(EXIT_FAILURE);
    }

    // CREATION DES FICHIERS POUR LES SHM SI NECESSAIRE
    creation_fichiers_necessaires(FALSE);

    // CREATION DES SHM
    char chemin[500];  // pour concaténer les éléments du chemin

    // SHM INFO SERVEUR
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_INFO_SERVEUR);
    key_t tok_INFO_SERVEUR = ftok(chemin, CONST_PROJECT_ID_INFO); 
    SHM_INFO_SERVEUR = shmget(tok_INFO_SERVEUR, sizeof(info_serveur_t), 0666 | IPC_CREAT); 
    CHECK(SHM_INFO_SERVEUR, "DEBUG ] CLIENT ] Erreur shmget SHM_INFO_SERVEUR"); 

    // SHM SCORECONST_FIC_SHM_SCORE
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_SCORE);
    key_t tok_SCORE = ftok(chemin, CONST_PROJECT_ID_SCORE); 
    SHM_SCORE = shmget(tok_SCORE, sizeof(score_t), 0666 | IPC_CREAT); 
    CHECK(SHM_SCORE, "DEBUG ] CLIENT ] Erreur shmget SHM_SCORE"); 


    // BOITE AUX LETTRES
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_BAL);
    key_t tok_BAL = ftok(chemin, CONST_PROJECT_ID_BAL);
    CHECK(tok_BAL, "DEBUG ] CLIENT ] Erreur ftok BAL");
    BAL_ID = msgget(tok_BAL, 0666);
    CHECK(BAL_ID, "DEBUG ] CLIENT ] Erreur msgget BAL");

    // ------------------------------------------------------------------------------------ //
    #pragma endregion


    #pragma region DEROUTE 
    // --------------------------------------- DEROUTE --------------------------------------- //

    struct sigaction newact; 
    newact.sa_flags = 0;
    sigemptyset(&newact.sa_mask);
    newact.sa_handler = deroute;
    CHECK(sigaction(SIGINT, &newact, NULL), "Problème sigaction");
    CHECK(sigaction(SIG_START, &newact, NULL), "Problème sigaction");

    // --------------------------------------------------------------------------------------- //
    #pragma endregion


    #pragma region ACQUISITION PID SERVEUR    
    // --------------------------------------- ACQUISITION PID SERVEUR  --------------------------------------- //
    
    maj_info_serveur();

    pthread_mutex_lock(&MUT_INFO_SERVEUR);
    printf("CLIENT] PID du serveur : %d\n", serveur.pid_serveur);
    pthread_mutex_unlock(&MUT_INFO_SERVEUR);

    // -------------------------------------------------------------------------------------------------------- //
    #pragma endregion


    #pragma region CONNEXION
    // --------------------------------------- CONNEXION --------------------------------------- //


        // ACQUISITION DU PSEUDO
        printf("CLIENT] Pseudo (%d char. max) : ", CONST_LONGUEUR_PSEUDO);
        fgets(joueur.pseudo, CONST_LONGUEUR_PSEUDO, stdin);
        joueur.pseudo[strlen(joueur.pseudo)-1] = '\0';

        // ACQUISITION DU PID
        joueur.pid_client = getpid();
        printf("CLIENT] PID du client : %d\n", joueur.pid_client);

        msg_login_t msg_login;
        msg_login.type = MSG_TYPE_LOGIN;
        msg_login.msg = joueur;
        
        msgsnd(BAL_ID, &msg_login, MSG_SIZEOF(msg_login_t), 0);


        msg_reponse_serveur_t msg_reponse;

        msgrcv(BAL_ID, &msg_reponse, MSG_SIZEOF(msg_reponse_serveur_t), joueur.pid_client, 0);

        printf("CLIENT] Réponse du serveur : \"%s\"\n", msg_reponse.msg);

        if (! strcmp(msg_reponse.msg, CONST_LOGIN_OK)) {

            printf("CLIENT] Login réussi !\n");

        } 
        else if (! strcmp(msg_reponse.msg, CONST_LOGIN_ECHEC)) {
            printf("CLIENT] ERREUR : Login échoué !\n");
            
            sem_close(SEM_INFO_SERVEUR);
            sem_close(SEM_SCORE);

            return EXIT_FAILURE;
        } 
        else {
            printf("CLIENT] ERREUR : Réponse inconnue !\n");
            
            sem_close(SEM_INFO_SERVEUR);
            sem_close(SEM_SCORE);

            return EXIT_FAILURE;
        }


    // ----------------------------------------------------------------------------------------- //
    #pragma endregion


    #pragma region CONFIGURATION NCURSES
    // --------------------------------------- CONFIGURATION NCURSES --------------------------------------- //

    init_ncurses();

    // ----------------------------------------------------------------------------------------------------- //
    #pragma endregion


    #pragma region MAINLOOP
    // --------------------------------------- RECUPERATION DU MODE --------------------------------------- //

    maj_info_serveur();

    // ---------------------------------------------------------------------------------------------------- //

    bool_t _attente_effectuee = FALSE;  // TRUE/FALSE, on a fait un passage par le mode attente avant le mode partie
    bool_t _premiere_exec = TRUE;  // TRUE/FALSE, permière éxecution d'un mode du client
    etat_serveur_t _dernier_etat = serveur.etat_serveur;
    etat_serveur_t etat;  // etat du serveur stocké en dehors de la variable globale pour pas avoir besoin de mutex lock/unlock
    
    bool_t ready = FALSE;  // joueur prêt ou non dans le lobby ?


    // --------------------------------------- BOUCLE PRINCIPALE --------------------------------------- //

    while (1)
    {

        pthread_mutex_lock(&MUT_INFO_SERVEUR);
        etat = serveur.etat_serveur;
        if (_dernier_etat != etat) {
            _premiere_exec = TRUE;
        }
        _dernier_etat = etat;
        pthread_mutex_unlock(&MUT_INFO_SERVEUR);

        if (etat == ATTENTE) {

            #pragma region ATTENTE
            // --------------------------------------- ATTENTE --------------------------------------- //
            _attente_effectuee = TRUE;  // on est passé par l'attente
            if (_premiere_exec) {
                // code éxecuté la première fois en mode ATTENTE (style qualificatif P1 en automatisme)

                affichage_logo(2, 23);
                
                mvprintw(7, 13, "Waiting room..."); 
                mvprintw(8, 13, "Welcome ");
                attron(COLOR_PAIR(2));
                printw("%s", joueur.pseudo);
                attron(COLOR_PAIR(1));
                printw(" to the game : Tetriis !!");
                mvprintw(10, 13, "Use ");
                attron(A_BOLD);
                printw("[ENTER]");
                attroff(A_BOLD);
                printw(" to be ready or not"); 
                mvprintw(11, 13, "Ready : ");
                attron(COLOR_PAIR(3));
                printw("NO ");
                attron(COLOR_PAIR(1));
                mvprintw(CONST_NB_LIGNES-1, 0, "Tetriis was made by GREBERT Cloe and DUTHOIT Thomas"); 

                refresh();


                ready = FALSE;  // on arrive en attente -> on est pas prêt par défaut


                _premiere_exec = FALSE;
            }

            char touche = getch();  // récupération de l'input

            if (touche == '\n') {  // touche entrée activée, on toggle le fait d'être prêt
                ready = !ready;

                msg_ready_player_t msg_ready;
                msg_ready.type = MSG_TYPE_READY;
                msg_ready.pid_joueur = joueur.pid_client;
                msg_ready.ready = ready;

                msgsnd(BAL_ID, &msg_ready, MSG_SIZEOF(msg_ready_player_t), 0);  // envoie de l'état d'attente dans la BAL

                // on change l'affichage
                mvprintw(11, 13, "Ready : ");
                attron(A_BOLD);
                if (ready) {
                    attron(COLOR_PAIR(2));
                    printw("YES");
                } else {
                    attron(COLOR_PAIR(3));
                    printw("NO ");
                }
                attroff(A_BOLD);
                attron(COLOR_PAIR(1));
            }
            


            // --------------------------------------------------------------------------------------- //
            #pragma endregion

        } 
        else if (etat == PODIUM) {

            #pragma region PODIUM
            // --------------------------------------- PODIUM --------------------------------------- //
            if (_premiere_exec) {
                // code éxecuté la première fois en mode PODIUM (style qualificatif P1 en automatisme)
                
                _attente_effectuee = FALSE;  // on a fini la partie, on doit repasser par l'attente


                _premiere_exec = FALSE;
            }
            
            // --------------------------------------------------------------------------------------- //
            #pragma endregion

        } 
        else if (etat == PARTIE) {

            #pragma region PARTIE
            // --------------------------------------- PARTIE --------------------------------------- //
            if (_attente_effectuee) {
                if (_premiere_exec) {
                    // code éxecuté la première fois en mode PARTIE (style qualificatif P1 en automatisme)

                    clear();

                    mvprintw(0, 0, "EN MODE PARTIE !");

                    refresh();

                    _premiere_exec = FALSE;
                }


                render();


            } else {
                printf("CLIENT] Partie en cours ...\n");

                maj_info_serveur();
            }
            // --------------------------------------------------------------------------------------- //
            #pragma endregion

        }
    }
    

    // ------------------------------------------------------------------------------------------------- //

    #pragma endregion


    sem_close(SEM_INFO_SERVEUR);
    sem_close(SEM_SCORE);

    return EXIT_SUCCESS;
}




#pragma region FONCTIONS
// --------------------------------------- FONCTIONS UTILES --------------------------------------- //

void maj_info_serveur() {  // mets à jour la variable globale "serveur"
    // ATTENTE DISPONIBILITE DE LA SHM PROTEGEE PAR LA SEM
    sem_wait(SEM_INFO_SERVEUR);
    pthread_mutex_lock(&MUT_INFO_SERVEUR);

        // lecture du PID du serveur dans la SHM
        info_serveur_t * info_serveur = shmat(SHM_INFO_SERVEUR, NULL, O_RDONLY);
        
        serveur.pid_serveur = info_serveur->pid_serveur;
        serveur.etat_serveur = info_serveur->etat_serveur;

        shmdt(info_serveur);
    
    pthread_mutex_unlock(&MUT_INFO_SERVEUR);
    sem_post(SEM_INFO_SERVEUR);  // SHM DE NOUVEAU DISPONIBLE -> SEM REMISE
}

// ------------------------------------------------------------------------------------------------ //
#pragma endregion


#pragma region AFFICHAGE
// --------------------------------------- AFFICHAGE --------------------------------------- //


void render() {
    int x_off = 5; 
    int y_off = 3;
    for (int y=0; y<CONST_HAUTEUR_GRILLE_RENDER; y++) {
        mvprintw(y_off+y, x_off, "|");
        mvprintw(y_off+y, x_off+CONST_LARGEUR_GRILLE_RENDER*CONST_LARGEUR_SCALE, "|");
    }
    refresh();
}

// ----------------------------------------------------------------------------------------- //
#pragma endregion






#pragma region DEROUTE
// --------------------------------------- DEROUTE --------------------------------------- //

void deroute(int signal){
    switch(signal){
        case SIGINT : 
            endwin(); 
            printf("CLIENT] SIGINT, fermeture du client\n"); 
            
            sem_close(SEM_INFO_SERVEUR); 
            sem_close(SEM_SCORE);
            exit(EXIT_SUCCESS);  
            break; 
        case SIG_START : 
            
            // SIG_START -> le serveur nous signale de passer de attente à partie
            clear();

            for (int s=3; s>0; s--) {
                for (int p=1; p<4; p++) {
                    affichage_compteur(s, p); 
                    usleep(333333); // 1/3 s  
                }
                usleep(1000000);
            }
            affichage_lancement(); 
            usleep(1000000); // 1 s

            refresh();


            maj_info_serveur();


            break; 
        default :  
            printf("Commande inconnue"); 
            break; 
    }
}

// --------------------------------------------------------------------------------------- //
#pragma endregion 