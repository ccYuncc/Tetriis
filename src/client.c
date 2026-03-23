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
pthread_mutex_t MUT_TETROMINO = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUT_THREAD_PATIE = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUT_NCURSES = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t MUT_SCORE = PTHREAD_MUTEX_INITIALIZER; 

// AUTRES
info_serveur_t serveur; // informations du serveur tetriis
login_t joueur_login;  // contient les informations sur le client actuel
joueur_t joueur;  // contient les informations sur le client actuel + score
int tetrominos[7][16] = {  // differens tetrominos (pièces de tetris) possibles
    {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},  // I
    {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0},  // O
    {0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0},  // S
    {0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},  // Z
    {0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},  // T
    {0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},  // L
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0}   // J
};

int couleurs_tetrominos[7] = {  // correspond à l'id des paires de couleurs de ncurses dans le init
    4, // I : cyan
    6, // O : jaune
    2, // S : vert
    3, // Z : rouge
    5, // T : violet
    3, // L : orange (existe pas donc rouge)
    7, // J : bleu
};

int grille[CONST_HAUTEUR_GRILLE][CONST_LARGEUR_GRILLE] = {0};

// protegee par mutex
int idx_tetr;
int idx_proch_tetr;
int x_tetr;
int y_tetr;
int rot_tetr;

int sup_consecutives = 0;
// ------------------

bool_t  thread_partie_while;


// PROTOTYPES
void maj_info_serveur();
void deroute(int signal);
void premier_render();
void tetromino_render(int idx_tetromino, int x, int y, int rot);
void tetromino_effacer(int idx_tetromino, int x, int y, int rot);
void tetromino_rotation(int tetromino[16], int rot);
void effacer_grille();
int generer_tetr();
void render();
void bordures_render();
bool_t collision_bords();
bool_t collision_bas();
bool_t collision_grille();
bool_t ajouter_tetr_grille();
int supprimer_lignes();
void maj_BAL_score();

void * thread_partie(void * arg);
void * thread_recep(void * arg);


