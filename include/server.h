/**
 *      _____ ___ _____ ___ ___ ___ ___
 *     |_   _| __|_   _| _ \_ _|_ _/ __|
 *       | | | _|  | | |   /| | | |\__ \
 *       |_| |___| |_| |_|_\___|___|___/
 *
 * Project : Tetriis
 * File name : server.h
 * Description :  
 *          Include file for the server with its typedef, constants etc...
 * Authors : DUTHOIT Thomas / GREBERT Cloé
 */

#ifndef SERVER_H
#define SERVER_H

#include "shared.h"

#define CONST_NOMBRE_JOUEURS 50


typedef struct {
    login_t liste_joueurs[CONST_NOMBRE_JOUEURS];
    int nb_joueurs;
    int nb_joueurs_en_partie; 
} joueurs_t;


#endif  // SERVER_H