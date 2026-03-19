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
    init_pair(7, COLOR_BLUE, COLOR_BLACK);  // BLEU sur fond NOIR    -> 7
    init_pair(8, COLOR_YELLOW, COLOR_YELLOW); //  JAUNE sur fond JAUNE -> 8
    init_pair(9, COLOR_WHITE, COLOR_WHITE); //  BLANC sur fond BLANC -> 9
    init_pair(10, COLOR_RED, COLOR_RED); //  ROUGE sur fond ROUGE -> 10
    attron(COLOR_PAIR(1));
    refresh();
}

void affichage_compteur(int compteur, int points) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);  
    clear(); 
    affichage_logo(2, ((cols-34)/2)); 
    if(compteur == 1) attron(COLOR_PAIR(4)); 
    else if(compteur == 2) attron(COLOR_PAIR(5)); 
    else if(compteur == 3) attron(COLOR_PAIR(6)); 
    mvprintw(10, ((cols-4)/2), "%d", compteur); 
    for (int i=0; i<points; i++) {
        printw(".");
    }
    attron(COLOR_PAIR(1));
    refresh();
}

void affichage_lancement(){
    int rows, cols;
    getmaxyx(stdscr, rows, cols);  
    clear(); 
    affichage_logo(2, ((cols-34)/2)); 
    attron(COLOR_PAIR(2));
    mvprintw(10, ((cols-11)/2), "Let's go !!"); 
    mvprintw(12, ((cols-11)/2), "Have fun !!"); 
    attron(COLOR_PAIR(1));
    
    refresh();
}

void affichage_podium(score_t * info_score){
    int rows, cols; 
    getmaxyx(stdscr, rows, cols); 

    clear(); 
    affichage_logo(2, ((cols-34)/2)); 
    mvprintw(7, ((cols-34)/2), "We hope you enjoy the game"); 

    attron(A_BOLD);
    mvprintw((rows/2)-3, ((cols-7)/2), "Results");
    attroff(A_BOLD);


    for(int i = 0; i < 5 ; i++){
        attron(COLOR_PAIR(9));
        mvprintw((rows/2)+4+i, (cols/6)-10, "__________");  
        mvprintw((rows/2)+4+i, (4*cols/6)-10, "__________"); 
    }

    for(int i = 0; i < 7; i++){
        attron(COLOR_PAIR(8));
        mvprintw((rows/2)+2+i, (cols/6), "__________"); 
        mvprintw((rows/2)+2+i, (4*cols/6), "__________"); 
    }

    for(int i = 0; i < 4; i++){
        attron(COLOR_PAIR(10));
        mvprintw((rows/2)+5+i, (cols/6)+10, "__________");  
        mvprintw((rows/2)+5+i, (4*cols/6)+10, "__________"); 
    }

    attron(COLOR_PAIR(1));

    mvprintw((rows/2)+3, (cols/6)-9, "%s", info_score->last_survivors[1]); 
    mvprintw((rows/2)+1, (cols/6)+1, "%s", info_score->last_survivors[0]); 
    mvprintw((rows/2)+4, (cols/6)+11, "%s", info_score->last_survivors[2]); 

    mvprintw((rows/2)+3, (4*cols/6)-9, "%s", info_score->deuxieme.login_joueur.pseudo); 
    mvprintw((rows/2)+1, (4*cols/6)+1, "%s", info_score->premier.login_joueur.pseudo); 
    mvprintw((rows/2)+4, (4*cols/6)+11, "%s", info_score->troisieme.login_joueur.pseudo); 

    mvprintw((rows/2)+2, (4*cols/6)-9, "(%d)", info_score->deuxieme.score); 
    mvprintw((rows/2), (4*cols/6)+1, "(%d)", info_score->premier.score); 
    mvprintw((rows/2)+3, (4*cols/6)+11, "(%d)", info_score->troisieme.score); 

    mvprintw(rows-1, 0, "Tetriis was made by GREBERT Cloe and DUTHOIT Thomas"); 
    refresh(); 
}