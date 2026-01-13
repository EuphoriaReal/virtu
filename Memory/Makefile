# ===============================
# Makefile - Démo mémoire partagée
# ===============================

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

TARGETS = writer reader

# Règle par défaut
all: $(TARGETS)

# Compilation des exécutables
writer: writer.c
	$(CC) $(CFLAGS) -o $@ $<

reader: reader.c
	$(CC) $(CFLAGS) -o $@ $<

# Nettoyage
clean:
	rm -f $(TARGETS)

.PHONY: all clean
