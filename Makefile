CC = gcc
CFLAG = -O2
LDFLAG = -lpthread

all: myftpclient myftpserver myftp.o myftpclient.o myftpserver.o

myftp.o : myftp.c myftp.h
	$(CC) -o $@ -c $< $(CFLAG)

myftpserver.o: myftpserver.c myftp.h
	$(CC) -o $@ -c $< $(CFLAG)

myftpclient.o: myftpclient.c myftp.h
	$(CC) -o $@ -c $< $(CFLAG)

myftpserver: myftpserver.o myftp.o
	$(CC) -o $@ $^ $(LDFLAG)

myftpclient: myftpclient.o myftp.o 
	$(CC) -o $@ $^ $(LDFLAG)

clean:
	rm myftpserver myftpclient myftp.o myftpclient.o myftpserver.o
