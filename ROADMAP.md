# A ne pas oublier
- Qui récupère l'info ? le client ? le serveur ?
- Mémoire partagée : pratique (resource à protéger)

# Arborescence des tâches
## CLIENT
- Logique de jeu en local


## SERVEUR
- 1 thread par client
- choix des tetrominos pour chaque joueur
- meilleur score de la partie protégé par mutex, actualisé en temps réel et partagé à tous les joueurs


# Processus de connexion
utiliser un ??? pour transmettre le PID du CLIENT au SERVEUR

utiliser un signal SERVEUR --> CLIENT pour valider / interdire la connexion



# ETAPES
## 0. Spécifications / Architecture / Diagrammes UML
- [x] Spécifications
- [x] Cahier des charges
- [ ] Répartition des technologies par fonctionnalité
- [ ] Design des différentes parties (podium, jeu, salle d'attente, démarrage, ...)
- [ ] diagrammes UML (séquences, activités, états)

## 1. Processus de connexion
- [ ] côté serveur
- [ ] côté client

## 2. Logique de jeu principale
- [ ] interface graphique (client)
- [ ] jeu Tetriis
- [ ] podium

## 3. Ajout des mécaniques secondaires (lobby, bonus/malus, meilleur score)
- [ ] salle d'attente
- [ ] table des résultats
- [ ] démarrage 
- [ ] bonus/malus

## 4. Polish (menus, graphismes avancées, pseudos)
- [ ] couleurs
- [ ] pseudos/menus