int main(int argc, char **argv){

    #pragma region INIT
    // --------------------------------------- INIT --------------------------------------- //
    #ifdef MODE_TEST
        printf("ATTENTION: Compilé en mode TEST !\n");

        // variables utiles au mode test:
        int data = 0;
    #endif  // MODE_TEST

    pthread_t TH_PARTIE;
    pthread_t TH_ECOUTE;
    score_t * info_score; 

    int rows, cols;

    // OUVERTURE DES SEMAPHORES
    SEM_INFO_SERVEUR = sem_open(CONST_SEM_NOM_INFO_SERVEUR, 0); 
    SEM_SCORE = sem_open(CONST_SEM_NOM_SCORE, 0); 
    
    // TEST ERREUR DE CREATION
    if (SEM_INFO_SERVEUR == SEM_FAILED || SEM_SCORE == SEM_FAILED  ) {
        printf("You need to start the server before starting a client !\n");
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

    // SHM SCORE
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
    CHECK(sigaction(SIG_END, &newact, NULL), "Problème sigaction");

    // --------------------------------------------------------------------------------------- //
    #pragma endregion


    #pragma region ACQUISITION PID SERVEUR    
    // --------------------------------------- ACQUISITION PID SERVEUR  --------------------------------------- //
    
    maj_info_serveur();

    pthread_mutex_lock(&MUT_INFO_SERVEUR);
    printf("CLIENT] Server PID : %d\n", serveur.pid_serveur);
    pthread_mutex_unlock(&MUT_INFO_SERVEUR);

    // -------------------------------------------------------------------------------------------------------- //
    #pragma endregion

    #pragma region CONNEXION
    // --------------------------------------- CONNEXION --------------------------------------- //

        joueur_login.fermer = FALSE;  // on veut ouvrir une connexion

        // ACQUISITION DU PSEUDO

        if (argc >= 2) {  // lancé avec ./bin/client.exe pseudo

            strncpy(joueur_login.pseudo, argv[1], CONST_LONGUEUR_PSEUDO);

        } else {  // lancé avec ./bin/client.exe
            char buff_pseudo_temp[CONST_LONGUEUR_PSEUDO+1];
            printf("CLIENT] Pseudo (%d char. max) : ", CONST_LONGUEUR_PSEUDO);
            fgets(buff_pseudo_temp, CONST_LONGUEUR_PSEUDO+1, stdin);
            buff_pseudo_temp[strlen(joueur_login.pseudo)-1] = '\0';
            strncpy(joueur_login.pseudo, buff_pseudo_temp, CONST_LONGUEUR_PSEUDO);

            while (strcmp(joueur_login.pseudo, "") == 0) {
                printf("CLIENT] Please enter your pseudo... Pseudo (%d char. max) : ", CONST_LONGUEUR_PSEUDO);
                fgets(buff_pseudo_temp, CONST_LONGUEUR_PSEUDO+1, stdin);
                buff_pseudo_temp[strlen(joueur_login.pseudo)-1] = '\0';
                strncpy(joueur_login.pseudo, buff_pseudo_temp, CONST_LONGUEUR_PSEUDO);
            }
        }

        // ACQUISITION DU PID
        joueur_login.pid_client = getpid();
        printf("CLIENT] client's PID : %d\n", joueur_login.pid_client);

        msg_login_t msg_login;
        msg_login.type = MSG_TYPE_LOGIN;
        msg_login.msg = joueur_login;
        
        msgsnd(BAL_ID, &msg_login, MSG_SIZEOF(msg_login_t), 0);


        msg_reponse_serveur_t msg_reponse;

        msgrcv(BAL_ID, &msg_reponse, MSG_SIZEOF(msg_reponse_serveur_t), joueur_login.pid_client, 0);

        printf("CLIENT] Server anwser : \"%s\"\n", msg_reponse.msg);

        if (! strcmp(msg_reponse.msg, CONST_LOGIN_OK)) {

            printf("CLIENT] Login sucessful !\n");

        } 
        else if (! strcmp(msg_reponse.msg, CONST_LOGIN_ECHEC)) {
            printf("CLIENT] ERROR : Login failed !\n");
            
            sem_close(SEM_INFO_SERVEUR);
            sem_close(SEM_SCORE);

            return EXIT_FAILURE;
        } 
        else {
            printf("CLIENT] ERREUR : unknown anwser !\n");
            
            sem_close(SEM_INFO_SERVEUR);
            sem_close(SEM_SCORE);

            return EXIT_FAILURE;
        }

        joueur.login_joueur = joueur_login;
        joueur.score = 0;


    // ----------------------------------------------------------------------------------------- //
    #pragma endregion


    #pragma region CONFIGURATION NCURSES
    // --------------------------------------- CONFIGURATION NCURSES --------------------------------------- //

    pthread_mutex_lock(&MUT_NCURSES);

        init_ncurses();

    pthread_mutex_unlock(&MUT_NCURSES);

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
    
    bool_t ready = FALSE;  // joueur_login prêt ou non dans le lobby ?
    bool_t mort = FALSE;  // le joueur_login a perdu mais la partie n'est pas finie

    srand(joueur_login.pid_client);  // pour que chaque joueur_login ait des pièces différentes, on utilise leurs PID (unique) pour avoir une seed pour rand

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

                pthread_mutex_lock(&MUT_THREAD_PATIE);

                    thread_partie_while = FALSE;

                pthread_mutex_unlock(&MUT_THREAD_PATIE);

                pthread_mutex_lock(&MUT_NCURSES);

                    clear();

                    getmaxyx(stdscr, rows, cols);

                    affichage_logo(2, ((cols-34)/2));
                    
                    mvprintw(7, ((cols-34)/2), "Waiting room..."); 
                    mvprintw(8, ((cols-34)/2), "Welcome ");
                    attron(COLOR_PAIR(2));
                    printw("%s", joueur_login.pseudo);
                    attron(COLOR_PAIR(1));
                    printw(" to the game : Tetriis !!");
                    mvprintw(10, ((cols-34)/2), "Use ");
                    attron(A_BOLD);
                    printw("[ENTER]");
                    attroff(A_BOLD);
                    printw(" to be ready or not"); 
                    mvprintw(11, ((cols-34)/2), "Ready : ");
                    attron(COLOR_PAIR(3));
                    printw("NO ");
                    attron(COLOR_PAIR(1));
                    mvprintw(CONST_NB_LIGNES-1, 0, "Tetriis was made by GREBERT Cloe and DUTHOIT Thomas"); 

                    refresh();

                pthread_mutex_unlock(&MUT_NCURSES);


                ready = FALSE;  // on arrive en attente -> on est pas prêt par défaut


                _premiere_exec = FALSE;
            }

            char touche = getch();  // récupération de l'input

            if (touche == '\n') {  // touche entrée activée, on toggle le fait d'être prêt
                ready = !ready;

                msg_ready_player_t msg_ready;
                msg_ready.type = MSG_TYPE_READY;
                msg_ready.pid_joueur = joueur_login.pid_client;
                msg_ready.ready = ready;

                msgsnd(BAL_ID, &msg_ready, MSG_SIZEOF(msg_ready_player_t), 0);  // envoie de l'état d'attente dans la BAL
                getmaxyx(stdscr, rows, cols);

                // on change l'affichage
                pthread_mutex_lock(&MUT_NCURSES);
                    mvprintw(11, ((cols-34)/2), "Ready : ");
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
                pthread_mutex_unlock(&MUT_NCURSES);
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

                pthread_mutex_lock(&MUT_THREAD_PATIE);

                    thread_partie_while = FALSE;

                pthread_mutex_unlock(&MUT_THREAD_PATIE);

                _premiere_exec = FALSE;
            }

            pthread_mutex_lock(&MUT_SCORE); 
                            
                sem_wait(SEM_SCORE);           
            
                    info_score = shmat(SHM_SCORE, NULL, 0); 

                    pthread_mutex_lock(&MUT_NCURSES);

                        clear();
                    
                        affichage_podium(info_score); 

                    pthread_mutex_unlock(&MUT_NCURSES);

                    shmdt(info_score); 
                    
                sem_post(SEM_SCORE); 

            pthread_mutex_unlock(&MUT_SCORE); 
            usleep(500000); // 50ms
            maj_info_serveur();
            
            
            // --------------------------------------------------------------------------------------- //
            #pragma endregion

        } 
        else if (etat == PARTIE) {

            #pragma region PARTIE
            // --------------------------------------- PARTIE --------------------------------------- //
            if (_attente_effectuee) {

                #ifndef MODE_TEST  // section en mode normal
                
                    if (_premiere_exec) {
                        // code éxecuté la première fois en mode PARTIE (style qualificatif P1 en automatisme)

                        pthread_mutex_lock(&MUT_TETROMINO);

                        x_tetr = CONST_LARGEUR_GRILLE / 2 - 2;
                        y_tetr = -4;

                        idx_tetr = generer_tetr();
                        idx_proch_tetr = generer_tetr();

                        rot_tetr = 0;

                        memset(grille, 0, sizeof(grille));

                        pthread_mutex_unlock(&MUT_TETROMINO);

                        pthread_mutex_lock(&MUT_NCURSES);

                            clear();

                            premier_render();  // pour afficher les éléments statiques du GUI

                        pthread_mutex_unlock(&MUT_NCURSES);

                        pthread_mutex_lock(&MUT_THREAD_PATIE);

                            thread_partie_while = TRUE;  // on "active" le thread qui fait tomber les pièces

                        pthread_mutex_unlock(&MUT_THREAD_PATIE);
                        pthread_create(&TH_PARTIE, NULL, thread_partie, NULL);  // puis une fois la condition activée on crée le thread
                        pthread_detach(TH_PARTIE);  // on le détache pour ne pas avoir à join le thread pour qu'il libère ses resources

                        pthread_create(&TH_ECOUTE, NULL, thread_recep, NULL);  // puis une fois la condition activée on crée le thread
                        pthread_detach(TH_ECOUTE);  // on le détache pour ne pas avoir à join le thread pour qu'il libère ses resources

                        

                        pthread_mutex_lock(&MUT_SCORE); 

                        joueur.score = 0;  // reset du score

                        pthread_mutex_unlock(&MUT_SCORE); 


                        mort = FALSE;

                        
                        _premiere_exec = FALSE;
                    }

                    if (!mort) {  // encore en train de jouer              
                        
                        
                        timeout(50); // le getch() est bloquant pendant 50ms...
                        char touche = getch();
                        
                        
                        if (touche == 'a') {  // gauche
                            pthread_mutex_lock(&MUT_TETROMINO);

                            pthread_mutex_lock(&MUT_NCURSES);
                                tetromino_effacer(idx_tetr, x_tetr + CONST_X_OFF_GRILLE, y_tetr + CONST_Y_OFF_GRILLE, rot_tetr);
                            pthread_mutex_unlock(&MUT_NCURSES);

                            x_tetr--;
                            if (collision_bords() || collision_grille()) {
                                x_tetr ++;
                            }


                            pthread_mutex_unlock(&MUT_TETROMINO);
                        }
                        else if (touche == 'e') {  // droite
                            pthread_mutex_lock(&MUT_TETROMINO);
                            
                            pthread_mutex_lock(&MUT_NCURSES);
                                tetromino_effacer(idx_tetr, x_tetr + CONST_X_OFF_GRILLE, y_tetr + CONST_Y_OFF_GRILLE, rot_tetr);
                            pthread_mutex_unlock(&MUT_NCURSES);

                            x_tetr++;
                            if (collision_bords() || collision_grille()) {
                                x_tetr --;
                            }
                        
                            pthread_mutex_unlock(&MUT_TETROMINO);
                        }
                        else if (touche == 'z') {  // rotation
                            pthread_mutex_lock(&MUT_TETROMINO);

                            pthread_mutex_lock(&MUT_NCURSES);
                                tetromino_effacer(idx_tetr, x_tetr + CONST_X_OFF_GRILLE, y_tetr + CONST_Y_OFF_GRILLE, rot_tetr);
                            pthread_mutex_unlock(&MUT_NCURSES);
                            
                            rot_tetr = (rot_tetr+1)%4;
                            if (collision_bords() || collision_grille()) {
                                rot_tetr = (rot_tetr+3)%4;  // +3 plutot que -1 pour évouter le fait que -1%4 donne -1
                            }
                        
                            pthread_mutex_unlock(&MUT_TETROMINO);
                        }
                        else if (touche == 's' ) {
                            pthread_mutex_lock(&MUT_TETROMINO);
                            #pragma region GRAVITE
                            // --------------------------------------- GRAVITE --------------------------------------- //
                            
                            pthread_mutex_lock(&MUT_NCURSES);
                                tetromino_effacer(idx_tetr, x_tetr + CONST_X_OFF_GRILLE, y_tetr + CONST_Y_OFF_GRILLE, rot_tetr);
                                bordures_render();
                            pthread_mutex_unlock(&MUT_NCURSES);

                            y_tetr++;  // on descend la piece

                            if (collision_bas() || collision_grille()) {
                                y_tetr--;

                                if (ajouter_tetr_grille()) {  // ajout impossible

                                    msg_game_player_t msg_game_player;
                                    msg_game_player.type = MSG_TYPE_GAME;
                                    msg_game_player.pid_joueur = joueur_login.pid_client;
                                    msg_game_player.type_msg = GAME_MSG_DEATH;

                                    msgsnd(BAL_ID, &msg_game_player, MSG_SIZEOF(msg_game_player_t), 0);  // envoie du message dans la BAL

                                    pthread_mutex_lock(&MUT_THREAD_PATIE);

                                    thread_partie_while = FALSE;

                                    pthread_mutex_unlock(&MUT_THREAD_PATIE);
                                } else {  // ajout possible

                                    // augmenter le score
                                    pthread_mutex_lock(&MUT_SCORE); 

                                    if (idx_tetr == 0 || idx_tetr == 1) {  // O ou I
                                        joueur.score += 5;
                                    } else if (idx_tetr == 2 || idx_tetr == 3 || idx_tetr == 5 || idx_tetr == 6) {  // S Z L ou J
                                        joueur.score += 10;
                                    } else {  // T
                                        joueur.score += 15;
                                    }
                                    sem_wait(SEM_SCORE); 
                                    maj_BAL_score();
                                    sem_post(SEM_SCORE); 

                                    pthread_mutex_unlock(&MUT_SCORE); 

                                    // reset pièce
                                    x_tetr = CONST_LARGEUR_GRILLE / 2 - 2;
                                    y_tetr = -4;

                                    idx_tetr = idx_proch_tetr;
                                    idx_proch_tetr = generer_tetr();

                                    rot_tetr = 0;
                                }
                            }

                            // --------------------------------------------------------------------------------------- //
                            #pragma endregion

                            #pragma region LIGNES COMPLETES
                            // --------------------------------------- LIGNES COMPLETES --------------------------------------- //
                            int lignes_sup = supprimer_lignes();
                            if (lignes_sup > 0) {
                                pthread_mutex_lock(&MUT_NCURSES);
                                    effacer_grille();
                                pthread_mutex_unlock(&MUT_NCURSES);

                                sup_consecutives += lignes_sup;

                                // augmenter le score
                                pthread_mutex_lock(&MUT_SCORE); 

                                if (sup_consecutives == 1) {
                                    joueur.score += 20;
                                }
                                else if (sup_consecutives == 2) {
                                    joueur.score += 50;
                                }
                                else if (sup_consecutives == 3) {
                                    joueur.score += 100;
                                }
                                else {  // 4 lignes ou +
                                    joueur.score += 300;
                                }
                                sem_wait(SEM_SCORE); 
                                maj_BAL_score();
                                sem_post(SEM_SCORE); 

                                pthread_mutex_unlock(&MUT_SCORE); 

                            } else {
                                
                                if (sup_consecutives >= CONST_MIN_POUR_BONUS) {  // on vient d'arrêter de supprimer des lignes
                                        msg_game_player_t msg_game_player;
                                        msg_game_player.type = MSG_TYPE_GAME;
                                        msg_game_player.pid_joueur = joueur_login.pid_client;
                                        msg_game_player.type_msg = GAME_MSG_LINE;

                                        msgsnd(BAL_ID, &msg_game_player, MSG_SIZEOF(msg_game_player_t), 0);  // envoie du message dans la BAL
                                }
                                
                                sup_consecutives = 0;
                            }
                            // ------------------------------------------------------------------------------------------------ //
                            #pragma endregion


                            pthread_mutex_unlock(&MUT_TETROMINO);
                        }
                        
                        pthread_mutex_lock(&MUT_THREAD_PATIE);

                            if (!mort && !thread_partie_while) {
                                // mort pas eéncore détectée mais le thread ne s'éxécute plus -> on viens de mort
                                pthread_mutex_lock(&MUT_NCURSES);
                                    clear();

                                    getmaxyx(stdscr, rows, cols);
                                    
                                    affichage_logo(2, ((cols-34)/2));
                    
                                    mvprintw(8, ((cols-34)/2), "GAME OVER !");
                                    mvprintw(CONST_NB_LIGNES-1, 0, "Tetriis was made by GREBERT Cloe and DUTHOIT Thomas"); 

                                    refresh();
                                pthread_mutex_unlock(&MUT_NCURSES);
                            }

                            mort = !thread_partie_while;  // si le thread s'éxécute -> on est pas mort

                        pthread_mutex_unlock(&MUT_THREAD_PATIE);

                            if (mort) {
                                continue;  // on skip le render et le refresh
                            }


                        // maj de l'affuchage après la logique finie
                        pthread_mutex_lock(&MUT_NCURSES);
                        pthread_mutex_lock(&MUT_SCORE); 
                            render();
                            refresh();
                        pthread_mutex_unlock(&MUT_SCORE);
                        pthread_mutex_unlock(&MUT_NCURSES);
                    } else {
                        // on est mort, on ne fait rien car on attent le "SIG_END" du serveur*
                        maj_info_serveur();
                    }

                #else  // section en mode test
                    if (_premiere_exec) {
                        // code éxecuté la première fois en mode PARTIE (style qualificatif P1 en automatisme)

                        clear();
                        
                        _premiere_exec = FALSE;
                    }
                    mvprintw(2, 2, "[TEST] data a envoyer au serveur : ");
                    refresh();
                    char c = getch();
                    if (c != '\n') {
                        if (c >= '0' && c <= '9') {
                            data = c - '0';
                            mvprintw(2, 2, "[TEST] data a envoyer au serveur : %d", data);
                        }
                    } else {  // envoi

                        msg_game_player_t msg_game_player;
                        msg_game_player.type = MSG_TYPE_GAME;
                        msg_game_player.pid_joueur = joueur_login.pid_client;
                        msg_game_player.type_msg = data;

                        msgsnd(BAL_ID, &msg_game_player, MSG_SIZEOF(msg_game_player_t), 0);  // envoie du message dans la BAL

                        mvprintw(4, 2, "[TEST] dernier message envoye : %d", data);
                    }

                #endif  // MODE_TEST
            } else {
                //printf("CLIENT] Partie en cours ...\n");
                pthread_mutex_lock(&MUT_NCURSES);
                    clear();
                    getmaxyx(stdscr, rows, cols);
                    affichage_logo(2, 23);
                    
                    mvprintw(8, ((cols-34)/2), "Welcome ");
                    attron(COLOR_PAIR(2));
                    printw("%s", joueur_login.pseudo);
                    attron(COLOR_PAIR(1));
                    printw(" to the game : Tetriis !!");
                    mvprintw(7, ((cols-34)/2), "Game in progress ..."); 
                    mvprintw(CONST_NB_LIGNES-1, 0, "Tetriis was made by GREBERT Cloe and DUTHOIT Thomas"); 

                    refresh();
                pthread_mutex_unlock(&MUT_NCURSES);

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

void tetromino_rotation(int tetromino[16], int rot) {

    int tetr_modif[16];

    // voir le canva pour le graphique (c"est plus clair)
    // https://www.canva.com/design/DAG96A7aaU0/DesScJkOnfjSMdZ-axWK7w/edit

    switch (rot) {
        case 0:
            // 0°
            // pas de rotation
            break;
        case 1:
            // 90°
            for (int y=0; y<4; y++) {
                for (int x=0; x<4; x++) {
                    tetr_modif[y*4+x] = tetromino[y + (3-x)*4];
                }
            }

            memcpy(tetromino, tetr_modif, 16*sizeof(int));

            break;
        case 2:
            // 180°
            for (int i=0; i<16; i++) {
                tetr_modif[i] = tetromino[15-i];
                
            }

            memcpy(tetromino, tetr_modif, 16*sizeof(int));

            break;
        case 3:
            // 270°
            for (int y=0; y<4; y++) {
                for (int x=0; x<4; x++) {
                    tetr_modif[y*4+x] = tetromino[(3-y) + x*4];
                }
            }

            memcpy(tetromino, tetr_modif, 16*sizeof(int));

            break;
    }

    return;
}

int generer_tetr() {
    return rand() % 7;
}

bool_t collision_bords() {
    int tetr[16];
    memcpy(tetr, tetrominos[idx_tetr], 16*sizeof(int));

    tetromino_rotation(tetr, rot_tetr);


    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {

            if (tetr[y*4+x]) {

                int grille_x = x_tetr + x;

                if (grille_x < 0 || grille_x >= CONST_LARGEUR_GRILLE) {
                    return TRUE;  // en dehors des bords -> collision
                }
            }
        }
    }

    return FALSE;
}

bool_t collision_bas() {
    int tetr[16];
    memcpy(tetr, tetrominos[idx_tetr], 16*sizeof(int));

    tetromino_rotation(tetr, rot_tetr);


    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {

            if (tetr[y*4+x]) {

                int grille_y = y_tetr + y;

                if (grille_y >= CONST_HAUTEUR_GRILLE) {
                    return TRUE;  // tout en bas -> collision
                }
            }
        }
    }

    return FALSE;
}

