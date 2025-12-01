FROM ubuntu:latest

# Installation de gcc
RUN apt-get update && apt-get install -y gcc

# Création du répertoire
RUN mkdir -p /home/script

# Copie du script
COPY script.sh /home/script/

# permissions
RUN chmod +x /home/script/script.sh && \
    chmod 755 /home/script/script.sh

# Répertoire de travail
WORKDIR /home/script

# Point d'entrée avec bash explicite
ENTRYPOINT ["/bin/bash", "/home/script/script.sh"]