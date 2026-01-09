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


#define CONST_SEM_NOM_INFO_SERVEUR "/TETRIIS_SEM_INFO_SERVEUR"
#define CONST_SEM_NOM_SCORE "/TETRIIS_SEM_SCORE"

#define CONST_DIR_SHM ".tetriis"  // répertoire "caché"
#define CONST_FIC_SHM_INFO_SERVEUR ".tetriis/info_serveur"  // fichier dans un répertoire "caché"
#define CONST_FIC_SHM_SCORE ".tetriis/score"  // fichier dans un répertoire "caché"

#define CONST_FIC_BAL ".tetriis/bal"

#define CONST_PROJECT_ID 1

#define CONST_LONGUEUR_PSEUDO 20

#define TRUE 1
#define FALSE 0

#define SIG_LOGIN SIGUSR1

#define CHECK(sts,msg) if ((sts) == -1 )  { perror(msg);exit(-1);}

typedef int bool_t;

typedef enum {
    ATTENTE=1,
    PARTIE=2,
    POSIUM=3,
} etat_serveur_t;

typedef struct {
    pid_t pid_serveur; 
    etat_serveur_t etat_serveur; 
} info_serveur_t; 

typedef struct {
    pid_t pid_client; 
    char pseudo[CONST_LONGUEUR_PSEUDO];
    int client_vers_serveur[2];
    int serveur_vers_client[2]; 
} login_t; 

typedef struct {
    int premier;
    int deuxieme;
    int troisieme;
} score_t;


void creation_fichiers_necessaires();



#endif  // SHARED_H