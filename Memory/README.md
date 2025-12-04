# Démonstration de Buffer Overflow en Mémoire Partagée (IPC)

Ce projet démontre comment un **buffer overflow** peut se produire dans une mémoire partagée entre processus, et comment le détecter. C'est un exemple éducatif pour comprendre :

* La gestion de la mémoire partagée (IPC - Inter-Process Communication)
* Les vulnérabilités de type buffer overflow
* Les techniques de détection (canary values)
* L'importance de la validation des entrées

## Architecture

Le projet se compose de **deux processus indépendants** :

### 1. **Writer** (Processus écrivain)

* Crée et écrit dans la mémoire partagée
* Propose 3 modes d'écriture :
  * **Sécurisée** : utilise `strncpy` avec vérification de taille
  * **Overflow léger** : dépasse le buffer de quelques octets
  * **Overflow important** : écrase complètement la structure

### 2. **Reader** (Processus lecteur)

* Lit et surveille la mémoire partagée
* Détecte les corruptions via un "canary"
* Affiche des dumps hexadécimaux pour l'analyse

## Structure de la Mémoire Partagée

```
+------------------+
| count (4 bytes)  |  ← Compteur d'écritures
+------------------+
| buffer[100]      |  ← Buffer principal (VULNÉRABLE)
+------------------+
| overflow_detector|  ← "Canary" pour détecter l'overflow
|     [20]         |     Valeur: "CANARY_INTACT"
+------------------+
```

## Compilation et Exécution

### Compilation

bash

```bash
make
# ou
gcc -Wall -Wextra -g -o writer writer.c
gcc -Wall -Wextra -g -o reader reader.c
```

### Lancement

**Terminal 1 - Writer :**

bash

```bash
./writer
```

**Terminal 2 - Reader (dans un autre terminal) :**

bash

```bash
./reader
```

## Scénarios de Test

### Scénario 1 : Écriture normale (sécurisée)

1. Dans **writer** : choisir option `1`
2. Entrer un texte court (< 99 caractères)
3. Dans **reader** : choisir option `2` pour vérifier l'intégrité
4. Résultat : Canary intact, pas d'overflow

### Scénario 2 : Overflow

1. Dans **writer** : choisir option 2
2. Un texte de 199 caractères est généré
3. Dans **reader** : observer la corruption complète
4. Résultat : Le canary est complètement écrasé

## Analyse Technique

### Pourquoi le buffer overflow se produit-il ?

c

```c
// DANGEREUX : strcpy ne vérifie pas la taille
strcpy(data->buffer, long_string);

// SÉCURISÉ : strncpy limite la copie
strncpy(data->buffer, input,sizeof(data->buffer)-1);
```

### Le mécanisme du Canary

Le "canary" est une valeur sentinelle placée après le buffer :

* Si le canary reste `"CANARY_INTACT"` → pas d'overflow
* Si le canary change → le buffer a été dépassé

## Nettoyage

Pour tout nettoyer après les tests :

bash

```bash
make clean        # Supprime les exécutables + mémoire partagée
make clean_shm    # Supprime uniquement la mémoire partagée
```
