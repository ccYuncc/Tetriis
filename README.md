```diff
        _____ ___ _____ ___ ___ ___ ___ 
       |_   _| __|_   _| _ \_ _|_ _/ __|
         | | | _|  | | |   /| | | |\__ \
         |_| |___| |_| |_|_\___|___|___/           TETRIIS - par Cloé GREBERT & Thomas DUTHOIT

```

---
# SOMMAIRE
- [<ins>**I. Introduction**</ins>](#i-introduction)
- [<ins>**II. Livrable**</ins>](#ii-livrable)
- [<ins>**III. Conception**</ins>](#iii-conception)
  - [**III.1 Diagrammes UML**](#iii1-diagrammes-uml)
    - [Diagramme d'activité du client](#1-diagramme-dactivité-du-client)
    - [Diagramme d'activité du serveur](#2-diagramme-dactivité-du-serveur)
    - [Diagramme de séquence pour la connexion d'un client au serveur](#3-diagramme-de-séquence-pour-la-connexion-dun-client-au-serveur)
  - [**III.2 Mockups des vues de l'application**](#iii2-mockups-des-vues-de-lapplication)
    - [1. Salle d'attente](#1-salle-dattente)
    - [2. Attente d'une fin de partie](#2-attente-dune-fin-de-partie)
    - [3. Déroulement d'une partie](#3-déroulement-dune-partie)
    - [4. Fin de partie](#4-fin-de-partie)
---

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

# III. CONCEPTION
Afin de concevoir et de décrire le comportement de Tetriis, plusieurs travaux de préparation ont été réalisés avant de commencer la production du jeu afin d'anticiper les différentes étapes et fonctionnalités du programme.
## III.1 Diagrammes UML
Tout d'abord, les diagrammes UML nous ont permis de planifier la logique de l'application, que ce soit pour le client ou pour le serveur, ainsi que l'interraction entre ces entités.
### 1. Diagramme d'activité du client
![Diagramme activité client](https://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/UML/activiteclient.puml)
### 2. Diagramme d'activité du serveur
![Diagramme activité serveur](https://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/UML/activiteserveur.puml)
### 3. Diagramme de séquence pour la connexion d'un client au serveur
![Diagramme séquence connexion](https://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/UML/connexion.puml)

## III.2 Mockups des vues de l'application
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
La vue principale du jeu est la vue affichée lorsqu'une partie se déroule, une fois de plus, un affichage serveur est prévu enn plus de l'affichage sur le client.
- Déroulé de la partie côté client:
  ![Salle d'attente client](https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/Design/Design_Partie_JOUEUR.png)
- Déroulé de la partie côté Serveur:
  ![Salle d'attente client](https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/Design/Design_Partie_SERVEUR.png)

### 4. Fin de partie
Une fois la partie terminée, les podiums sont affichés sur le serveur et sur les clients avant de revenir à une salle d'attente (cf. [Diagrammes d'activité](#iii1-diagrammes-uml))
  ![Salle d'attente client](https://raw.githubusercontent.com/ccYuncc/Tetriis/refs/heads/main/Design/Design_Podium_JOUEUR.png)