bool_t collision_grille() {
    int tetr[16];
    memcpy(tetr, tetrominos[idx_tetr], 16*sizeof(int));

    tetromino_rotation(tetr, rot_tetr);

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {

            if (tetr[y*4+x]) {

                int grille_x = x_tetr + x;
                int grille_y = y_tetr + y;

                if (grille_x >= 0 && grille_x < CONST_LARGEUR_GRILLE  && grille_y >= 0 && grille_y < CONST_HAUTEUR_GRILLE) {
                    if (grille[grille_y][grille_x] != 0) return TRUE;
                } 
            }
        }
    }

    return FALSE;
}

bool_t ajouter_tetr_grille() {
    int tetr[16];
    memcpy(tetr, tetrominos[idx_tetr], 16*sizeof(int));

    tetromino_rotation(tetr, rot_tetr);

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {

            if (tetr[y*4+x]) {

                int grille_x = x_tetr + x;
                int grille_y = y_tetr + y;

                if (grille_x < 0 || grille_x >= CONST_LARGEUR_GRILLE || grille_y < 0 || grille_y >= CONST_HAUTEUR_GRILLE) {
                    return TRUE;  // en dehors des bords -> impossible de fusionner le tetromino
                } else {
                    grille[grille_y][grille_x] = couleurs_tetrominos[idx_tetr];  // on met de la bonne couleur dans la grille
                }
            }
        }
    }

    return FALSE;
}

