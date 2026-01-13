
# Jeu de la Bombe

## Architecture

```
VM1 (192.168.200.22)           VM2 (192.168.200.23)
├── Server (port 6000)         ├── Registry (port 5000)
├── Player1                    ├── Player3
└── Player2                    └── Player4
```

## Configuration

Sur VM1 et VM2, editer `/etc/docker/daemon.json` :

```json
{"insecure-registries": ["192.168.200.23:5000"]}
```

Puis :

```bash
sudo systemctl restart docker
```

## Deploiement

### VM2

```bash
docker-compose -f docker-compose-vm2.yml up -d registry
```

### VM1

```bash
docker-compose -f docker-compose-vm1.yml build
docker-compose -f docker-compose-vm1.yml push
docker-compose -f docker-compose-vm1.yml up -d
```

### VM2

```bash
docker-compose -f docker-compose-vm2.yml up -d player3 player4
```

## Lancer une partie via VM1

```bash
docker attach server
```

| Commande   | Action                     |
| ---------- | -------------------------- |
| `start`  | Lancer la partie           |
| `status` | Voir les joueurs           |
| `reset`  | Reinitialiser pour rejouer |
| `quit`   | Arreter le serveur         |

## Bonus Windows

```cmd
python player_windows.py 192.168.200.22 MonPseudo
```

## Logs

```bash
docker logs -f server
docker logs -f player1
```
