
# inodes_demo

## Description

`inodes_demo` est un petit démonstrateur pour explorer deux aspects des systèmes de fichiers :  

1. **Gros fichier** : création d'un fichier de grande taille (50 Mo) et affichage de son espace logique et réel.  
2. **Saturation des inodes** : création de nombreux petits fichiers pour observer l'impact sur l'espace disque et les inodes.

Le programme inclut également une **réinitialisation simple** pour supprimer le répertoire de travail `fs_demo`.

---

## Compilation

Pour compiler le programme, utilisez le Makefile fourni :

```bash
make
````

Le binaire généré s'appelle :

```bash
./inodes_demo
```

---

## Utilisation

Le programme accepte un argument pour choisir le mode :

```bash
./inodes_demo {big|inode|reset}
```

* `big` : crée le gros fichier (`fs_demo/image.jpg`) et affiche ses caractéristiques.
* `inode` : crée de nombreux petits fichiers dans `fs_demo/small` pour démontrer la saturation des inodes.
* `reset` : supprime le répertoire `fs_demo` et tous ses fichiers, permettant de repartir à zéro.

### Exemples

Créer le gros fichier :

```bash
./inodes_demo big
```

Créer les petits fichiers pour saturation des inodes :

```bash
./inodes_demo inode
```

Réinitialiser la démo :

```bash
./inodes_demo reset
```

---

## Nettoyage

Pour supprimer le binaire et les fichiers objets générés par la compilation :

```bash
make clean
```