/**
 *      _____ ___ _____ ___ ___ ___ ___
 *     |_   _| __|_   _| _ \_ _|_ _/ __|
 *       | | | _|  | | |   /| | | |\__ \
 *       |_| |___| |_| |_|_\___|___|___/
 *
 * Project : Tetriis
 * File name : shared.c
 * Description :  
 *          Functions shared between client and server
 * Authors : DUTHOIT Thomas / GREBERT Cloé
 */


#include "shared.h"


void creation_fichiers_necessaires() {
    // CFREATION DES FICHIERS POUR LES SHM SI NECESSAIRE
    FILE * _fic = NULL;
    char chemin[500];  // pour concaténer les éléments du chemin
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_DIR_SHM);
    mkdir(chemin, 0700);  // création du dossier si il n'existe pas

    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_INFO_SERVEUR);
    if (_fic = fopen(chemin, "a")) {
        fclose(_fic);
    }
    _fic = NULL;
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_SCORE);
    if (_fic = fopen(chemin, "a")) {
        fclose(_fic);
    }
    _fic = NULL;
    snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_BAL);
    if (_fic = fopen(chemin, "a")) {
        fclose(_fic);
    }
}