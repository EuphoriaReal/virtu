
# Démonstration de buffer overflow en mémoire partagée

## Objectif

Ce projet illustre, de manière volontairement simple, les effets d'un **buffer overflow** dans une structure partagée entre deux processus à l'aide de la mémoire partagée System.

---

## Fichiers

- `writer.c`  
  Crée la mémoire partagée et écrit dans le buffer.
  Deux modes sont proposés : écriture sécurisée et écriture avec overflow.

- `reader.c`  
  Lit la mémoire partagée et vérifie son intégrité.

---

## Compilation

```bash
make
```

Deux exécutables sont générés :

* `writer`
* `reader`

---

## Utilisation

### Initialisation / écriture

```bash
./writer reset
./writer safe
./writer overflow
```

* `reset` : initialise la mémoire partagée et le canary.
* `safe` : écriture contrôlée, sans corruption.
* `overflow` : écriture volontairement dangereuse provoquant un overflow.

### Lecture / vérification

```bash
./reader
```

Le programme affiche :

* le contenu du buffer,
* l'état du canary,
* la détection éventuelle d'un overflow.

---

## Principe de la démonstration

La structure partagée contient :

* un compteur,
* un buffer de taille fixe,
* une zone sentinelle ( *canary* ).

Un dépassement du buffer entraîne l'écrasement du canary, ce qui permet de détecter simplement une corruption mémoire.

---
