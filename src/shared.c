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
    curs_set(0);  // cacher le curseur
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);  // BLANC sur fond NOIR     -> 1
    init_pair(2, COLOR_GREEN, COLOR_BLACK);  // VERT sur fond NOIR      -> 2
    init_pair(3, COLOR_RED, COLOR_BLACK);  // ROUGE sur fond NOIR       -> 3
    init_pair(4, COLOR_CYAN, COLOR_BLACK);  // CYAN sur fond NOIR       -> 4 
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);  // MAGENTA sur fond NOIR -> 5
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);  // JAUNE sur fond NOIR    -> 6
    attron(COLOR_PAIR(1));
    refresh();
}

void affichage_compteur(int compteur, int points) {
    clear(); 
    affichage_logo(2, 23); 
    if(compteur == 1) attron(COLOR_PAIR(4)); 
    else if(compteur == 2) attron(COLOR_PAIR(5)); 
    else if(compteur == 3) attron(COLOR_PAIR(6)); 
    mvprintw(10, 35, "%d", compteur); 
    for (int i=0; i<points; i++) {
        printw(".");
    }
    attron(COLOR_PAIR(1));
    refresh();
}

void affichage_lancement(){
    clear(); 
    affichage_logo(2, 23); 
    attron(COLOR_PAIR(2));
    mvprintw(10, 35, "Let's go !!"); 
    attron(COLOR_PAIR(1));
    
    refresh();
}