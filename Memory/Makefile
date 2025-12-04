# Makefile pour le projet Buffer Overflow sur mémoire partagée

CC = gcc
CFLAGS = -Wall -Wextra -g
TARGETS = writer reader

all: $(TARGETS)

writer: writer.c
	$(CC) $(CFLAGS) -o writer writer.c

reader: reader.c
	$(CC) $(CFLAGS) -o reader reader.c

clean:
	rm -f $(TARGETS)
	ipcs -m | grep 0x00001234 | awk '{print $$2}' | xargs -r ipcrm -m

run_writer:
	./writer

run_reader:
	./reader

# Nettoyer les segments de mémoire partagée orphelins
clean_shm:
	@echo "Nettoyage des segments de mémoire partagée..."
	@ipcs -m | grep 0x00001234 | awk '{print $$2}' | xargs -r ipcrm -m
	@echo "Nettoyage terminé"

status:
	@echo "=== Segments de mémoire partagée actifs ==="
	@ipcs -m

help:
	@echo "Commandes disponibles:"
	@echo "  make          - Compile writer et reader"
	@echo "  make clean    - Supprime les exécutables et la mémoire partagée"
	@echo "  make run_writer - Lance le processus writer"
	@echo "  make run_reader - Lance le processus reader"
	@echo "  make clean_shm  - Nettoie uniquement la mémoire partagée"
	@echo "  make status     - Affiche les segments de mémoire partagée actifs"

.PHONY: all clean run_writer run_reader clean_shm status help