int supprimer_lignes() {
    int ligne_effacees = 0;

    for (int i=CONST_HAUTEUR_GRILLE-1; i>=0; i--) {
        bool_t flag = TRUE;
        for (int x=0; x<CONST_LARGEUR_GRILLE; x++) {
            if (grille[i][x] == 0) {
                flag = FALSE;
                break;
            }
        }
        if (flag) {  // ligne complète
            for (int y=i; y>=1; y--) {
                memcpy(grille[y], grille[y-1], sizeof(grille[0]));
            }


            ligne_effacees ++;
        }
    }

    return ligne_effacees;
}


void maj_BAL_score() {

    bool_t en_podium = FALSE;


    score_t *info_score;

    info_score = shmat(SHM_SCORE, NULL, 0); 

    if (
        info_score->premier.login_joueur.pid_client == joueur.login_joueur.pid_client
        || info_score->deuxieme.login_joueur.pid_client == joueur.login_joueur.pid_client
        || info_score->troisieme.login_joueur.pid_client == joueur.login_joueur.pid_client
    ) {
        en_podium = TRUE;
    }


    if (info_score->premier.score < joueur.score) {
        if (en_podium && info_score->premier.login_joueur.pid_client == joueur.login_joueur.pid_client) {
            /*
            1: moi
            2: 2ème
            3: 3ème

            devient 

            1: moi
            2: 2ème
            3: 3ème
            */
            info_score->premier.score = joueur.score;
        } else if (en_podium && info_score->deuxieme.login_joueur.pid_client == joueur.login_joueur.pid_client) {
            /*
            1: 1er
            2: moi
            3: 3ème

            devient 

            1: moi
            2: 1er
            3: 3ème
            */

            // 1er -> 2ème
            info_score->deuxieme.score = info_score->premier.score;
            info_score->deuxieme.login_joueur.pid_client = info_score->premier.login_joueur.pid_client;
            strcpy(info_score->deuxieme.login_joueur.pseudo, info_score->premier.login_joueur.pseudo);

            // moi -> 1er
            info_score->premier.score = joueur.score;
            info_score->premier.login_joueur.pid_client = joueur.login_joueur.pid_client;
            strcpy(info_score->premier.login_joueur.pseudo, joueur.login_joueur.pseudo);


        } else if (en_podium && info_score->troisieme.login_joueur.pid_client == joueur.login_joueur.pid_client) {
            /*
            1: 1er
            2: 2ème
            3: moi

            devient 

            1: moi
            2: 1er
            3: 2ème
            */

            // 2ème -> 3ème
            info_score->troisieme.score = info_score->deuxieme.score;
            info_score->troisieme.login_joueur.pid_client = info_score->deuxieme.login_joueur.pid_client;
            strcpy(info_score->troisieme.login_joueur.pseudo, info_score->deuxieme.login_joueur.pseudo);
            // 1er -> 2ème
            info_score->deuxieme.score = info_score->premier.score;
            info_score->deuxieme.login_joueur.pid_client = info_score->premier.login_joueur.pid_client;
            strcpy(info_score->deuxieme.login_joueur.pseudo, info_score->premier.login_joueur.pseudo);
            // moi -> 1er
            info_score->premier.score = joueur.score;
            info_score->premier.login_joueur.pid_client = joueur.login_joueur.pid_client;
            strcpy(info_score->premier.login_joueur.pseudo, joueur.login_joueur.pseudo);
        } else {
            /*
            1: 1er
            2: 2ème
            3: 3ème

            devient 

            1: moi
            2: 1er
            3: 2ème
            */

            // 2ème -> 3ème
            info_score->troisieme.score = info_score->deuxieme.score;
            info_score->troisieme.login_joueur.pid_client = info_score->deuxieme.login_joueur.pid_client;
            strcpy(info_score->troisieme.login_joueur.pseudo, info_score->deuxieme.login_joueur.pseudo);
            // 1er -> 2ème
            info_score->deuxieme.score = info_score->premier.score;
            info_score->deuxieme.login_joueur.pid_client = info_score->premier.login_joueur.pid_client;
            strcpy(info_score->deuxieme.login_joueur.pseudo, info_score->premier.login_joueur.pseudo);
            // moi -> 1er
            info_score->premier.score = joueur.score;
            info_score->premier.login_joueur.pid_client = joueur.login_joueur.pid_client;
            strcpy(info_score->premier.login_joueur.pseudo, joueur.login_joueur.pseudo);
        }
        
    } else if (info_score->deuxieme.score < joueur.score
                && !(info_score->premier.login_joueur.pid_client == joueur.login_joueur.pid_client) ) {

        if (en_podium && info_score->deuxieme.login_joueur.pid_client == joueur.login_joueur.pid_client) {
            /*
            1: 1er
            2: moi
            3: 3ème

            devient 

            1: 1er
            2: moi
            3: 3ème
            */
            info_score->deuxieme.score = joueur.score;
        } else if (en_podium && info_score->troisieme.login_joueur.pid_client == joueur.login_joueur.pid_client) {
            /*
            1: 1er
            2: 2ème
            3: moi

            devient 

            1: 1er
            2: moi
            3: 2ème
            */

            // 2ème -> 3ème
            info_score->troisieme.score = info_score->deuxieme.score;
            info_score->troisieme.login_joueur.pid_client = info_score->deuxieme.login_joueur.pid_client;
            strcpy(info_score->troisieme.login_joueur.pseudo, info_score->deuxieme.login_joueur.pseudo);
            // moi -> 2ème
            info_score->deuxieme.score = joueur.score;
            info_score->deuxieme.login_joueur.pid_client = joueur.login_joueur.pid_client;
            strcpy(info_score->deuxieme.login_joueur.pseudo, joueur.login_joueur.pseudo);
        } else {
            /*
            1: 1er
            2: 2ème
            3: 3ème

            devient 

            1: 1er
            2: moi
            3: 2ème
            */

            // 2ème -> 3ème
            info_score->troisieme.score = info_score->deuxieme.score;
            info_score->troisieme.login_joueur.pid_client = info_score->deuxieme.login_joueur.pid_client;
            strcpy(info_score->troisieme.login_joueur.pseudo, info_score->deuxieme.login_joueur.pseudo);
            // moi -> 2ème
            info_score->deuxieme.score = joueur.score;
            info_score->deuxieme.login_joueur.pid_client = joueur.login_joueur.pid_client;
            strcpy(info_score->deuxieme.login_joueur.pseudo, joueur.login_joueur.pseudo);
        }

    } else if (info_score->troisieme.score < joueur.score
                && !(info_score->premier.login_joueur.pid_client == joueur.login_joueur.pid_client) 
                && !(info_score->deuxieme.login_joueur.pid_client == joueur.login_joueur.pid_client) ) {

        if (en_podium && info_score->troisieme.login_joueur.pid_client == joueur.login_joueur.pid_client) {
            /*
            1: 1er
            2: 2ème
            3: moi

            devient 

            1: 1er
            2: 2ème
            3: moi
            */
            info_score->troisieme.score = joueur.score;
        } else {
            /*
            1: 1er
            2: 2ème
            3: 3ème

            devient 

            1: 1er
            2: 2ème
            3: moi
            */

            // moi -> 3ème
            info_score->troisieme.score = joueur.score;
            info_score->troisieme.login_joueur.pid_client = joueur.login_joueur.pid_client;
            strcpy(info_score->troisieme.login_joueur.pseudo, joueur.login_joueur.pseudo);
        }


    }

    shmdt(info_score); 
}


