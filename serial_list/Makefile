# fichier: Makefile

CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = list_serial_ports
SRC = ./src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

