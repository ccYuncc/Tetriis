/**
 *      _____ ___ _____ ___ ___ ___ ___
 *     |_   _| __|_   _| _ \_ _|_ _/ __|
 *       | | | _|  | | |   /| | | |\__ \
 *       |_| |___| |_| |_|_\___|___|___/
 *
 * Project : Tetriis
 * File name : shared.h
 * Description :  
 *          Constants shared between clients and server
 * Authors : DUTHOIT Thomas / GREBERT Cloé
 */

#ifndef SHARED_H
#define SHARED_H



#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/types.h>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <ncurses.h>
#include <unistd.h>


#define CONST_SEM_NOM_INFO_SERVEUR "/TETRIIS_SEM_INFO_SERVEUR"
#define CONST_SEM_NOM_SCORE "/TETRIIS_SEM_SCORE"

#define CONST_DIR_SHM ".tetriis"  // répertoire "caché"
#define CONST_FIC_SHM_INFO_SERVEUR ".tetriis/info_serveur"  // fichier dans un répertoire "caché"
#define CONST_FIC_SHM_SCORE ".tetriis/score"  // fichier dans un répertoire "caché"

#define CONST_FIC_BAL ".tetriis/bal"

#define CONST_PROJECT_ID_INFO 1
#define CONST_PROJECT_ID_SCORE 2
#define CONST_PROJECT_ID_BAL 3

#define CONST_LONGUEUR_PSEUDO 20
#define CONST_LONGUEUR_REPONSE_SERVEUR 20

#define TRUE 1
#define FALSE 0

#define SIG_LOGIN SIGUSR1


#define CONST_LOGIN_OK    "YES"
#define CONST_LOGIN_ECHEC "NO"

#define CONST_NB_COLONNES  80
#define CONST_NB_LIGNES    24


#define MSG_TYPE_LOGIN 1
#define MSG_TYPE_READY 2
#define MSG_TYPE_GAME 3


#define GAME_MSG_DEATH 1
#define GAME_MSG_LINE 2 
#define GAME_MSG_MALUS 3

#define SIG_START 10 // SIGUSR1
#define SIG_END 12 //  SIGUSR2


#define CHECK(sts,msg)      if ((sts) == -1 )  { perror(msg);exit(-1);}
#define MSG_SIZEOF(type)    sizeof(type) - sizeof(long)   // pour les sizeof des messages

typedef int bool_t;

typedef enum {
    ATTENTE=1,
    PARTIE=2,
    PODIUM=3,
} etat_serveur_t;

typedef struct {
    pid_t pid_serveur; 
    etat_serveur_t etat_serveur; 
} info_serveur_t; 

typedef struct {
    pid_t pid_client; 
    char pseudo[CONST_LONGUEUR_PSEUDO];
    bool_t fermer;  // 0 -> demande de connexion // 1 -> demande de déconnexion
} login_t; 

typedef struct{
  int score; 
  login_t login_joueur; 
} joueur_t; 

typedef struct
{
    long type;  // TYPE = 1
    login_t msg;
} msg_login_t;

typedef struct{
  joueur_t premier; 
  joueur_t deuxieme; 
  joueur_t troisieme;
  char last_survivors[3][CONST_LONGUEUR_PSEUDO]; 
} score_t; 

typedef char reponse_serveur_t[CONST_LONGUEUR_REPONSE_SERVEUR];

typedef struct
{
    long type;  // TYPE = PID CIBLE
    reponse_serveur_t msg;
} msg_reponse_serveur_t;

typedef struct
{
    long type; // TYPE = 2
    bool_t ready; 
    pid_t pid_joueur; 
} msg_ready_player_t;

typedef struct 
{
    long type; // TYPE = 3
    int type_msg; // SUJET DU MESSAGE : MORT (1) | LIGNE COMPLETE (2) | ....
    pid_t pid_joueur; 
} msg_game_player_t;

typedef struct 
{
    long type; 
    int type_msg; 
} msg_game_server_t; 


void creation_fichiers_necessaires(bool_t reset);
void affichage_logo(int y, int x);
void init_ncurses();  
void affichage_compteur(int compteur, int points);
void affichage_lancement();
void affichage_podium(score_t * info_score); 

#endif  // SHARED_H