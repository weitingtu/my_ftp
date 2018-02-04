CC = gcc
CFLAG = -lpthread

all: server client MTserver MTclient myftpclient myftpserver myftp.o myftpclient.o myftpserver.o

server: server.c
	$(CC) -o $@ $<

client: client.c
	$(CC) -o $@ $<

MTserver: MTserver.c
	$(CC) -o $@ $< $(CFLAG)

MTclient: MTclient.c
	$(CC) -o $@ $< $(CFLAG)

myftp.o : myftp.c myftp.h
	$(CC) -o $@ -c $< $(CFLAG)

myftpserver.o: myftpserver.c myftp.h
	$(CC) -o $@ -c $< $(CFLAG)

myftpserver: myftpserver.o myftp.o
	$(CC) -o $@ $^ $(CFLAG)

myftpclient.o: myftpclient.c myftp.h
	$(CC) -o $@ -c $< $(CFLAG)

myftpclient: myftpclient.o myftp.o 
	$(CC) -o $@ $^ $(CFLAG)

clean:
	rm server client MTserver MTclient myftpserver myftpclient myftp.o myftpclient.o myftpserver.o
