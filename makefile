all: client server build/shared.o

build/shared.o: src/shared.c include/shared.h
	gcc -I "include/" src/shared.c -c -o build/shared.o -lncursesw

client: src/client.c include/shared.h build/shared.o
	gcc -I "include/" src/client.c build/shared.o -o bin/client.exe -lncursesw

server: src/server.c include/shared.h build/shared.o
	gcc -I "include/" src/server.c build/shared.o -o bin/server.exe -lncursesw 