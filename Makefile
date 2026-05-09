CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

TARGET = csvreader

SRC = src/csvreader.c src/hashmap.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)