# Guide Docker - VM1 Linux

## I - Installation Docker

Commençons par installer Docker :

```bash
# Installation de curl si nécessaire
sudo apt update
sudo apt install curl

# Installation de Docker
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh

# Ajout de votre utilisateur au groupe docker
sudo usermod -aG docker $USER
```

Vérification :

```bash
docker --version
docker compose version
```

## II - C'est parti !

### Premier container

```bash
docker run hello-world
```

Cette commande télécharge et exécute un container de test. Si ça fonctionne, Docker est bien installé !

### Container Ubuntu interactif

```bash
docker run -it ubuntu bash
```

**Que font `-i` et `-t` ?**

* `-i` (interactive) : garde STDIN ouvert, permet l'interaction
* `-t` (tty) : alloue un pseudo-terminal, permet d'avoir un shell utilisable

Dans le container :

```bash
cat /etc/lsb-release
exit
```

### Gestion des containers

```bash
# Liste les containers en cours
docker ps

# Liste TOUS les containers (même arrêtés)
docker ps -a
```

### Container en mode détaché

```bash
docker run -d --name=test ubuntu bash
docker ps
docker ps -a
```

**Pourquoi le container s'arrête immédiatement ?** Parce que `bash` se termine instantanément s'il n'a rien à faire. Un container Docker tourne tant que son processus principal est actif.

Nettoyage et nouveau test :

```bash
docker rm test
docker run -d --name=test ubuntu sh -c 'while true;do echo "Hello";sleep 1;done'
docker ps
```

**Cette fois ça tourne !** Pourquoi ? Le processus principal (la boucle while) ne se termine jamais.

### Interaction avec un container en cours

```bash
# Terminal 1 - Attache au processus principal
docker attach test
```

```bash
# Terminal 2 - Démarre un nouveau processus bash
docker exec -it test bash
```

**Différence entre `attach` et `exec` :**

* `attach` : se connecte au processus principal (ici la boucle while)
* `exec` : lance un NOUVEAU processus dans le container (ici un bash séparé)

### Monitoring

```bash
# Terminal 3 - Statistiques en temps réel
docker stats test

# Terminal 4 - Logs en temps réel
docker logs -f test
```

Arrêt et nettoyage :

```bash
docker stop test
docker rm test
```

### Mode réseau host

```bash
docker run -it --name=test2 --network="host" ubuntu bash
```

Dans le container :

```bash
# Si ifconfig n'est pas installé
apt update
apt install net-tools
ifconfig
# OU
ip a
```

Sur l'hôte (autre terminal) :

```bash
ifconfig
# OU
ip a
```

**Constat :** Les interfaces réseau sont identiques !

**Les modes réseau Docker :**

* **bridge** (défaut) : réseau isolé avec NAT
* **host** : partage le réseau de l'hôte directement
* **none** : aucun réseau
* **container** : partage le réseau d'un autre container

### Option restart

```bash
docker run -it --name=test2 --restart=always --network="host" ubuntu bash
exit
docker ps
```

**Le container redémarre automatiquement !** `--restart=always` relance le container à chaque arrêt.

Pour le supprimer :

```bash
docker rm test2  # Ne fonctionne pas car il tourne
docker rm -f test2  # Force la suppression
```

### Option pour auto-suppression

**L'option `--rm`** supprime automatiquement le container à l'arrêt :

```bash
docker run -it --rm --name=test3 ubuntu bash
exit
docker ps -a  # test3 n'apparaît pas !
```

## III - Dockerfile personnalisé

Suivez d'abord le tutoriel : https://docs.docker.com/get-started/part2/

Créez ensuite votre Dockerfile :

```dockerfile
FROM ubuntu:latest

# Installation de gcc
RUN apt-get update && apt-get install -y gcc && rm -rf /var/lib/apt/lists/*

# Création du répertoire
RUN mkdir -p /home/script

# Copie du script
COPY script.sh /home/script/

# Rendre le script exécutable
RUN chmod +x /home/script/script.sh

# Point d'entrée
ENTRYPOINT ["/home/script/script.sh"]
```

Build de l'image :

```bash
docker build -t compilateur .
```

Test avec vos fichiers C :

```bash
# Dans le dossier contenant script1.c et script2.c
docker run -v $(pwd):/home/script compilateur
```

**Le piège** : Le script `script.sh` doit être exécutable (`chmod +x`). Sans ça, le container ne peut pas le lancer.

## IV - Docker Registry

Suivez le tutoriel : https://devopssec.fr/article/deployer-manipuler-securiser-un-serveur-registry-docker-prive

Tag et push de votre image :

```bash
# Tag pour votre registry local
docker tag compilateur localhost:5000/compilateur

# Push vers le registry
docker push localhost:5000/compilateur
```

Pour le registry HTTPS avec certificat auto-signé, configuration du daemon :

```bash
sudo nano /etc/docker/daemon.json
```

Ajoutez :

```json
{
  "insecure-registries" : ["IP_DE_VOTRE_REGISTRY:5000"]
}
```

Redémarrage :

```bash
sudo systemctl restart docker
```

## V - Docker Compose

Commandes de base :

```bash
docker compose up      # Lance les services
docker compose down    # Arrête et supprime
docker compose ps      # Liste les services
```
