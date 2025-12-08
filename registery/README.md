Ah parfait ! Alors simplifions : vous allez configurer le **registry HTTPS sur VM1** et y accéder depuis  **VM2** .

# Guide Docker Registry - 2 VMs uniquement

## Architecture

```
┌─────────────────────┐         ┌─────────────┐
│  VM1 (Linux)        │◀────────│    VM2      │
│  Registry HTTPS     │  pull   │ (Linux/Win) │
│  Port 5000          │  push   │             │
└─────────────────────┘         └─────────────┘
```

---

## Étape 1 : Configurer le registry HTTPS sur VM1

### 1.1 Arrêter et supprimer le registry HTTP actuel (si vous l'aviez lancé)

```bash
docker stop registry
docker rm registry
```

### 1.2 Créer la structure de répertoires

```bash
mkdir -p ~/docker-registry/certs
mkdir -p ~/docker-registry/data
cd ~/docker-registry
```

### 1.3 Obtenir l'IP de VM1

```bash
# Voir votre IP
ip a

# Notez votre IP (ex: 192.168.1.100)
```

### 1.4 Générer le certificat SSL auto-signé

**Remplacez `IP_VM1` par votre IP réelle !**

```bash
openssl req -newkey rsa:4096 -nodes -sha256 \
  -keyout certs/domain.key \
  -x509 -days 365 \
  -out certs/domain.crt \
  -subj "/C=FR/ST=Brittany/L=Vannes/O=MyCompany/CN=IP_VM1" \
  -addext "subjectAltName=IP:IP_VM1"
```

**Exemple avec IP 192.168.1.100** :

```bash
openssl req -newkey rsa:4096 -nodes -sha256 \
  -keyout certs/domain.key \
  -x509 -days 365 \
  -out certs/domain.crt \
  -subj "/C=FR/ST=Brittany/L=Vannes/O=MyCompany/CN=192.168.1.100" \
  -addext "subjectAltName=IP:192.168.1.100"
```

### 1.5 Lancer le registry avec HTTPS

```bash
docker run -d \
  -p 5000:5000 \
  --restart=always \
  --name registry \
  -v ~/docker-registry/data:/var/lib/registry \
  -v ~/docker-registry/certs:/certs \
  -e REGISTRY_HTTP_TLS_CERTIFICATE=/certs/domain.crt \
  -e REGISTRY_HTTP_TLS_KEY=/certs/domain.key \
  registry:2
```

### 1.6 Vérifier sur VM1

```bash
# Container tourne ?
docker ps | grep registry

# Test HTTPS local
curl -k https://localhost:5000/v2/
# Devrait retourner : {}

# Logs
docker logs registry
```

---

## Étape 2 : Configurer VM1 pour utiliser son propre registry

### 2.1 Éditer daemon.json sur VM1

```bash
sudo nano /etc/docker/daemon.json
```

Ajoutez (remplacez par votre IP réelle) :

```json
{
  "insecure-registries" : ["192.168.1.100:5000", "localhost:5000"]
}
```

### 2.2 Redémarrer Docker sur VM1

```bash
sudo systemctl restart docker

# Relancer le registry
docker start registry

# Vérifier
docker ps | grep registry
```

### 2.3 Pusher votre image depuis VM1

```bash
# Tag pour le registry (remplacez l'IP)
docker tag mon-compilateur 192.168.1.100:5000/mon-compilateur

# Push
docker push 192.168.1.100:5000/mon-compilateur

# Vérifier
curl -k https://192.168.1.100:5000/v2/_catalog
# Devrait afficher : {"repositories":["mon-compilateur"]}
```

**VM1 peut maintenant pusher vers son propre registry !**

---

## Étape 3 : Configurer VM2 pour accéder au registry de VM1

### 3.1 Depuis VM2, tester la connexion

```bash
# Ping VM1 (remplacez par l'IP de VM1)
ping 192.168.1.100

# Test curl avec -k
curl -k https://192.168.1.100:5000/v2/
# Devrait retourner : {}
```

### 3.2 Configurer daemon.json sur VM2

```bash
sudo nano /etc/docker/daemon.json
```

Ajoutez (avec l'IP de VM1) :

```json
{
  "insecure-registries" : ["192.168.1.100:5000"]
}
```

### 3.3 Redémarrer Docker sur VM2

```bash
sudo systemctl restart docker
sudo systemctl status docker
```

### 3.4 Tester depuis VM2

```bash
# Pull l'image depuis VM1
docker pull 192.168.1.100:5000/mon-compilateur

# Vérifier
docker images | grep mon-compilateur

# Tester l'image
docker run --rm 192.168.1.100:5000/mon-compilateur
```

✅ **VM2 peut maintenant pull/push depuis le registry de VM1 !**

---

## Étape 4 : Test complet bidirectionnel

### Depuis VM2 : Créer et pusher une image

```bash
# Créer une image simple
docker run -it --name test ubuntu bash
# Dans le container
apt update && apt install -y curl
exit

# Commit le container en image
docker commit test mon-image-vm2

# Tag pour le registry de VM1
docker tag mon-image-vm2 192.168.1.100:5000/mon-image-vm2

# Push vers VM1
docker push 192.168.1.100:5000/mon-image-vm2

# Nettoyer
docker rm test
```

### Depuis VM1 : Pull l'image de VM2

```bash
# Voir le catalogue
curl -k https://localhost:5000/v2/_catalog

# Pull l'image créée depuis VM2
docker pull 192.168.1.100:5000/mon-image-vm2

# Vérifier
docker images | grep mon-image-vm2
```

---

## Commandes récapitulatives

### Sur VM1 (serveur registry)

```bash
# Démarrer le registry HTTPS
cd ~/docker-registry
docker run -d -p 5000:5000 --restart=always --name registry \
  -v ~/docker-registry/data:/var/lib/registry \
  -v ~/docker-registry/certs:/certs \
  -e REGISTRY_HTTP_TLS_CERTIFICATE=/certs/domain.crt \
  -e REGISTRY_HTTP_TLS_KEY=/certs/domain.key \
  registry:2

# Voir le catalogue
curl -k https://localhost:5000/v2/_catalog
```

### Sur VM1 et VM2 (clients)

```bash
# Configuration
sudo nano /etc/docker/daemon.json
# {"insecure-registries":["IP_VM1:5000"]}
sudo systemctl restart docker

# Push
docker tag mon-image IP_VM1:5000/mon-image
docker push IP_VM1:5000/mon-image

# Pull
docker pull IP_VM1:5000/mon-image
```

---

## Troubleshooting

### "connection refused" depuis VM2

```bash
# Sur VM1 : vérifier le registry
docker ps | grep registry
docker logs registry

# Vérifier le port
sudo netstat -tulpn | grep 5000

# Firewall ?
sudo ufw status
sudo ufw allow 5000/tcp  # Si nécessaire
```

### "x509 certificate" errors

```bash
# Vérifier daemon.json
cat /etc/docker/daemon.json

# Redémarrer Docker
sudo systemctl restart docker
```

### Voir les images dans le registry

```bash
# Catalogue
curl -k https://IP_VM1:5000/v2/_catalog

# Tags d'une image
curl -k https://IP_VM1:5000/v2/mon-compilateur/tags/list
```
