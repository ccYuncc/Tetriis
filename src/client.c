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

// AUTRES
info_serveur_t serveur; // informations du serveur tetriis
login_t joueur;  // contient les informations sur le client actuel


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
    creation_fichiers_necessaires();

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
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_PROJECT_ID);
    key_t tok_BAL = ftok(chemin, CONST_FIC_BAL);
    CHECK(tok_BAL, "DEBUG ] CLIENT ] Erreur ftok BAL");

    BAL_ID = msgget(tok_BAL, 0666);
    CHECK(BAL_ID, "DEBUG ] CLIENT ] Erreur msgget BAL");

    // ------------------------------------------------------------------------------------ //
    #pragma endregion

    #pragma region ACQUISITION PID SERVEUR    
    // --------------------------------------- ACQUISITION PID SERVEUR  --------------------------------------- //
    
    // ATTENTE DISPONIBILITE DE LA SHM PROTEGEE PAR LA SEM
    sem_wait(SEM_INFO_SERVEUR);

        // lecture du PID du serveur dans la SHM
        info_serveur_t * info_serveur = shmat(SHM_INFO_SERVEUR, NULL, O_RDONLY);
        CHECK(info_serveur, "DEBUG ] CLIENT ] Erreur shmat SHM_INFO_SERVEUR");
        
        serveur.pid_serveur = info_serveur->pid_serveur;
        serveur.etat_serveur = info_serveur->etat_serveur;

        shmdt(info_serveur);

        printf("CLIENT] PID du serveur : %d", serveur.pid_serveur);
    
    sem_post(SEM_INFO_SERVEUR);  // SHM DE NOUVEAU DISPONIBLE -> SEM REMISE

    // -------------------------------------------------------------------------------------------------------- //
    #pragma endregion


    #pragma region CONNEXION
    // --------------------------------------- CONNEXION --------------------------------------- //


        strcpy(joueur.pseudo, "PSEUDO_TEST");  // TODO: faire l'acquisition d'un pseudo
        joueur.pid_client = getpid();
        printf("CLIENT] PID du client : %d", joueur.pid_client);



    // ----------------------------------------------------------------------------------------- //
    #pragma endregion


    sem_close(SEM_INFO_SERVEUR);
    sem_close(SEM_SCORE);

    return 0;
}