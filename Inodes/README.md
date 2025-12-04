
# Démonstrateur Inodes & Blocs — Linux

Ce projet illustre le fonctionnement des inodes et des blocs dans un système de fichiers Linux.

Il permet de comparer la taille théorique des fichiers et  l’espace réel occupé sur le disque .

### **Contenu du projet**

* `inodes_demo.c` : code source du démonstrateur
* `Makefile` : compilation et gestion de la démo
* `inodes_demo/` : répertoire temporaire pour les fichiers de test

### **Fonctionnalités**

1. **Gros fichier simulé (type JPG)**
   * Création d’un fichier de taille configurable rempli de zéros avec un magic number JPG
   * Affichage de la taille théorique, du nombre de blocs, de la taille d’un bloc et de l’espace réel occupé
2. **Saturation des inodes avec petits fichiers**
   * Création de nombreux fichiers d’1 octet
   * Affichage du nombre de fichiers, de la taille théorique totale, de l’espace pris par les inodes et de l’espace réel sur disque

### **Concepts clés**

* **Inodes** : stockent les métadonnées des fichiers (permissions, taille, blocs).
* **Blocs** : unité minimale d’allocation du système de fichiers (souvent 4096 octets).
* Différence entre taille théorique et réelle due aux blocs complets et aux inodes.

### **Utilisation**

Compiler :

```
make
```

Exécuter le programme :

```
make run
```

Gros fichier :

```
make big
```

Saturation des inodes :

```
make inode
```

Réinitialiser la démo :

```
make reset
```

Nettoyer :

```
make clean
```
