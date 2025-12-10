# Gestionnaire de Processus Zombies

Programme C démontrant la gestion des processus zombies et des signaux sous Linux.

## Fonctionnement

Le programme crée automatiquement un processus zombie toutes les  **3 secondes** . Ces zombies peuvent être nettoyés via des signaux personnalisés.

### Compilation

bash

```bash
# Avec Makefile
make

# Ou manuellement
gcc -Wall -Wextra -std=c99 -o zombie_manager zombie_manager.c
```

## Utilisation

### Démarrer le programme

bash

```bash
./zombie_manager
```

Le programme affiche son PID au démarrage. Notez-le pour envoyer des signaux.

### Commandes (depuis un autre terminal)

**Nettoyer les zombies :**

bash

```bash
kill -SIGUSR1 <PID>
```

**Arrêter la création de zombies :**

bash

```bash
kill -SIGUSR2 <PID>
```

**Quitter proprement :**

bash

```bash
# Dans le terminal du programme
Ctrl+C
```

## Exemple de session

bash

```bash
# Terminal 1
$ ./zombie_manager
╔═══════════════════════════════════════════════╗
║     GESTIONNAIRE DE PROCESSUS ZOMBIES        ║
╚═══════════════════════════════════════════════╝

[MANAGER] Démarrage avec PID: 12345

Signaux configurés:
  - SIGUSR1: Nettoyer les zombies    (kill -SIGUSR1 12345)
  - SIGUSR2: Arrêter création        (kill -SIGUSR2 12345)
  - SIGINT:  Quitter proprement      (Ctrl+C)

[ZOMBIE 12346] Créé et terminé
[MANAGER] Zombie #1 créé (PID: 12346). Total zombies: 1
...

# Terminal 2
$ ps aux |grep Z
# Voir les zombies (état "Z+")

$ kill -SIGUSR1 12345
# Les zombies sont nettoyés

$ kill -SIGUSR2 12345
# Arrête la création, nettoie et quitte
```

## Vérification des zombies

bash

```bash
# Voir tous les processus zombies du système
ps aux |grep Z

# Voir les enfants zombies d'un processus spécifique
ps --ppid <PID> -o pid,stat,cmd
```

## Signaux utilisés

| Signal  | Effet                                   |
| ------- | --------------------------------------- |
| SIGUSR1 | Nettoie tous les zombies existants      |
| SIGUSR2 | Arrête la création, nettoie et quitte |
| SIGINT  | Quitter proprement (Ctrl+C)             |

## Configuration

Variables modifiables dans le code :

c

```c
define ZOMBIE_INTERVAL 3// Secondes entre chaque zombie
define MAX_ZOMBIES 50 // Nettoyage auto si limite atteinte
```

## Nettoyage

bash

```bash
make clean
```

## Concepts clés démontrés

* **fork()** : Création de processus enfants
* **wait() / waitpid()** : Récupération du statut des enfants
* **Processus zombies** : État entre terminaison et récupération par le parent
* **Signaux personnalisés** : SIGUSR1 et SIGUSR2
* **Handlers de signaux** : `sigaction()`
* **Flags atomiques** : `volatile sig_atomic_t` pour communication signal-safe
