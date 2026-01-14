CC = gcc
CFLAGS = -Wall -Wextra -std=c99

TARGET = inodes_demo

all: $(TARGET)

$(TARGET): inodes_demo.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

mrproper: clean
	rm -rf fs_demo

.PHONY: all clean mrproper