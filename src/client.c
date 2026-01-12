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
pthread_mutex_t MUT_FIN_TRHEAD_PRET = PTHREAD_MUTEX_INITIALIZER;

// AUTRES
info_serveur_t serveur; // informations du serveur tetriis
login_t joueur;  // contient les informations sur le client actuel
bool_t FIN_THREAD_PRET;  // pour mettre fin au thread "prêt"


// PROTOTYPES
void maj_info_serveur();

void * thread_pret(void * arg);



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
    key_t tok_INFO_SERVEUR = ftok(chemin, CONST_PROJECT_ID); 
    SHM_INFO_SERVEUR = shmget(tok_INFO_SERVEUR, sizeof(info_serveur_t), 0666 | IPC_CREAT); 
    CHECK(SHM_INFO_SERVEUR, "DEBUG ] CLIENT ] Erreur shmget SHM_INFO_SERVEUR"); 

    // SHM SCORECONST_FIC_SHM_SCORE
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_SCORE);
    key_t tok_SCORE = ftok(chemin, CONST_PROJECT_ID); 
    SHM_SCORE = shmget(tok_SCORE, sizeof(score_t), 0666 | IPC_CREAT); 
    CHECK(SHM_SCORE, "DEBUG ] CLIENT ] Erreur shmget SHM_SCORE"); 


    // BOITE AUX LETTRES
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_BAL);
    key_t tok_BAL = ftok(chemin, CONST_PROJECT_ID);
    CHECK(tok_BAL, "DEBUG ] CLIENT ] Erreur ftok BAL");
    BAL_ID = msgget(tok_BAL, 0666);
    CHECK(BAL_ID, "DEBUG ] CLIENT ] Erreur msgget BAL");


    pthread_t TH_PRET;
    // ------------------------------------------------------------------------------------ //
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

    pthread_mutex_lock(&MUT_INFO_SERVEUR);
    pthread_mutex_unlock(&MUT_INFO_SERVEUR);

    // ---------------------------------------------------------------------------------------------------- //

    bool_t _attente_effectuee = FALSE;  // TRUE/FALSE, on a fait un passage par le mode attente avant le mode partie
    bool_t _premiere_exec = TRUE;  // TRUE/FALSE, permière éxecution d'un mode du client
    etat_serveur_t _dernier_etat = serveur.etat_serveur;
    etat_serveur_t etat;  // etat du serveur stocké en dehors de la variable globale pour pas avoir besoin de mutex lock/unlock


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
                mvprintw(10, 13, "Use [ENTER] to be ready or not"); 
                mvprintw(CONST_NB_LIGNES-1, 0, "Tetriis was made by GREBERT Cloe and DUTHOIT Thomas"); 

                refresh();

                _premiere_exec = FALSE;
            }

            getch();  // récupération de l'input


            pthread_create(&TH_PRET, NULL, thread_pret, NULL); 


            // --------------------------------------------------------------------------------------- //
            #pragma endregion

        } 
        else if (etat == PODIUM) {

            #pragma region PODIUM
            // --------------------------------------- PODIUM --------------------------------------- //
            _attente_effectuee = FALSE;  // on a fini la partie, on doit repasser par l'attente
            if (_premiere_exec) {
                // code éxecuté la première fois en mode PODIUM (style qualificatif P1 en automatisme)


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


                    _premiere_exec = FALSE;
                }

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












#pragma region THREADS

void * thread_pret(void * arg) {

    while (1)
    {
        pthread_mutex_lock(&MUT_FIN_TRHEAD_PRET);
        if (FIN_THREAD_PRET) {
            pthread_mutex_unlock(&MUT_FIN_TRHEAD_PRET);
            break;
        }
        pthread_mutex_unlock(&MUT_FIN_TRHEAD_PRET);
    }
    
    pthread_exit(0);
}

#pragma endregion