// ------------------------------------------------------------------------------------------------ //
#pragma endregion


#pragma region AFFICHAGE
// --------------------------------------- AFFICHAGE --------------------------------------- //


void premier_render() {
    // affichage du plateau
    bordures_render();

    mvprintw(CONST_Y_OFF_GRILLE-1, CONST_X_OFF_GRILLE + CONST_LARGEUR_GRILLE + 5, "Next :");

    mvprintw(CONST_Y_OFF_GRILLE+5, CONST_X_OFF_GRILLE + CONST_LARGEUR_GRILLE + 5, "Score :");

    mvprintw(CONST_Y_OFF_GRILLE+8, CONST_X_OFF_GRILLE + CONST_LARGEUR_GRILLE + 5, "Podium :");


    mvprintw(CONST_NB_LIGNES-1, 0, "Tetriis was made by GREBERT Cloe and DUTHOIT Thomas"); 

}


void tetromino_render(int idx_tetromino, int x, int y, int rot) {
    int tetr[16];
    memcpy(tetr, tetrominos[idx_tetromino], 16*sizeof(int));

    tetromino_rotation(tetr, rot);


    attron(COLOR_PAIR(couleurs_tetrominos[idx_tetromino]));
    for (int _x =0; _x < 4; _x++) {
        for (int _y=0; _y<4; _y++) {
            if (tetr[_y*4+_x]) {
                mvprintw(y+_y,x+_x, "#");
            }
        }
    }
    attron(COLOR_PAIR(1));
}

