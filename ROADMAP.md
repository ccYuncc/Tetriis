- Diagrammes UML
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
## 1. Processus de connexion
## 2. Logique de jeu principale
## 3. Ajout des mécaniques secondaires (lobby, bonus/malus, meilleur score)
## 4. Polish (menus, podium, graphismes avancées, pseudos)
