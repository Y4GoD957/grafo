# Makefile da Pratica 04 - Conectividade e Planaridade

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
LDLIBS = -lm
TARGET = main
SRC = grafo_lista.c conectividade.c planaridade.c main.c
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