void tetromino_effacer(int idx_tetromino, int x, int y, int rot) {  // comme tetromino_render mais on affiche des espaces à la place
    int tetr[16];
    memcpy(tetr, tetrominos[idx_tetromino], 16*sizeof(int));

    tetromino_rotation(tetr, rot);

    for (int _x =0; _x < 4; _x++) {
        for (int _y=0; _y<4; _y++) {
            if (tetr[_y*4+_x]) {
                mvprintw(y+_y,x+_x, " ");
            }
        }
    }
}

void render() {

    effacer_grille();

    // pièce actuelle
    tetromino_render(idx_tetr, x_tetr + CONST_X_OFF_GRILLE, y_tetr + CONST_Y_OFF_GRILLE, rot_tetr);

    // pièces figées
    for (int x =0; x < CONST_LARGEUR_GRILLE; x++) {
        for (int y=0; y<CONST_HAUTEUR_GRILLE; y++) {
            if (grille[y][x] != 0) {
                attron(COLOR_PAIR(grille[y][x]));
                mvprintw(CONST_Y_OFF_GRILLE+y, CONST_X_OFF_GRILLE+x, "#");
            }
        }
    }
    attron(COLOR_PAIR(1));

    // bordures
    bordures_render();

    // prochaine pièce (efface puis affiche pour eviter les chevauchements)
    mvprintw(CONST_Y_OFF_GRILLE  , CONST_X_OFF_GRILLE + CONST_LARGEUR_GRILLE + 5, "    ");
    mvprintw(CONST_Y_OFF_GRILLE+1, CONST_X_OFF_GRILLE + CONST_LARGEUR_GRILLE + 5, "    ");
    mvprintw(CONST_Y_OFF_GRILLE+2, CONST_X_OFF_GRILLE + CONST_LARGEUR_GRILLE + 5, "    ");
    mvprintw(CONST_Y_OFF_GRILLE+3, CONST_X_OFF_GRILLE + CONST_LARGEUR_GRILLE + 5, "    ");
    tetromino_render(idx_proch_tetr, CONST_X_OFF_GRILLE + CONST_LARGEUR_GRILLE + 5, CONST_Y_OFF_GRILLE, 0);

    mvprintw(CONST_Y_OFF_GRILLE+6, CONST_X_OFF_GRILLE + CONST_LARGEUR_GRILLE + 5, "%d", joueur.score);


    score_t *info_score;

    info_score = shmat(SHM_SCORE, NULL, 0); 
    if (info_score->premier.score > 0) {
        mvprintw(CONST_Y_OFF_GRILLE+9, CONST_X_OFF_GRILLE + CONST_LARGEUR_GRILLE + 5, "1st: %s (%d)            ", info_score->premier.login_joueur.pseudo, info_score->premier.score);
    }
    if (info_score->deuxieme.score > 0) {
        mvprintw(CONST_Y_OFF_GRILLE+10, CONST_X_OFF_GRILLE + CONST_LARGEUR_GRILLE + 5, "2nd: %s (%d)            ", info_score->deuxieme.login_joueur.pseudo, info_score->deuxieme.score);
    }
    if (info_score->troisieme.score > 0) {
        mvprintw(CONST_Y_OFF_GRILLE+11, CONST_X_OFF_GRILLE + CONST_LARGEUR_GRILLE + 5, "3rd: %s (%d)            ", info_score->troisieme.login_joueur.pseudo, info_score->troisieme.score);
    }

    shmdt(info_score);





    int rows, cols;

    getmaxyx(stdscr, rows, cols);

    if (rows < CONST_NB_LIGNES || cols < CONST_NB_COLONNES) {
        attron(COLOR_PAIR(3));
        mvprintw(0, 0, "TERMINAL WINDOW IS TO SMALL !");
        attron(COLOR_PAIR(1));
    } else {
        mvprintw(0, 0, "                             ");  // effacer le message
        tetromino_render(idx_tetr, x_tetr + CONST_X_OFF_GRILLE, y_tetr + CONST_Y_OFF_GRILLE, rot_tetr); // redessiner la pièce actuelle si elle s'est faite effacer par la ligne d'au dessus
    }
}


