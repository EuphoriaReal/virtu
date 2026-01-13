CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = zombie
SRC = zombie.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
