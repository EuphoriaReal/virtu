FROM ubuntu:latest

# Installation de gcc
RUN apt-get update && apt-get install -y gcc && rm -rf /var/lib/apt/lists/*

# Création du répertoire
RUN mkdir -p /home/script

# Copie du script
COPY script.sh /home/script/

# Correction format + permissions
RUN sed -i 's/\r$//' /home/script/script.sh && \
    chmod +x /home/script/script.sh && \
    chmod 755 /home/script/script.sh

# Répertoire de travail
WORKDIR /home/script

# Point d'entrée avec bash explicite
ENTRYPOINT ["/bin/bash", "/home/script/script.sh"]