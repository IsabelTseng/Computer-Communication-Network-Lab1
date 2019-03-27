CC = gcc -std=c99
CCFLAGS = -D_GNU_SOURCE

SERVER= server
CLIENT= client

all:
	$(CC) -o $(SERVER) $(CCFLAGS)  $(SERVER).c
	$(CC) -o $(CLIENT) $(CCFLAGS)  $(CLIENT).c

clean:
	rm -rf $(SERVER) $(CLIENT)
