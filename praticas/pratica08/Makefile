# Makefile da Pratica 08 - Arvore Geradora Minima

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
LDLIBS = -lm
TARGET = main
SRC = agm.c main.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
