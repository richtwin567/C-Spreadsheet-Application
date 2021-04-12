CC=gcc
CFLAGS=-pthread

server: src/server/spreadsheetServer.c
	$(CC) src/server/spreadsheetServer.c $(CFLAGS) -o server

client: src/client/clientMain.c
	$(CC) src/client/clientMain.c $(CFLAGS) -o client