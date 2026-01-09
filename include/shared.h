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
#include <pthread.h>


#define CONST_LONGUEUR_PSEUDO 20

#define TRUE 1
#define FALSE 0

typedef int bool_t;

typedef struct{
    pid_t pid_serveur; 
    etat_serveur_t etat_serveur; 
} info_serveur_t; 

typedef enum {
    ATTENTE=1,
    PARTIE=2,
    POSIUM=3,
} etat_serveur_t;

typedef struct{
    pid_t pid_client; 
    char pseudo[CONST_LONGUEUR_PSEUDO];
    int client_vers_serveur[2];
    int serveur_vers_client[2]; 
} login_t; 


#endif  // SHARED_H