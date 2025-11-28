all: client server


client: src/client.c include/shared.h
	gcc -I "include/" src/client.c -o bin/client.exe

server: src/server.c include/shared.h
	gcc -I "include/" src/client.c -o bin/server.exe