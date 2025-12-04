# =====================================================
# Makefile pour inodes_demo.c
# =====================================================

# Compilateur
CC = gcc
CFLAGS = -Wall -O2

# Nom de l'exécutable
TARGET = inodes_demo

# Fichier source
SRC = inodes_demo.c

# -----------------------------------------------------
# Règles
# -----------------------------------------------------

# Règle par défaut : compiler
all: $(TARGET)

# Compilation
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Nettoyage
clean:
	rm -f $(TARGET)
	rm -rf inodes_demo

# Exécuter le programme
run:
	./$(TARGET)

# Cas 1 : gros fichier
big:
	./$(TARGET) big

# Cas 2 : saturation des inodes
inode:
	./$(TARGET) inode

# Réinitialisation
reset:
	./$(TARGET) reset

.PHONY: all clean run big inode reset
