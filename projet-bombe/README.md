# Jeu de la Bombe - Docker

Jeu distribue : les joueurs se passent une bombe jusqu'a explosion. Le dernier en vie gagne.

## Architecture

```
VM1 (192.168.200.23)           VM2 (192.168.200.22)
- Server (port 6000)           - Registry (port 5000)
- Player1                      - Player3
- Player2                      - Player4
```

## Deploiement

### 1. VM2 : Demarrer le registry

```bash
docker-compose -f docker-compose-vm2.yml up -d registry
```

### 2. Configurer Docker pour le registry insecure (VM1 et VM2)

Editer `/etc/docker/daemon.json` :
```json
{"insecure-registries": ["192.168.200.22:5000"]}
```

Redemarrer Docker :
```bash
sudo systemctl restart docker
```

### 3. VM1 : Build et push des images

```bash
docker-compose -f docker-compose-vm1.yml build
docker-compose -f docker-compose-vm1.yml push
```

### 4. VM1 : Lancer le serveur et les joueurs

```bash
docker-compose -f docker-compose-vm1.yml up -d
```

### 5. VM2 : Lancer les joueurs

```bash
docker-compose -f docker-compose-vm2.yml up -d player3 player4
```

### 6. Lancer la partie

```bash
docker attach server
# Taper "start" quand les 4 joueurs sont connectes
```

### Bonus : Windows

```cmd
python player_windows.py 192.168.200.23 MonPseudo
```

## Commandes serveur

- `start` : Lancer la partie
- `status` : Voir les joueurs
- `quit` : Arreter

## Logs

```bash
docker logs -f server
docker logs -f player1
```
