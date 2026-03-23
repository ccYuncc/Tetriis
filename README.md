```diff
        _____ ___ _____ ___ ___ ___ ___ 
       |_   _| __|_   _| _ \_ _|_ _/ __|
         | | | _|  | | |   /| | | |\__ \
         |_| |___| |_| |_|_\___|___|___/           TETRIIS - par Cloé GREBERT & Thomas DUTHOIT

```

---





# SOMMAIRE
- [<ins>**INSTALLATION**</ins>](#installation)
- [<ins>**MODE D'EMPLOI**</ins>](#mode-demploi)
    - [Le serveur](#1-le-serveur)
    - [Le client](#2-le-client)
- [<ins>**I. Introduction**</ins>](#i-introduction)
- [<ins>**II. Livrable**</ins>](#ii-livrable)
- [<ins>**III. Tetriis**</ins>](#iv-conception)
- [<ins>**IV. Conception**</ins>](#iv-conception)
  - [**IV.1 Diagrammes UML**](#iv1-diagrammes-uml)
    - [Diagramme d'activité du client](#1-diagramme-dactivité-du-client)
    - [Diagramme d'activité du serveur](#2-diagramme-dactivité-du-serveur)
    - [Diagramme d'état du client](#3-diagramme-détat-du-client)
    - [Diagramme d'état du serveur](#4-diagramme-détat-du-serveur)
  - [**III.2 Mockups des vues de l'application**](#iv2-mockups-des-vues-de-lapplication)
    - [1. Salle d'attente](#1-salle-dattente)
    - [2. Attente d'une fin de partie](#2-attente-dune-fin-de-partie)
    - [3. Déroulement d'une partie](#3-déroulement-dune-partie)
    - [4. Fin de partie](#4-fin-de-partie)
---


# INSTALLATION / COMPILATION

Pour installer Tetriis, vous pouvez télecharger l'archive de code directement ou la cloner à l'aide de la commande:
```sh
git clone https://github.com/ccYuncc/Tetriis.git
```
Une fois dans le répertoire du projet, la compilation de Tetriis se fait grâce à Makefile:
```sh
make
```
Ce qui aura pour effet de créer les 2 exécutables suivants:
- `bin/server.exe`
- `bin/client.exe`

La création des dossiers et fichiers pour l'utilisation de Tetriis est automatisée au lancement de ces exécutables, l'étape d'installation se termine ici.

# MODE D'EMPLOI

> ATTENTION : Comme le projet repose sur le système de fichier Linux et que le projet détecte automatiquement les chemins en fonction de l'utilisateur qui lance le programme, il est **impératif** de lancer tous les exécutables avec **le même utilisateur**.

## 1. Le serveur

Le serveur est le premier exécutable à lancer pour jouer à Tetriis.
A partir du répertoire du projet, le serveur est lançable grâce à l'éxécutable situé dans `bin/server.exe`. Cet exécutable peut être lancé peu importe le répertoire courant. Une fois lancé, il va fonctionner de manière totalement autonome, et il est arrêtable de deux manières:
- Touche `[ECHAP]`en mode attente
- `[Ctrl+C]` à n'importe quel moment

La fermeture du serveur entraîne la fermeture de tous les clients qui y étaient connectés.

## 2. Le client

A partir du répertoire du projet, le client est lançable grâce à l'éxécutable situé dans `bin/client.exe`. Cet exécutable peut être lancé peu importe le répertoire courant. Un pseudo peut être directement renseigné en argument de ligne de commande (`./bin/client.exe monPseudo`), sinon il vous sera demandé de le rentrer avant la connexion (Un pseudo a une longueur max de 10 caractères).

Une fois le pseudo rentré et la connexion effectuéee, voici les contrôles selont la vue affichée:
- Peu importe la vue:
  - `[Ctrl+C]`pour quitter
- Mode attente de lancement de partie:
  - `[ENTRER]`pour passer prêt/non prêt
- Mode partie:
  - `[A]`pour déplacer la pièce vers la gauche
  - `[E]`pour déplacer la pièce vers la droite
  - `[Z]`pour tourner la pièce
  - `[S]`pour faire tomber la pièce plus vite

# I. INTRODUCTION 
Le mini-projet se doit de respecter un ensemble de contraintes et d'attentes, listées ci dessous, pour être valide
- Par équipe de 2 étudiants
- Le sujet fonctionnel est libre mais doit être approuvé par l'enseignante
- Le cadre technique impose les contraintes suivantes :
  - Développement uniquement en langage C
  - Application réalisée par un ou plusieurs programmes
  - Application multi processus et/ou multi-threads
  - Obligation de mettre en œuvre :
    - Au moins 2 techniques de communication inter-processus (signaux, mémoire partagée, tubes, boites aux lettres)
    - Au moins une technique de protection de ressource partagée (mutex, sémaphores)
  - Application en mode texte ( pas de graphisme fenêtré)
  - Application s'exécutant sur une seule machine, pas de réseau, pas de sockets
  - 
# II. LIVRABLE
- Une application opérationnelle
- Un manuel d'installation et d'utilisation
- Une soutenance avec démonstration et expérimentation par vos camarades (le 30 Janvier 2026). La soutenance doit faire l'objet d'une présentation de votre application en exploitant des diagrammes UML pertinents (Diagramme de cas d'utilisation,, diagramme d'activité, diagramme de séquence)
- Accès au code source pour votre enseignante

# III. TETRIIS

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
- Pour qu'une partie se lance, il faut qu'il y ait au moins 3 joueurs connectés, et que 3/4 des joueurs se déclarent prêts à jouer.
- Pendant la partie, il gère la machine à état du jeu (attente, partie, podium), le mécanisme de connexion/déconnexion, les bonus/malus et la détection de fin de partie.

---

## PRINCIPES DE JEU
- <ins>**But général :**</ins> Le but du jeu est de former des lignes complètes de blocs en empilant des pièces qui tombent du haut de l'écran, à chaque fois qu'une ligne complète est faite, elle est supprimée, et toutes les pièces présentes au dessus descendent d'un bloc.
- <ins>**Bonus/Malus :**</ins> 
    - Malus : Le Malus (qui peut être vu comme un bonus pour le joueur qui le génère) ajoute une ligne supplémentaire chez les autres joueurs quand joueur supprime au moins 2 lignes d'un coup.
- <ins>**Fin de partie :**</ins> Une partie se finit quand il ne reste plus qu'un joueur en lice.
- <ins>**Défaite :**</ins> Un joueur perd la partie s'il ne peut plus faire descendre de pièce dans la zone de jeu
- <ins>**Victoire :**</ins> Un joueur peut gagner la partie de 2 manières différentes: il est le dernier en lice dans la partie (survivant)  ou il a le plus grand score à la fin de la partie (Highest Scorer). Il peut donc y avoir 2 joueurs gagnant maximum dans une partie.



# IV. CONCEPTION
Afin de concevoir et de décrire le comportement de Tetriis, plusieurs travaux de préparation ont été réalisés avant de commencer la production du jeu afin d'anticiper les différentes étapes et fonctionnalités du programme.
## IV.1 Diagrammes UML
Tout d'abord, les diagrammes UML nous ont permis de planifier la logique de l'application, que ce soit pour le client ou pour le serveur, ainsi que l'interaction entre ces entités.
### 1. Diagramme d'activité du client
![Diagramme activité client](https://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/UML/activiteclient.puml)
### 2. Diagramme d'activité du serveur
![Diagramme activité serveur](https://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/UML/activiteserveur.puml)
### 3. Diagramme d'état du client
![Diagramme séquence connexion](https://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/UML/diagrammeetatclient.puml)
### 4. Diagramme d'état du serveur
![Diagramme séquence connexion](https://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/UML/diagrammeetatserveur.puml)

## IV.2 Mockups des vues de l'application
Malgré la contrainte d'une application avec un rendu en mode texte uniquement, nous avons réalisés quelques designs pour les différentes vues qui composent notre application, notamment pour le client, cela nous a permi d'avoir une base solide sur laquelle se baser lors de la phase de production.
### 1. Salle d'attente
Lorsqu'une partie est en attente de lancement (pas assez de joueurs sont prêts), une vue de salle d'attente est affichée sur le client et sur le serveur:
- Salle d'attente côté client:
  ![Salle d'attente client](https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/Design/Design_Salle_Attente_JOUEUR.png)
- Salle d'attente côté serveur:
  ![Salle d'attente client](https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/Design/Design_Salle_Attente_SERVEUR.png)

### 2. Attente d'une fin de partie
Si un client tente de rejoindre un serveur sur lequel une partie déjà lancée, alors il aura une vue d'attente de la fin de cette partie, afin de pouvoir rejoindre la prochaine:
  ![Salle d'attente client](https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/Design/Design_Partie_Attente_JOUEUR.png)

### 3. Déroulement d'une partie
La vue principale du jeu est la vue affichée lorsqu'une partie se déroule, une fois de plus, un affichage serveur est prévu en plus de l'affichage sur le client.
- Déroulé de la partie côté client:
  ![Salle d'attente client](https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/Design/Design_Partie_JOUEUR.png)
- Déroulé de la partie côté Serveur:
  ![Salle d'attente client](https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/Design/Design_Partie_SERVEUR.png)

### 4. Fin de partie
Une fois la partie terminée, les podiums sont affichés sur le serveur et sur les clients avant de revenir à une salle d'attente (cf. [Diagrammes d'activité](#iii1-diagrammes-uml))
  ![Salle d'attente client](https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/Design/Design_Podium_JOUEUR.png)
