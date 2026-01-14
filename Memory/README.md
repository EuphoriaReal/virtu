
# Démo Buffer Overflow en Mémoire Partagée

Démonstration simple d'un débordement de buffer entre deux processus via System V shared memory.

## Compilation

```bash
make
```

## Utilisation

```bash
# Terminal 1 : monitoring continu
./writer reset
./reader

# Terminal 2 : tests
./writer safe      # écriture sécurisée
./writer overflow  # provoque le débordement
./writer reset     # réinitialise
```

## Principe

La structure partagée contient un buffer de 100 octets suivi d'un canary ("CANARY_OK"). En mode `overflow`, on écrit 150 octets dans le buffer, écrasant le canary. Le reader détecte cette corruption.

## Nettoyage

```bash
make clean
```
