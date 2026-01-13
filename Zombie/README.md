# Zombie Process Generator

## Description

Ce programme C illustre la création de **processus zombies** sous Linux/Unix.

Le programme :

- crée régulièrement des processus enfants
- les enfants se terminent immédiatement
- le parent **n’appelle pas `wait()`**, ce qui laisse les enfants en état **zombie**
- un **signal personnalisé (`SIGUSR1`)** permet de nettoyer les zombies via `waitpid()`

## Compilation

```bash
gcc zombie.c -o zombie
```

---

## Exécution

```bash
./zombie
```

Le programme affiche le **PID du processus parent**, nécessaire pour envoyer le signal.

---

## Création des zombies

Les zombies sont créés automatiquement toutes les **2 secondes**.

Pour les visualiser :

```bash
ps --ppid <PID_PARENT> -o pid,ppid,stat,cmd
```

---

## Nettoyage des zombies (signal personnalisé)

Les zombies ne peuvent pas être tués directement.
Ils disparaissent uniquement lorsque le parent appelle `wait()`.

Envoyer le signal :

```bash
kill -SIGUSR1 <PID_PARENT>
```

Effet :

* le parent récupère tous les enfants zombies
* les zombies disparaissent de la table des processus
