# -----------------------------
# Makefile pour inodes_demo
# -----------------------------

# Nom du programme final
TARGET = inodes_demo

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

# Fichiers sources et objets
SRCS = inodes_demo.c
OBJS = $(SRCS:.c=.o)

# -----------------------------
# Règles principales
# -----------------------------

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Supprime les fichiers objets et l'exécutable
clean:
	rm -f $(OBJS) $(TARGET)

# Supprime aussi le dossier de travail fs_demo
mrproper: clean
	rm -rf fs_demo

.PHONY: all clean mrproper