void effacer_grille() {
    for (int x =0; x < CONST_LARGEUR_GRILLE; x++) {
        for (int y=0; y<CONST_HAUTEUR_GRILLE; y++) {
            mvprintw(CONST_Y_OFF_GRILLE+y, CONST_X_OFF_GRILLE+x, " ");
        }
    }
}

void bordures_render() {
    int x_off = CONST_X_OFF_GRILLE; 
    int y_off = CONST_Y_OFF_GRILLE;
    for (int y=0; y<CONST_HAUTEUR_GRILLE; y++) {
        mvprintw(y_off+y, x_off-1, "|");
        mvprintw(y_off+y, x_off+CONST_LARGEUR_GRILLE, "|");
    }
    for (int x=0; x<CONST_LARGEUR_GRILLE; x++) {
        mvprintw(y_off+CONST_HAUTEUR_GRILLE, x_off+x, "=");
    }
    mvprintw(y_off+CONST_HAUTEUR_GRILLE, x_off-1, "+");
    mvprintw(y_off+CONST_HAUTEUR_GRILLE, x_off+CONST_LARGEUR_GRILLE, "+");
}


// ----------------------------------------------------------------------------------------- //
#pragma endregion


#pragma region THREAD
// --------------------------------------- THREAD --------------------------------------- //
void * thread_partie(void * arg) {

    bool_t activ = TRUE;

    while (activ) {

        usleep(CONST_GRAVITE_SLEEP_US);  // delai

        pthread_mutex_lock(&MUT_TETROMINO);

            #pragma region GRAVITE
            // --------------------------------------- GRAVITE --------------------------------------- //
            
            pthread_mutex_lock(&MUT_NCURSES);
                tetromino_effacer(idx_tetr, x_tetr + CONST_X_OFF_GRILLE, y_tetr + CONST_Y_OFF_GRILLE, rot_tetr);
                bordures_render();
            pthread_mutex_unlock(&MUT_NCURSES);

            y_tetr++;  // on descend la piece

            if (collision_bas() || collision_grille()) {
                y_tetr--;

                if (ajouter_tetr_grille()) {  // ajout impossible

                    msg_game_player_t msg_game_player;
                    msg_game_player.type = MSG_TYPE_GAME;
                    msg_game_player.pid_joueur = joueur_login.pid_client;
                    msg_game_player.type_msg = GAME_MSG_DEATH;

                    msgsnd(BAL_ID, &msg_game_player, MSG_SIZEOF(msg_game_player_t), 0);  // envoie du message dans la BAL

                    pthread_mutex_lock(&MUT_THREAD_PATIE);

                    thread_partie_while = FALSE;
                    activ = thread_partie_while;

                    pthread_mutex_unlock(&MUT_THREAD_PATIE);
                } else {  // ajout possible

                    // reset pièce
                    x_tetr = CONST_LARGEUR_GRILLE / 2 - 2;
                    y_tetr = -4;

                    // augmenter le score
                    pthread_mutex_lock(&MUT_SCORE); 

                    if (idx_tetr == 0 || idx_tetr == 1) {  // O ou I
                        joueur.score += 5;
                    } else if (idx_tetr == 2 || idx_tetr == 3 || idx_tetr == 5 || idx_tetr == 6) {  // S Z L ou J
                        joueur.score += 10;
                    } else {  // T
                        joueur.score += 15;
                    }
                    sem_wait(SEM_SCORE); 
                    maj_BAL_score();
                    sem_post(SEM_SCORE); 
                    
                    pthread_mutex_unlock(&MUT_SCORE); 

                    idx_tetr = idx_proch_tetr;
                    idx_proch_tetr = generer_tetr();

                    rot_tetr = 0;
                }
            }

            // --------------------------------------------------------------------------------------- //
            #pragma endregion

            #pragma region LIGNES COMPLETES
            // --------------------------------------- LIGNES COMPLETES --------------------------------------- //
            int lignes_sup = supprimer_lignes();
            if (lignes_sup > 0) {
                pthread_mutex_lock(&MUT_NCURSES);
                    effacer_grille();
                pthread_mutex_unlock(&MUT_NCURSES);

                sup_consecutives += lignes_sup;

                // augmenter le score
                pthread_mutex_lock(&MUT_SCORE); 

                if (sup_consecutives == 1) {
                    joueur.score += 20;
                }
                else if (sup_consecutives == 2) {
                    joueur.score += 50;
                }
                else if (sup_consecutives == 3) {
                    joueur.score += 100;
                }
                else {  // 4 lignes ou +
                    joueur.score += 300;
                }
                sem_wait(SEM_SCORE); 
                maj_BAL_score();
                sem_post(SEM_SCORE); 

                pthread_mutex_unlock(&MUT_SCORE); 
            } else {
                
                if (sup_consecutives >= CONST_MIN_POUR_BONUS) {  // on vient d'arrêter de supprimer des lignes
                        msg_game_player_t msg_game_player;
                        msg_game_player.type = MSG_TYPE_GAME;
                        msg_game_player.pid_joueur = joueur_login.pid_client;
                        msg_game_player.type_msg = GAME_MSG_LINE;

                        msgsnd(BAL_ID, &msg_game_player, MSG_SIZEOF(msg_game_player_t), 0);  // envoie du message dans la BAL
                }
                
                sup_consecutives = 0;
            }
            // ------------------------------------------------------------------------------------------------ //
            #pragma endregion


        pthread_mutex_unlock(&MUT_TETROMINO);

        // grâce au timeout, on fait le render jsute dans le main
        // pthread_mutex_lock(&MUT_NCURSES);
        //     render();
        //     refresh();        
        // pthread_mutex_unlock(&MUT_NCURSES);

        // on quitte le thread ?
        pthread_mutex_lock(&MUT_THREAD_PATIE);

            activ = thread_partie_while;

        pthread_mutex_unlock(&MUT_THREAD_PATIE);
    }
    
    pthread_exit(0); 
}


