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


void creation_fichiers_necessaires(bool_t reset) {

    FILE * _fic = NULL;
    char chemin[500];  // pour concaténer les éléments du chemin

    // SI RESET, ON SUPPRIME LES FICHIERS D'ABORD
    if (reset) {
        snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_INFO_SERVEUR);
        remove(chemin);
        snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_SHM_SCORE);
        remove(chemin);
        snprintf(chemin, 500, "%s/%s", getenv("HOME"), CONST_FIC_BAL);
        remove(chemin);
    }
    // CFREATION DES FICHIERS POUR LES SHM SI NECESSAIRE
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

void affichage_logo(int y, int x){
    const char *logo[4] = {
        " _____ ___ _____ ___ ___ ___ ___",
        "|_   _| __|_   _| _ \\_ _|_ _/ __|",
        "  | | | _|  | | |   /| | | |\\__ \\ ", 
        "  |_| |___| |_| |_|_\\___|___|___/"
    };

    for (int i = 0; i < 4; i++) {
        mvprintw(y + i, x, "%s", logo[i]);
    }
    refresh();
}

void init_ncurses() {
    clear();
    initscr();
    resize_term(CONST_NB_LIGNES, CONST_NB_COLONNES);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);  // BLANC sur fond NOIR
    init_pair(2, COLOR_GREEN, COLOR_BLACK);  // VERT sur fond NOIR
    attron(COLOR_PAIR(1));
    refresh();
}