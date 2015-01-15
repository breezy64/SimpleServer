CC=gcc
CFLAGS=-g -Wall

server_init.o:server_init.c server_init.h
	$(CC) -c $(CFLAGS) server_init.c
serve.o:serve.c serve.h
	$(CC) -c $(CFLAGS) serve.c
serverf.o:serverf.c server_init.h
	$(CC) -c $(CFLAGS) serverf.c
serverf: serverf.o server_init.o serve.o
	$(CC) $(CFLAGS) -o server_f serverf.o server_init.o serve.o
server_p.o:server_p.c server_init.h
	$(CC) -c $(CFLAGS) server_p.c
serverp: server_p.o server_init.o serve.o
	$(CC) $(CFLAGS) -pthread -o server_p server_p.o server_init.o serve.o
all:serverp serverf
	$(CC) $(CFLAGS) -pthread -o server_p server_p.o server_init.o serve.o
	$(CC) $(CFLAGS) -o server_f serverf.o server_init.o serve.o

	
