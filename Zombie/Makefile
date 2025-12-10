
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = zombie_manager

all: $(TARGET)

$(TARGET): zombie_manager.c
	$(CC) $(CFLAGS) -o $(TARGET) zombie_manager.c

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run