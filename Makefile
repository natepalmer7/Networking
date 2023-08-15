CC = gcc
DEPS = client-server.h
OBJ = helpers.o

all: client server

client: $(OBJ) $(DEPS) client.c
	$(CC) -o client client.c $(OBJ)  # <-- Ensure this line starts with a tab, not spaces

server: $(OBJ) $(DEPS) server.c
	$(CC) -o server server.c $(OBJ)  # <-- Ensure this line starts with a tab, not spaces

clean:
	rm -f client server $(OBJ)  # <-- Ensure this line starts with a tab