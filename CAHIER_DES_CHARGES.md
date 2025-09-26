```diff
        _____ ___ _____ ___ ___ ___ ___ 
       |_   _| __|_   _| _ \_ _|_ _/ __|
         | | | _|  | | |   /| | | |\__ \
         |_| |___| |_| |_|_\___|___|___/           TETRIIS - par Cloé GREBERT & Thomas DUTHOIT
```
# TETRIIS - Cahier des charges


---

## OBJECTIF GENERAL

L'objectif de ce projet est de mettre en place un jeu multijoueur inspiré de Tetris en code C sous un système d'exploitation Unix. 
Tetriis se distingue du jeu original en ajoutant des spécificités tel que des bonus ou malus en fonction du score, ou encore du multijoueur en temps réel.

---

## CLIENT
- Au niveau client, il faut une interface ergonomique et visuellement plaisante.
- En dehors de la zone de jeu principale, il faut également l'affichage d'informations importante, comme le nombre de joueurs encore en vie, le classement et le score du joueur.
- Le client devra également gérer la procédure de connexion avec le serveur et la gestion d'erreurs si présentes.
- Une salle d'attente doit être présente pour faire patienter le temps que tous les joueurs soient prêts pour lancer une partie ou alors lorsqu'un joueur rejoint une partie en cours.
- Un podium en fin de partie devra afficher les meilleurs joueurs de la partie.
- Lorsqu'un joueur perd, un écran récapitulatif avec les scores des autres joueurs s'affiche et s'actualise en temps réel.

---

## SERVEUR
- Au démarrage, le serveur affiche son PID ainsi que l'IP de la machine pour que les joueurs puissent se connecter en SSH au serveur pour jouer à distance en affichant la commande SSH à éxecuter pour se connecter.
- Pour qu'une partie se lance, il faut qu'il y ait au moins 2 joueurs connectés, et que 3/4 des joueurs se déclarent prêts à jouer.
- Pendant la partie, il gère toute la partie, que ce soit la gestion des scores, la génération des pièces, les bonus/malus, et la détection de fin de partie.

---

## PRINCIPES DE JEU
- <ins>**But général :**</ins> Le but du jeu est de former des lignes complètes de blocs en empilant des pièces qui tombent du haut de l'écran, à chaque fois qu'une ligne complète est faite, elle est supprimée, et toutes les pièces présentes au dessus descendent d'un bloc.
- <ins>**Bonus/Malus :**</ins> 
    - Bonus : Au niveau du Bonus, le joueur ayant le plus grand score de la partie est immunisé contre les malus des autres joueurs
    - Malus : Au niveau du Malus, quand un joueur supprime 2 lignes d'un coup, une ligne supplémentaire apparaît chez les autres joueurs.
- <ins>**Fin de partie :**</ins> Une partie se finit quand il ne reste plus qu'un joueur en lice.
- <ins>**Défaite :**</ins> Un joueur perd la partie s'il ne peut plus faire descendre de pièce dans la zone de jeu
- <ins>**Victoire :**</ins> Un joueur peut gagner la partie de 2 manières différentes: il est le dernier en lice dans la partie (survivant)  ou il a le plus grand score à la fin de la partie (Highest Scorer). Il peut donc y avoir 2 joueurs gagnant maximum dans une partie.



