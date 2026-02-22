all: client server build/shared.o

build/shared.o: src/shared.c include/shared.h
	gcc -I "include/" src/shared.c -c -o build/shared.o -lncursesw

client: src/client.c include/shared.h build/shared.o include/client.h
	gcc -I "include/" src/client.c build/shared.o -o bin/client.exe -lncursesw

server: src/server.c include/shared.h build/shared.o include/server.h
	gcc -I "include/" src/server.c build/shared.o -o bin/server.exe -lncursesw 

test: src/server.c include/shared.h include/client.h include/server.h
	gcc -I "include/" src/shared.c -D MODE_TEST=MODE_TEST -c -o build/TEST_shared.o -lncursesw
	gcc -I "include/" src/client.c build/TEST_shared.o -D MODE_TEST=MODE_TEST -o bin/TEST_client.exe -lncursesw
	gcc -I "include/" src/server.c build/TEST_shared.o -D MODE_TEST=MODE_TEST -o bin/TEST_server.exe -lncursesw 
