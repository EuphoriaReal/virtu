CC = gcc
CFLAGS = -Wall -std=c99

all: writer reader

writer: writer.c
	$(CC) $(CFLAGS) -o $@ $<

reader: reader.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f writer reader
	ipcrm -M 0x1234 2>/dev/null || true

.PHONY: all clean