void * thread_recep(void * arg) {
    bool_t activ = TRUE;

    while (activ) {


        msg_game_server_t msg;

        ssize_t res = msgrcv(BAL_ID, &msg, MSG_SIZEOF(msg_game_server_t), joueur_login.pid_client, IPC_NOWAIT);

        if (res == -1) {
            // rien reçu ou erreur -> on ignore
        } else {
            if (msg.type_msg == GAME_MSG_MALUS) {
                pthread_mutex_lock(&MUT_TETROMINO);

                for (int x=0; x<CONST_LARGEUR_GRILLE; x++) {
                    for (int y=0; y<CONST_HAUTEUR_GRILLE-1; y++) {
                        if (grille[y+1][x] != 0) {
                            grille[y][x] = 1;  // BLANC sur NOIR
                            break;  // on passe à la colonne suivante
                        }
                        else if (y == CONST_HAUTEUR_GRILLE-2) {
                            grille[y+1][x] = 1;  // BLANC sur NOIR
                            break;  // on passe à la colonne suivante
                        }
                    }
                }

                int lignes_sup = supprimer_lignes();
                if (lignes_sup > 0) {
                    pthread_mutex_lock(&MUT_NCURSES);
                        effacer_grille();
                    pthread_mutex_unlock(&MUT_NCURSES);

                    sup_consecutives += lignes_sup;

                    // augmenter le score
                    pthread_mutex_lock(&MUT_SCORE); 

                    if (sup_consecutives == 1) {
                        joueur.score += 20;
                    }
                    else if (sup_consecutives == 2) {
                        joueur.score += 50;
                    }
                    else if (sup_consecutives == 3) {
                        joueur.score += 100;
                    }
                    else {  // 4 lignes ou +
                        joueur.score += 300;
                    }
                    sem_wait(SEM_SCORE); 
                    maj_BAL_score();
                    sem_post(SEM_SCORE); 

                    pthread_mutex_unlock(&MUT_SCORE); 
                    
                } else {
                    
                    if (sup_consecutives >= CONST_MIN_POUR_BONUS) {  // on vient d'arrêter de supprimer des lignes
                            msg_game_player_t msg_game_player;
                            msg_game_player.type = MSG_TYPE_GAME;
                            msg_game_player.pid_joueur = joueur_login.pid_client;
                            msg_game_player.type_msg = GAME_MSG_LINE;

                            msgsnd(BAL_ID, &msg_game_player, MSG_SIZEOF(msg_game_player_t), 0);  // envoie du message dans la BAL
                    }
                    
                    sup_consecutives = 0;
                }

                pthread_mutex_unlock(&MUT_TETROMINO);
            }
        }


        pthread_mutex_lock(&MUT_THREAD_PATIE);
            activ = thread_partie_while;
        pthread_mutex_unlock(&MUT_THREAD_PATIE);
    }
}


// ------------------------------------------------------------------------------------ //
#pragma endregion 





#pragma region DEROUTE
// --------------------------------------- DEROUTE --------------------------------------- //

void deroute(int signal){
    switch(signal){
        case SIGINT : 
            endwin(); 
            printf("CLIENT] SIGINT, fermeture du client\n"); 



            // déconnexion par rapport au serveur
            joueur_login.fermer = TRUE;  // on veut quitter le serveur car le client se ferme
            msg_login_t msg_login;
            msg_login.type = MSG_TYPE_LOGIN;
            msg_login.msg = joueur_login;
            
            msgsnd(BAL_ID, &msg_login, MSG_SIZEOF(msg_login_t), 0);  // envoi dans la BAL
            


            sem_close(SEM_INFO_SERVEUR); 
            sem_close(SEM_SCORE);
            exit(EXIT_SUCCESS);  
            break; 
        case SIG_START : 
            
            // SIG_START -> le serveur nous signale de passer de attente à partie
            pthread_mutex_lock(&MUT_NCURSES);
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
            pthread_mutex_unlock(&MUT_NCURSES);

            maj_info_serveur();


            break; 

        case SIG_END : 
            // SIG_END -> le serveur nous signale de passer de partie à podium

            pthread_mutex_lock(&MUT_THREAD_PATIE);
                thread_partie_while = FALSE;
            pthread_mutex_unlock(&MUT_THREAD_PATIE);

            // pthread_mutex_lock(&MUT_NCURSES);
            //     clear(); 
            //     refresh(); 
            // pthread_mutex_unlock(&MUT_NCURSES);

            // maj_info_serveur(); 

            // mvprintw(10, 10, "LOADING...");  // utilisé pour "flush" ncurses et permettre d'udpate le rendu

            // sleep(1);
            
            // refresh(); 


            break; 

        default :  
            printf("Commande inconnue"); 
            break; 
    }
}

// --------------------------------------------------------------------------------------- //
#pragma endregion 